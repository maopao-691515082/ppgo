#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

namespace experimental
{

static DBLite::ZMap WriteZM(DBLite::ZMap zm, const WriteBatch::RawOpsMap &wb_ops)
{
    static const ssize_t
        kGSStrCountMax = 40,
        kGSStrCountMergeThreshold = kGSStrCountMax / 2;

    auto wb_ops_iter = wb_ops.begin();
    while (wb_ops_iter != wb_ops.end())
    {
        //根据WB中当前要处理的K找到对应的需要合并的GS，以及其后继的GS信息
        ::lom::GoSlice<Str> gs;
        std::optional<Str> next_gs_first_k;
        ::lom::GoSlice<Str> next_gs;
        ssize_t merge_idx = 0;  //保持指向当前合并的GS应该插入的位置
        if (zm.Size() > 0)
        {
            auto v_ptr = zm.Get(wb_ops_iter->first, &merge_idx);
            if (v_ptr == nullptr)
            {
                //找到插入位置，使用前驱的GS，若无前驱则使用当前GS
                if (merge_idx > 0)
                {
                    -- merge_idx;
                }
                auto pp = zm.GetByIdx(merge_idx);
                gs = *pp.second;
            }
            else
            {
                //恰好有一个以WB的当前K开头的GS
                gs = *v_ptr;
            }

            //删掉这个GS
            zm = zm.DelByIdx(merge_idx);

            //删除后，当前`merge_idx`是GS的后继，获取后继的信息
            if (merge_idx < zm.Size())
            {
                auto pp = zm.GetByIdx(merge_idx);
                next_gs_first_k = *pp.first;
                next_gs = *pp.second;
            }
        }

        //进行合并
        Assert(gs.Len() % 2 == 0);
        ssize_t gs_idx = 0;
        GoSlice<Str> merged_kvs;
        for (;;)
        {
            Str k, v;

            if (wb_ops_iter != wb_ops.end())
            {
                auto const &wb_ops_k = wb_ops_iter->first;
                if (gs_idx < gs.Len())
                {
                    //两边都有值
                    int cmp_result = wb_ops_k.Cmp(gs.At(gs_idx));
                    if (cmp_result < 0)
                    {
                        if (!wb_ops_iter->second)
                        {
                            ++ wb_ops_iter;
                            continue;
                        }
                        k = wb_ops_k;
                        v = *wb_ops_iter->second;
                        ++ wb_ops_iter;
                    }
                    else if (cmp_result > 0)
                    {
                        k = gs.At(gs_idx);
                        v = gs.At(gs_idx + 1);
                        gs_idx += 2;
                    }
                    else
                    {
                        gs_idx += 2;
                        if (!wb_ops_iter->second)
                        {
                            ++ wb_ops_iter;
                            continue;
                        }
                        k = wb_ops_k;
                        v = *wb_ops_iter->second;
                        ++ wb_ops_iter;
                    }
                }
                else
                {
                    if (next_gs_first_k && wb_ops_k >= *next_gs_first_k)
                    {
                        //当前GS已经merge完毕
                        break;
                    }
                    if (!wb_ops_iter->second)
                    {
                        ++ wb_ops_iter;
                        continue;
                    }
                    k = wb_ops_k;
                    v = *wb_ops_iter->second;
                    ++ wb_ops_iter;
                }
            }
            else
            {
                if (gs_idx < gs.Len())
                {
                    k = gs.At(gs_idx);
                    v = gs.At(gs_idx + 1);
                    gs_idx += 2;
                }
                else
                {
                    //both end
                    break;
                }
            }

            if (merged_kvs.Len() > 0 && merged_kvs.Len() + 2 > kGSStrCountMax)
            {
                //即将超限，新生成GS
                Str new_gs_first_k = merged_kvs.At(0);
                ssize_t len_soft_max_kv_count = merged_kvs.Len() / 2;
                if (len_soft_max_kv_count >= 3)
                {
                    len_soft_max_kv_count = len_soft_max_kv_count * 2 / 3;
                }
                auto new_gs = merged_kvs.Slice(0, len_soft_max_kv_count * 2).Copy();
                merged_kvs = merged_kvs.Slice(len_soft_max_kv_count * 2);

                //插入
                zm = zm.AddByIdx(merge_idx, new_gs_first_k, new_gs);
                ++ merge_idx;
            }
            if (merged_kvs.Len() > 0)
            {
                Assert(k > merged_kvs.At(merged_kvs.Len() - 2));
            }
            merged_kvs = merged_kvs.Append(k).Append(v);
        }

        //尾部处理
        if (merged_kvs.Len() > 0)
        {
            Str new_gs_first_k = merged_kvs.At(0);
            auto new_gs = merged_kvs;
            merged_kvs = merged_kvs.Nil();

            //计算一下是否需要和后继进行合并
            if (next_gs_first_k && new_gs.Len() + next_gs.Len() < kGSStrCountMergeThreshold)
            {
                //需要合并，删掉后继GS，链接到新的GS后面
                zm = zm.DelByIdx(merge_idx);
                new_gs.AppendGoSlice(next_gs);
            }

            //插入
            if (new_gs.Cap() > new_gs.Len() * 5 / 4)
            {
                new_gs = new_gs.Copy();
            }
            zm = zm.AddByIdx(merge_idx, new_gs_first_k, new_gs);
            ++ merge_idx;
        }
    }

    return zm;
}

LOM_ERR DBLiteImpl::Write(const WriteBatch &wb)
{
    auto const &wb_ops = wb.RawOps();
    if (wb_ops.empty())
    {
        return nullptr;
    }

    //保存一下老的`zm_`，在锁外释放空间
    ZMap old_zm;

    {
        std::lock_guard<std::mutex> wlg(write_lock_);

        old_zm = zm_;   //已经保证了没有人修改`zm_`，所以不加`update_lock_`直接读
        ZMap new_zm = WriteZM(old_zm, wb_ops);

        {
            std::lock_guard<std::mutex> ulg(update_lock_);
            zm_ = new_zm;
        }
    }

    return nullptr;
}

Snapshot::Ptr DBLiteImpl::NewSnapshot()
{
    ZMap zm;
    {
        std::lock_guard<std::mutex> lg(update_lock_);
        zm = zm_;
    }
    return ::lom::ordered_kv::Snapshot::Ptr(new Snapshot(zm));
}

DBLite::ZMap DBLiteImpl::RawZMap()
{
    std::lock_guard<std::mutex> ulg(update_lock_);
    return zm_;
}

LOM_ERR DBLite::Open(DBLite::Ptr &db)
{
    db = std::make_shared<DBLiteImpl>();
    return nullptr;
}

}

}

}

}
