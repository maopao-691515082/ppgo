#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

DBImpl::DBImpl(const char *path_str, Options opts, ::lom::Err::Ptr &err)
{
    if (path_str == nullptr)
    {
        //memory mode
        return;
    }

    //disk mode
    ::lom::os::Path path(path_str);
    ::lom::os::FileStat fst;
    err = ::lom::os::FileStat::Stat(path.Str().CStr(), fst);
    if (err)
    {
        return;
    }
    if (!fst.Exists())
    {
        if (!opts.create_if_missing)
        {
            err = ::lom::Err::Sprintf("no such dir `%s`", path.Str().CStr());
            return;
        }
        err = path.MakeDirs();
        if (err)
        {
            return;
        }
        
    }
}

::lom::Err::Ptr DBImpl::Write(const WriteBatch &wb)
{
    static const ssize_t
        kZLDataLenMax = 1024,
        kZLDataLenMergeThreshold = kZLDataLenMax / 2;

    auto const &wb_ops = wb.RawOps();
    if (wb_ops.empty())
    {
        return nullptr;
    }

    //保存一下老的`zm_`，在锁外释放空间
    ZMap old_zm;

    {
        std::lock_guard<std::mutex> wlg(write_lock_);

        //这里已经保证了没有人修改`zm_`，所以不加`update_lock_`直接读
        auto zm = zm_;
        old_zm = zm;

        //在`zm`上执行写操作
        auto wb_ops_iter = wb_ops.begin();
        while (wb_ops_iter != wb_ops.end())
        {
            //根据WB中当前要处理的K找到对应的需要合并的ZL，以及其后继的ZL信息
            ::lom::immut::ZList zl;
            std::optional<Str> next_zl_first_k;
            ::lom::immut::ZList next_zl;
            ssize_t merge_idx = 0;  //保持指向当前合并的ZL应该插入的位置
            if (zm.Size() > 0)
            {
                auto v_ptr = zm.Get(wb_ops_iter->first, &merge_idx);
                if (v_ptr == nullptr)
                {
                    //找到插入位置，使用前驱的ZL，若无前驱则使用当前ZL
                    if (merge_idx > 0)
                    {
                        -- merge_idx;
                    }
                    auto pp = zm.GetByIdx(merge_idx);
                    zl = *pp.second;
                }
                else
                {
                    //恰好有一个以WB的当前K开头的ZL
                    zl = *v_ptr;
                }

                //删掉这个ZL
                zm = zm.DelByIdx(merge_idx);

                //删除后，当前`merge_idx`是ZL的后继，获取后继的信息
                if (merge_idx < zm.Size())
                {
                    auto pp = zm.GetByIdx(merge_idx);
                    next_zl_first_k = *pp.first;
                    next_zl = *pp.second;
                }
            }

            //进行合并
            Assert(zl.StrCount() % 2 == 0);
            ::lom::immut::ZList::Iterator zl_iter(zl);
            GoSlice<StrSlice> merged_kvs;
            ssize_t merged_kvs_data_len = 0;
            for (;;)
            {
                StrSlice k, v;

                if (wb_ops_iter != wb_ops.end())
                {
                    auto wb_ops_k = wb_ops_iter->first.Slice();
                    if (zl_iter.Valid())
                    {
                        //两边都有值
                        int cmp_result = wb_ops_k.Cmp(zl_iter.Get());
                        if (cmp_result < 0)
                        {
                            if (!wb_ops_iter->second)
                            {
                                ++ wb_ops_iter;
                                continue;
                            }
                            k = wb_ops_k;
                            v = wb_ops_iter->second->Slice();
                            ++ wb_ops_iter;
                        }
                        else if (cmp_result > 0)
                        {
                            k = zl_iter.Get();
                            zl_iter.Next();
                            Assert(zl_iter.Valid());
                            v = zl_iter.Get();
                            zl_iter.Next();
                        }
                        else
                        {
                            zl_iter.Next();
                            Assert(zl_iter.Valid());
                            zl_iter.Next();
                            if (!wb_ops_iter->second)
                            {
                                ++ wb_ops_iter;
                                continue;
                            }
                            k = wb_ops_k;
                            v = wb_ops_iter->second->Slice();
                            ++ wb_ops_iter;
                        }
                    }
                    else
                    {
                        if (next_zl_first_k && wb_ops_k >= next_zl_first_k->Slice())
                        {
                            //当前ZL已经merge完毕
                            break;
                        }
                        if (!wb_ops_iter->second)
                        {
                            ++ wb_ops_iter;
                            continue;
                        }
                        k = wb_ops_k;
                        v = wb_ops_iter->second->Slice();
                        ++ wb_ops_iter;
                    }
                }
                else
                {
                    if (zl_iter.Valid())
                    {
                        k = zl_iter.Get();
                        zl_iter.Next();
                        Assert(zl_iter.Valid());
                        v = zl_iter.Get();
                        zl_iter.Next();
                    }
                    else
                    {
                        //both end
                        break;
                    }
                }

                ssize_t add_data_len = 4 + k.Len() + v.Len();
                if (merged_kvs_data_len > 0 && merged_kvs_data_len + add_data_len > kZLDataLenMax)
                {
                    //即将超限，新生成ZL
                    Str new_zl_first_k = merged_kvs.At(0);
                    ssize_t len_soft_max_kv_count = merged_kvs.Len() / 2;
                    if (len_soft_max_kv_count >= 3)
                    {
                        len_soft_max_kv_count = len_soft_max_kv_count * 2 / 3;
                    }
                    auto new_zl = ::lom::immut::ZList().Extend(
                        merged_kvs.Slice(0, len_soft_max_kv_count * 2));
                    merged_kvs = merged_kvs.Slice(len_soft_max_kv_count * 2);
                    merged_kvs_data_len = 0;
                    for (ssize_t i = 0; i < merged_kvs.Len(); ++ i)
                    {
                        merged_kvs_data_len += 2 + merged_kvs.At(i).Len();
                    }

                    //插入
                    zm = zm.AddByIdx(merge_idx, new_zl_first_k, new_zl);
                    ++ merge_idx;
                }
                if (merged_kvs.Len() > 0)
                {
                    Assert(k > merged_kvs.At(merged_kvs.Len() - 2));
                }
                merged_kvs = merged_kvs.Append(k).Append(v);
                merged_kvs_data_len += add_data_len;
            }

            //尾部处理
            if (merged_kvs_data_len > 0)
            {
                Str new_zl_first_k = merged_kvs.At(0);
                auto new_zl = ::lom::immut::ZList().Extend(merged_kvs);
                merged_kvs = merged_kvs.Nil();
                merged_kvs_data_len = 0;

                //计算一下是否需要和后继进行合并
                if (next_zl_first_k && new_zl.SpaceCost() + next_zl.SpaceCost() < kZLDataLenMergeThreshold)
                {
                    //需要合并，删掉后继ZL，链接到新的ZL后面
                    zm = zm.DelByIdx(merge_idx);
                    new_zl = new_zl.Extend(next_zl);
                }

                //插入
                zm = zm.AddByIdx(merge_idx, new_zl_first_k, new_zl);
                ++ merge_idx;
            }
        }

        //更新`zm_`
        {
            std::lock_guard<std::mutex> ulg(update_lock_);
            zm_ = zm;
        }
    }

    return nullptr;
}

::lom::ordered_kv::Snapshot::Ptr DBImpl::NewSnapshot()
{
    ZMap zm;
    {
        std::lock_guard<std::mutex> lg(update_lock_);
        zm = zm_;
    }
    return ::lom::ordered_kv::Snapshot::Ptr(new Snapshot(zm));
}

::lom::Err::Ptr DB::Open(const char *path, DB::Ptr &db, DB::Options opts)
{
    ::lom::Err::Ptr err;
    DB::Ptr new_db(new DBImpl(path, opts, err));
    if (!err)
    {
        db = new_db;
    }
    return err;
}

}

}

}
