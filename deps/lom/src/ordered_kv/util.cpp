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
    class OpsIter
    {
        const WriteBatch::RawOpsMap &ops_;
        std::optional<WriteBatch::RawOpsMap::const_iterator> iter_;

    public:

        OpsIter(const WriteBatch::RawOpsMap &ops) : ops_(ops), iter_(ops_.begin())
        {
        }

        bool IsLeftBorder() const
        {
            return !iter_;
        }

        bool IsRightBorder() const
        {
            return iter_ && *iter_ == ops_.end();
        }

        bool Valid() const
        {
            return iter_ && *iter_ != ops_.end();
        }

        bool IsSet() const
        {
            return Valid() && (*iter_)->second;
        }

        bool IsDel() const
        {
            return Valid() && !(*iter_)->second;
        }

        StrSlice Key() const
        {
            Assert(Valid());
            return (*iter_)->first.Slice();
        }

        StrSlice Value() const
        {
            Assert(IsSet());
            return (*iter_)->second->Slice();
        }

        void Next()
        {
            if (IsLeftBorder())
            {
                iter_ = ops_.begin();
            }
            else if (!IsRightBorder())
            {
                ++ *iter_;
            }
        }

        void Prev()
        {
            if (!IsLeftBorder())
            {
                if (*iter_ == ops_.begin())
                {
                    //to left border
                    iter_ = std::nullopt;
                }
                else
                {
                    -- *iter_;
                }
            }
        }

        void SeekFirst()
        {
            iter_ = ops_.begin();
        }
        void SeekLast()
        {
            if (ops_.empty())
            {
                iter_ = std::nullopt;
            }
            else
            {
                iter_ = ops_.end();
                -- *iter_;
            }
        }

        void Seek(const Str &k)
        {
            iter_ = ops_.lower_bound(k);
        }
    };

    Snapshot::Ptr snapshot_;

    OpsIter ops_iter_;
    Iterator::Ptr db_iter_;

    /*
    rd_=reverse-direction：指示当前方向是否为反向
    若`rd_`为正向（false），则较小的元素是当前元素，反之较大的元素为当前元素
    */
    bool rd_ = false;

    //ops和DB的当前K的大小关系
    int key_cmp_result_;

    //根据当前两个子迭代器的状态和方向指示更新状态，更新后的状态必然是规整化的
    void Update()
    {
        if (Err())
        {
            return;
        }

        if (db_iter_->Err())
        {
            SetErr(db_iter_->Err());
            return;
        }

        auto cmp_k = [this] () {
            if (ops_iter_.IsLeftBorder())
            {
                key_cmp_result_ = db_iter_->IsLeftBorder() ? 0 : -1;
            }
            else if (ops_iter_.IsRightBorder())
            {
                key_cmp_result_ = db_iter_->IsRightBorder() ? 0 : 1;
            }
            else
            {
                key_cmp_result_ =
                    db_iter_->IsLeftBorder() ?
                        1 :
                        (
                            db_iter_->IsRightBorder() ?
                                -1 :
                                ops_iter_.Key().Cmp(db_iter_->Key())
                        )
                ;
            }
        };

        //反复计算当前的K大小关系，并跳过ops中的删除标记，直到不需要跳过
        bool skipped_del = false;
        do
        {
            skipped_del = false;

            cmp_k();

            if (rd_)
            {
                if (key_cmp_result_ >= 0 && ops_iter_.IsDel())
                {
                    ops_iter_.Prev();
                    if (key_cmp_result_ == 0)
                    {
                        db_iter_->Prev();
                    }
                    skipped_del = true;
                }
            }
            else
            {
                if (key_cmp_result_ <= 0 && ops_iter_.IsDel())
                {
                    ops_iter_.Next();
                    if (key_cmp_result_ == 0)
                    {
                        db_iter_->Next();
                    }
                    skipped_del = true;
                }
            }
        } while (skipped_del);
    }

protected:

    virtual bool IsLeftBorderImpl() const override
    {
        return ops_iter_.IsLeftBorder() && db_iter_->IsLeftBorder();
    }
    virtual bool IsRightBorderImpl() const override
    {
        return ops_iter_.IsRightBorder() && db_iter_->IsRightBorder();
    }

    virtual StrSlice KeyImpl() const override
    {
        return
            (rd_ && key_cmp_result_ >= 0) || (!rd_ && key_cmp_result_ <= 0) ?
                ops_iter_.Key() :
                db_iter_->Key();
    }
    virtual StrSlice ValueImpl() const override
    {
        return
            (rd_ && key_cmp_result_ >= 0) || (!rd_ && key_cmp_result_ <= 0) ?
                ops_iter_.Value() :
                db_iter_->Value();
    }

    virtual void SeekFirstImpl() override
    {
        ops_iter_.SeekFirst();
        db_iter_->SeekFirst();
        rd_ = false;
        Update();
    }
    virtual void SeekLastImpl() override
    {
        ops_iter_.SeekLast();
        db_iter_->SeekLast();
        rd_ = true;
        Update();
    }
    virtual void SeekImpl(const Str &k) override
    {
        ops_iter_.Seek(k);
        db_iter_->Seek(k);
        rd_ = false;
        Update();
    }
    virtual void SeekPrevImpl(const Str &k) override
    {
        ops_iter_.Seek(k);
        ops_iter_.Prev();
        db_iter_->SeekPrev(k);
        rd_ = true;
        Update();
    }

    virtual ssize_t MoveImpl(ssize_t step, const std::optional<Str> &stop_at) override
    {
        (void)(step);
        (void)(stop_at);
        //todo
        return 0;
    }

public:

    SnapshotIteratorImpl(const Snapshot::Ptr &snapshot, const Iterator::Ptr &db_iter) :
        snapshot_(snapshot), ops_iter_(snapshot->wb.RawOps()), db_iter_(db_iter)
    {
        Update();
    }
};

Iterator::Ptr Snapshot::NewIterator()
{
    auto db_iter = DBNewIterator();
    if (db_iter->Err() || wb.RawOps().empty())
    {
        return db_iter;
    }
    return Iterator::Ptr(new SnapshotIteratorImpl(shared_from_this(), db_iter));
}

}

}
