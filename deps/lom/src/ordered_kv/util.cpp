#include "../internal.h"

namespace lom
{

namespace ordered_kv
{

::lom::Err::Ptr Snapshot::Get(const Str &k, std::function<void (const StrSlice * /*ptr to v*/)> f) const
{
    auto iter = wb.RawOps().find(k);
    if (iter == wb.RawOps().end())
    {
        return DBGet(k, f);
    }

    if (iter->second)
    {
        auto s = iter->second->Slice();
        f(&s);
    }
    else
    {
        f(nullptr);
    }

    return nullptr;
}

::lom::Err::Ptr Snapshot::Get(const Str &k, std::function<StrSlice ()> &v) const
{
    auto iter = wb.RawOps().find(k);
    if (iter == wb.RawOps().end())
    {
        return DBGet(k, v);
    }

    if (iter->second)
    {
        v = [s = *iter->second] () -> StrSlice {
            return s.Slice();
        };
    }
    else
    {
        v = nullptr;
    }

    return nullptr;
}

class SnapshotIteratorImpl : public Iterator
{
    Snapshot::Ptr snapshot_;

    WriteBatch::RawOpsMap::const_iterator wb_iter_;
    bool before_begin_ = false;

    Iterator::Ptr db_iter_;

protected:

    virtual bool ValidImpl() const override
    {
        return db_iter_->Valid();
    }

    virtual std::function<std::pair<StrSlice /*k*/, StrSlice /*v*/> ()> KVGetterImpl() const = 0;

    virtual void SeekFirstImpl() = 0;
    virtual void SeekLastImpl() = 0;
    virtual void SeekImpl(const Str &k) = 0;
    virtual void RSeekImpl(const Str &k) = 0;

    virtual ssize_t MoveImpl(ssize_t step, const std::optional<Str> &stop_at) = 0;

public:

    SnapshotIteratorImpl(Snapshot::Ptr snapshot, Iterator::Ptr db_iter) :
        snapshot_(snapshot), wb_iter_(snapshot->wb.RawOps().begin()), db_iter_(db_iter)
    {
    }
};

Iterator::Ptr Snapshot::NewIterator() const
{
    auto db_iter = DBNewIterator();
    if (db_iter->Err() || wb.RawOps().empty())
    {
        return db_iter;
    }

    if (!db_iter->Valid())
    {
        db_iter = nullptr;
    }
    return Iterator::Ptr(new SnapshotIteratorImpl(shared_from_this(), db_iter));
}

}

}
