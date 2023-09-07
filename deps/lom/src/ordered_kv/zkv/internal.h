#pragma once

#include "../../internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

class DBImpl : public DB
{
    typedef ::lom::immut::AVL<Str, ::lom::immut::ZList> ZMap;

    class Snapshot : public ::lom::ordered_kv::Snapshot
    {
        class Iterator : public ::lom::ordered_kv::Iterator
        {
            ZMap zm_;

            ssize_t zm_idx_ = 0;
            ::lom::immut::ZList::Iterator zl_iter_;

            //若指定zl_iter，则合法性由调用者保证
            void Reset(ssize_t zm_idx, const ::lom::immut::ZList::Iterator *zl_iter = nullptr)
            {
                zm_idx_ = zm_idx;
                zl_iter_ =
                    zl_iter ?
                        *zl_iter :
                        (
                            0 <= zm_idx_ && zm_idx_ < zm_.Size() ?
                                ::lom::immut::ZList::Iterator(*zm_.GetByIdx(zm_idx_).second) :
                                ::lom::immut::ZList::Iterator()
                        )
                ;
            }

            void SeekZLIterLast(::lom::immut::ZList::Iterator &zl_iter)
            {
                Assert(zl_iter.StrCount() > 0 && zl_iter.StrCount() % 2 == 0);
                zl_iter.Seek(zl_iter.StrCount() - 2);
            }
            void SeekZLIterLast()
            {
                SeekZLIterLast(zl_iter_);
            }

        protected:

            virtual bool IsLeftBorderImpl() const override
            {
                return zm_idx_ < 0;
            }
            virtual bool IsRightBorderImpl() const override
            {
                return zm_idx_ >= zm_.Size();
            }

            virtual StrSlice KeyImpl() const override
            {
                return zl_iter_.Get();
            }
            virtual StrSlice ValueImpl() const override
            {
                return zl_iter_.Get(1);
            }

            virtual void SeekFirstImpl() override
            {
                Reset(0);
            }
            virtual void SeekLastImpl() override
            {
                Reset(zm_.Size() - 1);
                if (zm_idx_ >= 0)
                {
                    SeekZLIterLast();
                }
            }

            virtual void SeekImpl(const Str &k) override;
            virtual void SeekPrevImpl(const Str &k) override;

            virtual ssize_t MoveImpl(ssize_t step, const std::optional<Str> &stop_at) override;

        public:

            Iterator(const ZMap &zm) : zm_(zm)
            {
                Reset(0);
            }
        };

        ZMap zm_;

        void DBGet(const Str &k, std::function<void (std::function<bool (StrSlice &)>, StrSlice)> f) const;

    protected:

        virtual ::lom::Err::Ptr DBGet(const Str &k, std::function<void (const StrSlice *)> f) const override;
        virtual ::lom::Err::Ptr DBGet(const Str &k, std::function<StrSlice ()> &v) const override;

        virtual ::lom::ordered_kv::Iterator::Ptr DBNewIterator() const override;

    public:

        Snapshot(const ZMap &zm) : zm_(zm)
        {
        }
    };

    std::mutex write_lock_, update_lock_;

    ZMap zm_;

public:

    DBImpl(const char *path, Options opts, ::lom::Err::Ptr &err);

    virtual ::lom::Err::Ptr Write(const WriteBatch &wb) override;
    virtual ::lom::ordered_kv::Snapshot::Ptr NewSnapshot() override;
};

}

}

}
