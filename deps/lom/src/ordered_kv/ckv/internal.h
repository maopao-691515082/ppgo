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

    class DataFile
    {
        ssize_t id_;
        ::lom::os::File::Ptr file_;

        ssize_t seg_count_ = 0, freed_seg_count_ = 0;

    public:

        typedef ::std::shared_ptr<DataFile> Ptr;

        static const ssize_t kSizeThreshold = 16LL * 1024 * 1024;

        DataFile(ssize_t id, const ::lom::os::File::Ptr &file) : id_(id), file_(file)
        {
        }

        DataFile(ssize_t id, const ::lom::os::File::Ptr &file, ssize_t seg_count, ssize_t freed_seg_count) :
            id_(id), file_(file), seg_count_(seg_count), freed_seg_count_(freed_seg_count)
        {
        }
    };
    typedef ::lom::immut::AVLMap<ssize_t /*id*/, DataFile::Ptr> DataFiles;

    struct Core
    {
        typedef ::std::shared_ptr<Core> Ptr;

        std::mutex write_lock;

        Str path;
        Str serial;

        MetaDB::Ptr meta_db;

        DataFiles data_files;
        ::lom::io::BufWriter::Ptr data_writer;
        ssize_t curr_data_file_sz_ = 0;
    };

    Core::Ptr core_;

public:

    virtual ~DBImpl();

    LOM_ERR Init(const char *path, Options opts);

    virtual LOM_ERR Write(const WriteBatch &wb, std::function<void ()> commit_hook) override;
    virtual ::lom::ordered_kv::Snapshot::Ptr NewSnapshot(std::function<void ()> new_snapshot_hook) override;
};

}

}

}
