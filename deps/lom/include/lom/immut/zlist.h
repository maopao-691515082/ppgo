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
        StrSlice zs_;
        StrSlice v_;

        void ParseV();

    public:

        Iterator(const ZList &zl) : z_(zl.z_), zs_(zl.z_)
        {
            ParseV();
        }

        bool Valid() const
        {
            return zs_.Len() != 0;
        }

        StrSlice Get() const
        {
            return v_;
        }

        void Next()
        {
            zs_ = zs_.Slice(v_.Data() + v_.Len() + 1 - zs_.Data());
            ParseV();
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

    //各种修改操作，会返回修改后的新结果，不会改变原对象内容
    ZList Append(StrSlice s) const;
    ZList Extend(const ZList &zl) const;
    ZList Extend(const GoSlice<StrSlice> &gs) const;
};

}

}
