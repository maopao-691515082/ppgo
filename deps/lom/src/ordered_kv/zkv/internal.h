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
    typedef ::lom::immut::AVLMap<Str, ::lom::immut::ZList> ZMap;

    class Snapshot : public ::lom::ordered_kv::Snapshot
    {
        ZMap zm_;

    protected:

        virtual ::lom::Err::Ptr DBGet(const Str &k, std::function<void (const StrSlice *)> f) const override;
        virtual ::lom::Err::Ptr DBGet(const Str &k, std::function<StrSlice ()> &v) const override;

        virtual Iterator::Ptr DBNewIterator() const override;

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
