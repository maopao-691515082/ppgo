#pragma once

#include "../_internal.h"

#include "../go_slice.h"
#include "../str.h"

namespace lom
{

namespace immut
{

//`ZList`是一个将字符串列表紧凑存储在一起的不可变对象
class ZList
{
    ssize_t str_count_ = 0;
    Str z_;

public:

    class Iterator
    {
        Str z_;
        GoSlice<StrSlice> gs_;
        ssize_t gs_idx_ = 0;

    public:

        Iterator() = default;

        Iterator(const ZList &zl) : z_(zl.z_), gs_(zl.Parse())
        {
        }

        ssize_t StrCount() const
        {
            return gs_.Len();
        }
        ssize_t Idx() const
        {
            return gs_idx_;
        }

        bool Valid() const
        {
            return gs_idx_ >= 0 && gs_idx_ < gs_.Len();
        }

        StrSlice Get(ssize_t offset = 0) const
        {
            return gs_.At(gs_idx_ + offset);
        }

        void Seek(ssize_t idx)
        {
            Assert(idx >= -1 && idx <= gs_.Len());
            gs_idx_ = idx;
        }

        void Prev()
        {
            if (gs_idx_ >= 0)
            {
                -- gs_idx_;
            }
        }
        void Next()
        {
            if (gs_idx_ < gs_.Len())
            {
                ++ gs_idx_;
            }
        }
    };

    ZList() = default;

    //返回字符串数量
    ssize_t StrCount() const
    {
        return str_count_;
    }
    //返回消耗的空间
    ssize_t SpaceCost() const
    {
        return z_.Len();
    }

    //将包含的串解析为一个列表，可指定数量限制，`limit`<=0表示全部解析出来
    GoSlice<StrSlice> Parse(ssize_t limit = 0) const;

    //快速返回第一个字符串，调用者确保当前ZL不为空
    StrSlice FirstStr() const;

    //各种修改操作，会返回修改后的新结果，不会改变原对象内容
    ZList Append(StrSlice s) const;
    ZList Extend(const ZList &zl) const;
    ZList Extend(const GoSlice<StrSlice> &gs) const;
};

}

}
