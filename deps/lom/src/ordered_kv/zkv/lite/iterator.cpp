#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

namespace experimental
{

void DBLiteImpl::Snapshot::Iterator::SeekImpl(const Str &k)
{
    ssize_t zm_idx;
    auto v_ptr = zm_.Get(k, &zm_idx);
    if (v_ptr != nullptr || zm_idx == 0)
    {
        //`k`刚好匹配了一个GS头，或小于所有K
        Reset(zm_idx);
        return;
    }

    //在前驱节点找
    Reset(zm_idx - 1);
    auto ks = k.Slice();
    while (gs_idx_ < gs_->Len() && gs_->At(gs_idx_).Slice() < ks)
    {
        gs_idx_ += 2;
    }
    if (gs_idx_ >= gs_->Len())
    {
        //前驱中都小于`k`，以找到的位置为准
        Reset(zm_idx);
    }
}

void DBLiteImpl::Snapshot::Iterator::SeekPrevImpl(const Str &k)
{
    ssize_t zm_idx;
    zm_.Get(k, &zm_idx);
    Reset(zm_idx - 1, true);
    if (zm_idx == 0)
    {
        return;
    }

    auto ks = k.Slice();
    //前驱GS的第一个K是小于`k`的，所以必然能在里面找到`k`的前驱
    while (gs_->At(gs_idx_).Slice() > ks)
    {
        gs_idx_ -= 2;
    }
}

ssize_t DBLiteImpl::Snapshot::Iterator::MoveImpl(ssize_t step, const std::optional<Str> &stop_at)
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
            auto gs_remain_kv_count = (gs_->Len() - gs_idx_) / 2;
            if (gs_remain_kv_count <= step)
            {
                //`step`比较大，检查一下看能不能直接跳过当前GS
                std::optional<StrSlice> next_gs_first_k;
                if (zm_idx_ + 1 < zm_.Size())
                {
                    next_gs_first_k = zm_.GetByIdx(zm_idx_ + 1).first->Slice();
                }
                if (!stop_at || (next_gs_first_k && *next_gs_first_k <= stop_at->Slice()))
                {
                    //未指定截止串，或截止串不可能在当前GS，则直接跳过当前GS
                    step -= gs_remain_kv_count;
                    moved_step += gs_remain_kv_count;
                    Reset(zm_idx_ + 1);
                    continue;
                }
            }

            //不能跳过，一个个检查当前GS的元素
            while (
                step > 0 && gs_idx_ < gs_->Len() &&
                (!stop_at || gs_->At(gs_idx_).Slice() < stop_at->Slice()))
            {
                gs_idx_ += 2;
                -- step;
                ++ moved_step;
            }
            if (gs_idx_ < gs_->Len())
            {
                //当前GS没有遍历完，说明要么步数到了，要么到达`stop_at`了
                break;
            }
            //下一个GS
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
            auto gs_remain_kv_count = (gs_idx_ + 2) / 2;    //到前驱GS最后一个KV对的距离
            if (gs_remain_kv_count <= -step)
            {
                const ::lom::GoSlice<Str> *prev_gs = nullptr;
                StrSlice prev_gs_last_k;
                if (zm_idx_ > 0)
                {
                    prev_gs = zm_.GetByIdx(zm_idx_ - 1).second;
                    Assert(prev_gs->Len() > 0 && prev_gs->Len() % 2 == 0);
                    prev_gs_last_k = prev_gs->At(prev_gs->Len() - 2).Slice();
                }
                if (!stop_at || (prev_gs && prev_gs_last_k >= stop_at->Slice()))
                {
                    step += gs_remain_kv_count;
                    moved_step += gs_remain_kv_count;
                    Reset(zm_idx_ - 1, true);
                    continue;
                }
            }

            while (step < 0 && gs_idx_ >= 0 && (!stop_at || gs_->At(gs_idx_).Slice() > stop_at->Slice()))
            {
                gs_idx_ -= 2;
                ++ step;
                ++ moved_step;
            }
            if (gs_idx_ >= 0)
            {
                break;
            }
            Reset(zm_idx_ - 1, true);
        }
    }

    return moved_step;
}

}

}

}

}
