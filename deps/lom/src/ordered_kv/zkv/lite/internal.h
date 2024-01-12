#pragma once

#include "../../../internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

namespace experimental
{

class DBLiteImpl : public DBLite
{
    class Snapshot : public ::lom::ordered_kv::Snapshot
    {
        class Iterator : public ::lom::ordered_kv::Iterator
        {
            ZMap zm_;

            ssize_t zm_idx_ = 0;

            const ::lom::GoSlice<Str> *gs_ = nullptr;
            ssize_t gs_idx_ = 0;

            void Reset(ssize_t zm_idx, bool seek_gs_last = false)
            {
                zm_idx_ = zm_idx;
                if (0 <= zm_idx_ && zm_idx_ < zm_.Size())
                {
                    gs_ = zm_.GetByIdx(zm_idx_).second;
                    Assert(gs_->Len() > 0 && gs_->Len() % 2 == 0);
                    gs_idx_ = seek_gs_last ? gs_->Len() - 2 : 0;
                }
                else
                {
                    gs_ = nullptr;
                    gs_idx_ = 0;
                }
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
                return gs_->At(gs_idx_).Slice();
            }
            virtual StrSlice ValueImpl() const override
            {
                return gs_->At(gs_idx_ + 1).Slice();
            }

            virtual void SeekFirstImpl() override
            {
                Reset(0);
            }
            virtual void SeekLastImpl() override
            {
                Reset(zm_.Size() - 1, true);
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

    protected:

        virtual LOM_ERR DBGet(const Str &k, std::function<void (const StrSlice *)> const &f) const override;
        virtual LOM_ERR DBGet(const Str &k, std::function<StrSlice ()> &v) const override;

        virtual ::lom::ordered_kv::Iterator::Ptr DBNewIterator() const override;
        //virtual ::lom::ordered_kv::ForwardIterator::Ptr DBNewForwardIterator() const override;

    public:

        Snapshot(const ZMap &zm) : zm_(zm)
        {
        }
    };

    std::mutex write_lock_, update_lock_;

    ZMap zm_;

public:

    virtual LOM_ERR Write(const WriteBatch &wb) override;

    virtual Snapshot::Ptr NewSnapshot() override;

    virtual ZMap RawZMap() override;
};

}

}

}

}
