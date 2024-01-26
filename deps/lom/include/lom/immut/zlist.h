#pragma once

#include "../_internal.h"

#include "../go_slice.h"
#include "../str.h"
#include "../io/io.h"

namespace lom
{

namespace immut
{

//`ZList`是一个将字符串列表紧凑存储在一起的不可变对象
class ZList
{
    ssize_t str_count_ = 0;
    //注：不能直接用`Str`之类的不可变对象，必须长引用堆对象以确保各迭代器的`StrSlice`有效，这里直接用`Buf`
    std::shared_ptr<Str::Buf> z_;

    ZList(ssize_t str_count, std::shared_ptr<Str::Buf> &&z) : str_count_(str_count), z_(std::move(z))
    {
    }

    static std::shared_ptr<Str::Buf> EmptyBuf();

public:

    class Iterator
    {
        std::shared_ptr<Str::Buf> z_;
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

    ZList() : z_(EmptyBuf())
    {
    }

    ZList(const GoSlice<StrSlice> &gs)
    {
        auto zl = ZList().Extend(gs);
        str_count_ = zl.str_count_;
        z_ = zl.z_;
    }

    ZList(const ZList &zl) = default;
    ZList(ZList &&zl)
    {
        str_count_ = zl.str_count_;
        z_ = std::move(zl.z_);

        zl.str_count_ = 0;
        zl.z_ = EmptyBuf();
    }

    ZList &operator=(const ZList &zl) = default;
    ZList &operator=(ZList &&zl)
    {
        str_count_ = zl.str_count_;
        z_ = std::move(zl.z_);

        zl.str_count_ = 0;
        zl.z_ = EmptyBuf();

        return *this;
    }

    //返回字符串数量
    ssize_t StrCount() const
    {
        return str_count_;
    }
    //返回消耗的空间
    ssize_t SpaceCost() const
    {
        return z_->Cap();
    }

    //将包含的串解析为一个列表，可指定数量限制，`limit`<=0表示全部解析出来
    GoSlice<StrSlice> Parse(ssize_t limit = 0) const;

    //快速返回第一个字符串，调用者确保当前ZL不为空
    StrSlice FirstStr() const;

    /*
    函数形式的简单、快速的顺序迭代器
    可反复调用以迭代元素，调用返回`true`表示成功，`false`表示已结束
    调用的结果串引用的内容由迭代器维持，在迭代器生命周期内有效
    */
    std::function<bool (StrSlice &)> NewForwardIterator() const;

    //各种修改操作，会返回修改后的新结果，不会改变原对象内容
    ZList Append(StrSlice s) const;
    ZList Extend(const ZList &zl) const;
    ZList Extend(const GoSlice<StrSlice> &gs) const;

    //指定buf-io对象读写ZL
    LOM_ERR DumpTo(::lom::io::BufWriter &bw, bool need_flush = true) const;
    static LOM_ERR LoadFrom(::lom::io::BufReader &br, ZList &zl);
};

}

}
