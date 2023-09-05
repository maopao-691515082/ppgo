#pragma once

#include "../_internal.h"

#include "../str.h"
#include "../err.h"
#include "../go_slice.h"
#include "../limit.h"

namespace lom
{

namespace ordered_kv
{

//通用的批量写操作的接口
class WriteBatchBase
{
public:

    typedef std::shared_ptr<WriteBatchBase> Ptr;

    virtual ~WriteBatchBase() = default;

    virtual void Clear() = 0;

    virtual void Set(const Str &k, const Str &v) = 0;
    virtual void Del(const Str &k) = 0;
};

//一个默认的批量写操作对象的实现
class WriteBatch : public WriteBatchBase
{
public:

    //通过`v`的含值与否区分是`Set`还是`Del`
    typedef std::map<Str /*k*/, std::optional<Str> /*v*/> RawOpsMap;

private:

    RawOpsMap raw_ops_;

public:

    const RawOpsMap &RawOps() const
    {
        return raw_ops_;
    }

    virtual void Clear() override
    {
        raw_ops_.clear();
    }

    virtual void Set(const Str &k, const Str &v) override
    {
        raw_ops_[k] = v;
    }

    virtual void Del(const Str &k) override
    {
        raw_ops_[k] = std::nullopt;
    }
};

/*
通用的迭代器基类
    - 迭代器机制是将数据库视为KV对+双端边界位置的线性表，在范围内可以双向移动
    - 迭代器应该是只针对不可修改的快照视图，若快照可临时修改（如`Snapshot`的机制），
      则修改后继续访问对应迭代器的行为未定义
    - 建议迭代器实现中保留到对应快照的引用，以保证生命周期内的可用性，如果没有保证，
      则快照被销毁后继续使用对应迭代器的行为未定义
    - 新建立的迭代器应该保证默认指向开始为止，即默认执行`SeekFirst()`
*/
class Iterator
{
    ::lom::Err::Ptr err_;

protected:

    void SetErr(::lom::Err::Ptr err)
    {
        if (!err_)
        {
            err_ = err;
        }
    }

    virtual bool IsLeftBorderImpl() const = 0;
    virtual bool IsRightBorderImpl() const = 0;

    virtual StrSlice KeyImpl() const = 0;
    virtual StrSlice ValueImpl() const = 0;

    virtual void SeekFirstImpl() = 0;
    virtual void SeekLastImpl() = 0;
    virtual void SeekImpl(const Str &k) = 0;
    virtual void SeekPrevImpl(const Str &k)
    {
        Seek(k);
        Prev();
    }

    virtual ssize_t MoveImpl(ssize_t step, const std::optional<Str> &stop_at) = 0;

public:

    typedef std::shared_ptr<Iterator> Ptr;

    virtual ~Iterator() = default;

    /*
    若操作中出现错误，则可通过`Err`获取，且一旦出错，迭代器状态就被锁死，不可进一步操作，
    只能调用`Err`和`Valid`判断状态
    */
    ::lom::Err::Ptr Err() const
    {
        return err_;
    }

    bool IsLeftBorder() const
    {
        return err_ ? false : IsLeftBorderImpl();
    }

    bool IsRightBorder() const
    {
        return err_ ? false : IsRightBorderImpl();
    }

    /*
    在定位、移动等操作后都应该用`Valid`用来判断是否有效，确认有效才可以进行进一步读取
    若`Err`返回空而`Valid`返回false，则说明迭代器到达了边界
    */
    bool Valid() const
    {
        return !(err_ || IsLeftBorder() || IsRightBorder());
    }

    /*
    返回KV，只有在`Valid()`为true的情况下才有意义
        - 在一次调用后，直到迭代器销毁或状态改变之前，返回的两个串的引用应保证可用
            - 最简单的办法就是迭代器对象本身以直接或间接的形式保存KV的不可变的字符串
    */
    StrSlice Key() const
    {
        Assert(Valid());
        return KeyImpl();
    }
    StrSlice Value() const
    {
        Assert(Valid());
        return ValueImpl();
    }

    //定位到第一个K/最后一个K，若空库则定位至右边界/左边界
    void SeekFirst()
    {
        if (!err_)
        {
            SeekFirstImpl();
        }
    }
    void SeekLast()
    {
        if (!err_)
        {
            SeekLastImpl();
        }
    }

    //定位到从左到右第一个大于等于`k`的K，若所有K都比`k`小或为空数据集，则定位到右边界
    void Seek(const Str &k)
    {
        if (!err_)
        {
            SeekImpl(k);
        }
    }
    //相当于先`Seek(k)`再进行`Prev()`，`SeekPrevImpl`默认也是这样实现的，但是具体DB可以覆盖为更高效的实现
    void SeekPrev(const Str &k)
    {
        if (!err_)
        {
            SeekPrevImpl(k);
        }
    }

    /*
    移动迭代器
    通过`step`指定步数，正数向右移动，负数向左
    若`stop_at`不含值，则到达边界停止
    若`stop_at`含值，则在遇到或越过它处停下，即：
        - 若向右移动，则在第一个大于等于它的K处停下
        - 若向左移动，则在第一个小于等于它的K处停下
    若出错，则返回`-1`，否则返回实际移动的步数，说明：
        - 返回的步数和方向无关，是非负的计数值，例如`step`输入`-100`，若能成功向左移动100步，则返回值是`100`
        - 如果当前K就符合`stop_at`的要求，则不做移动（返回0）
    */
    ssize_t Move(ssize_t step, const std::optional<Str> &stop_at = std::nullopt)
    {
        if (err_)
        {
            return -1;
        }

        if (step > kSSizeSoftMax)
        {
            step = kSSizeSoftMax;
        }
        if (step < -kSSizeSoftMax)
        {
            step = -kSSizeSoftMax;
        }

        return step == 0 ? 0 : MoveImpl(step, stop_at);
    }

    //`Move`的各种特化封装，简化使用
    void Next()
    {
        Move(1);
    }
    void Prev()
    {
        Move(-1);
    }
    void MoveTo(const Str &k)
    {
        Move(kSSizeSoftMax, k);
    }
    void RMoveTo(const Str &k)
    {
        Move(-kSSizeSoftMax, k);
    }
};

/*
通用的快照基类
    - 快照对象是“创建快照时的DB数据版本+临时修改空间”的叠加视图，后者叠加在前者上面
        - 例：创建快照时数据集合为`{a:1,b:2}`，则后续DB数据修改不会影响本快照看到的数据，
          如果在快照中临时修改`a=3`，则后续对快照的访问相当于`{a:3,b:2}`，但不影响DB
        - 这种设计可在一定程度上支持更上层的事务机制，典型流程是：
            1 获取快照
            2 在此快照上进行读取、计算、写入操作
            3 如需要提交，则通过`WriteBatch`方法获取临时空间，一次性写入库
              如需要放弃提交，简单丢弃快照对象即可
              如需回滚后重新计算，可对临时空间进行`Clear`
        - 考虑到实现复杂和性能问题，临时空间并不保存历史版本，如果想实现类似提交点和部分回滚，
          可手动维护一个临时空间链表
    - 快照对象需要保证总是可用，一般实现方式就是保存一份到对应库的引用，由于各库的实现差异，
      就不在这个接口类显式做`DBBase`的引用了
*/
class Snapshot : public std::enable_shared_from_this<Snapshot>
{
protected:

    /*
    `Get`操作的DB版本，派生类只需要实现到DB的`Get`即可
    参数和返回说明参考`Get`的注释
    */
    virtual ::lom::Err::Ptr DBGet(
        const Str &k, std::function<void (const StrSlice * /*ptr to v*/)> f) const = 0;
    virtual ::lom::Err::Ptr DBGet(const Str &k, std::function<StrSlice ()> &v) const = 0;

    virtual Iterator::Ptr DBNewIterator() const = 0;

public:

    typedef std::shared_ptr<Snapshot> Ptr;

    virtual ~Snapshot() = default;

    WriteBatch wb;

    //获取值并调用回调函数，`f`的参数为空指针表示没有找到
    ::lom::Err::Ptr Get(const Str &k, std::function<void (const StrSlice * /*ptr to v*/)> f) const;
    /*
    获取值，通过`v`返回一个值的获取器，`v`返回值为空函数表示没有找到
    若`v`有效，则需要保证其总是可用的，即一直引用一份对应的快照数据
    */
    ::lom::Err::Ptr Get(const Str &k, std::function<StrSlice ()> &v) const;

    //创建一个当前快照的迭代器
    Iterator::Ptr NewIterator();
};

//库的接口类
class DBBase
{
public:

    typedef std::shared_ptr<DBBase> Ptr;

    virtual ~DBBase() = default;

    //执行写操作
    virtual ::lom::Err::Ptr Write(const WriteBatch &wb) = 0;

    //创建一个当前DB数据的快照
    virtual Snapshot::Ptr NewSnapshot() = 0;
};

}

}
