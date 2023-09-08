#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

void DBImpl::Snapshot::Iterator::SeekImpl(const Str &k)
{
    ssize_t zm_idx;
    auto v_ptr = zm_.Get(k, &zm_idx);
    if (v_ptr != nullptr || zm_idx == 0)
    {
        //`k`刚好匹配了一个ZL头，或小于所有K
        Reset(zm_idx);
        return;
    }

    //在前驱节点找
    Reset(zm_idx - 1);
    auto ks = k.Slice();
    while (zl_iter_.Valid() && zl_iter_.Get() < ks)
    {
        zl_iter_.Next();
        Assert(zl_iter_.Valid());
        zl_iter_.Next();
    }
    if (!zl_iter_.Valid())
    {
        //前驱中都小于`k`，以找到的位置为准
        Reset(zm_idx);
    }
}

void DBImpl::Snapshot::Iterator::SeekPrevImpl(const Str &k)
{
    ssize_t zm_idx;
    zm_.Get(k, &zm_idx);
    Reset(zm_idx - 1);
    if (zm_idx == 0)
    {
        return;
    }
    SeekZLIterLast();
    auto ks = k.Slice();
    //前驱ZL的第一个K是小于`k`的，所以必然能在里面找到`k`的前驱
    while (zl_iter_.Valid() && zl_iter_.Get() >= ks)
    {
        zl_iter_.Prev();
        Assert(zl_iter_.Valid());
        zl_iter_.Prev();
    }
    Assert(zl_iter_.Valid());
}

ssize_t DBImpl::Snapshot::Iterator::MoveImpl(ssize_t step, const std::optional<Str> &stop_at)
{
    ssize_t moved_step = 0;

    if (step > 0)
    {
        if (IsLeftBorderImpl())
        {
            SeekFirstImpl();
            -- step;
            ++ moved_step;
        }
        while (step > 0 && !IsRightBorderImpl())
        {
            auto zl_remain_count = zl_iter_.StrCount() - zl_iter_.Idx();
            Assert(zl_remain_count > 0 && zl_remain_count % 2 == 0);
            auto zl_remain_kv_count = zl_remain_count / 2;
            if (zl_remain_kv_count <= step)
            {
                //`step`比较大，检查一下看能不能直接跳过当前ZL
                std::optional<StrSlice> next_zl_first_k;
                if (zm_idx_ + 1 < zm_.Size())
                {
                    next_zl_first_k = zm_.GetByIdx(zm_idx_ + 1).first->Slice();
                }
                if (!stop_at || (next_zl_first_k && *next_zl_first_k <= stop_at->Slice()))
                {
                    //未指定截止串，或截止串不可能在当前ZL，则直接跳过当前ZL
                    step -= zl_remain_kv_count;
                    moved_step += zl_remain_kv_count;
                    Reset(zm_idx_ + 1);
                    continue;
                }
            }

            //不能跳过，一个个检查当前ZL的元素
            while (step > 0 && zl_iter_.Valid() && (!stop_at || zl_iter_.Get() < stop_at->Slice()))
            {
                zl_iter_.Next();
                Assert(zl_iter_.Valid());
                zl_iter_.Next();
                -- step;
                ++ moved_step;
            }
            if (zl_iter_.Valid())
            {
                //当前ZL没有遍历完，说明要么步数到了，要么到达`stop_at`了
                break;
            }
            //下一个ZL
            Reset(zm_idx_ + 1);
        }
    }
    else
    {
        //向前移动，和上面的流程逻辑上对称
        Assert(step < 0);
        if (IsRightBorderImpl())
        {
            SeekLastImpl();
            ++ step;
            ++ moved_step;
        }
        while (step < 0 && !IsLeftBorderImpl())
        {
            auto zl_remain_count = zl_iter_.Idx() + 2;  //到前驱ZL最后一个KV对的距离
            Assert(zl_remain_count > 0 && zl_remain_count % 2 == 0);
            auto zl_remain_kv_count = zl_remain_count / 2;
            if (zl_remain_kv_count <= -step)
            {
                ::lom::immut::ZList::Iterator prev_zl_iter;
                std::optional<StrSlice> prev_zl_last_k;
                if (zm_idx_ > 0)
                {
                    prev_zl_iter = ::lom::immut::ZList::Iterator(*zm_.GetByIdx(zm_idx_ - 1).second);
                    SeekZLIterLast(prev_zl_iter);
                    prev_zl_last_k = prev_zl_iter.Get();
                }
                if (!stop_at || (prev_zl_last_k && *prev_zl_last_k >= stop_at->Slice()))
                {
                    step += zl_remain_kv_count;
                    moved_step += zl_remain_kv_count;
                    //上面已经算过`prev_zl_iter`了，所以这里手动设置，避免一次重复计算
                    Reset(zm_idx_ - 1, &prev_zl_iter);
                    continue;
                }
            }

            while (step < 0 && zl_iter_.Valid() && (!stop_at || zl_iter_.Get() > stop_at->Slice()))
            {
                zl_iter_.Prev();
                if (zl_iter_.Valid())
                {
                    zl_iter_.Prev();
                }
                ++ step;
                ++ moved_step;
            }
            if (zl_iter_.Valid())
            {
                break;
            }
            Reset(zm_idx_ - 1);
            if (zm_idx_ >= 0)
            {
                SeekZLIterLast();
            }
        }
    }

    return moved_step;
}

}

}

}
