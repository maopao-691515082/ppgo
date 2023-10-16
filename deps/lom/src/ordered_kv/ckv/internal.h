#pragma once

#include "../../internal.h"

namespace lom
{

namespace ordered_kv
{

namespace ckv
{

class DBImpl : public DB
{
    class Snapshot : public ::lom::ordered_kv::Snapshot
    {
        class Iterator : public ::lom::ordered_kv::Iterator
        {
        protected:

            virtual bool IsLeftBorderImpl() const override
            {
                //todo
            }
            virtual bool IsRightBorderImpl() const override
            {
                //todo
            }

            virtual StrSlice KeyImpl() const override
            {
                //todo
            }
            virtual StrSlice ValueImpl() const override
            {
                //todo
            }

            virtual void SeekFirstImpl() override
            {
                //todo
            }
            virtual void SeekLastImpl() override
            {
                //todo
            }

            virtual void SeekImpl(const Str &k) override;
            virtual void SeekPrevImpl(const Str &k) override;

            virtual ssize_t MoveImpl(ssize_t step, const std::optional<Str> &stop_at) override;

        public:

            Iterator(/*todo*/)
            {
                //todo
            }
        };

    protected:

        virtual LOM_ERR DBGet(const Str &k, std::function<void (const StrSlice *)> f) const override;
        virtual LOM_ERR DBGet(const Str &k, std::function<StrSlice ()> &v) const override;

        virtual ::lom::ordered_kv::Iterator::Ptr DBNewIterator() const override;

    public:

        Snapshot(/*todo*/)
        {
            //todo
        }
    };

    MetaDB::Ptr meta_db_;

public:

    virtual ~DBImpl();

    LOM_ERR Init(const char *path, Options opts);

    virtual LOM_ERR Write(const WriteBatch &wb) override;
    virtual ::lom::ordered_kv::Snapshot::Ptr NewSnapshot() override;
};

}

}

}
