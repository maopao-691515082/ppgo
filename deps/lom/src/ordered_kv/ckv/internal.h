#pragma once

#include "../../internal.h"

namespace lom
{

namespace ordered_kv
{

namespace ckv
{

namespace experimental
{

class DBImpl : public DB
{
    class DataFile
    {
    public:

        typedef ::std::shared_ptr<DataFile> Ptr;

    private:

        ssize_t id_;
        ::lom::os::File::Ptr file_;

        //文件中的总seg数量（包括释放的），和其中已释放的seg数量
        ssize_t seg_count_ = 0, freed_seg_count_ = 0;

        Ptr AdjustSegCounts(ssize_t inc_seg_count, ssize_t inc_freed_seg_count) const
        {
            return Ptr(new DataFile(
                id_, file_, seg_count_ + inc_seg_count, freed_seg_count_ + inc_freed_seg_count));
        }

    public:

        static const ssize_t kSizeThreshold = 16LL * 1024 * 1024;

        DataFile(ssize_t id, const ::lom::os::File::Ptr &file) : id_(id), file_(file)
        {
        }

        DataFile(ssize_t id, const ::lom::os::File::Ptr &file, ssize_t seg_count, ssize_t freed_seg_count) :
            id_(id), file_(file), seg_count_(seg_count), freed_seg_count_(freed_seg_count)
        {
            Assert(seg_count_ >= 0 && freed_seg_count_ >= 0);
        }

        ssize_t ID() const
        {
            return id_;
        }
        ::lom::io::BufReader::Ptr NewPReadBufReader(ssize_t off) const
        {
            return ::lom::io::BufReader::New(
                [f = file_, off] (char *buf, ssize_t sz, ssize_t &rsz) mutable -> LOM_ERR {
                    auto err = f->PRead(off, buf, sz, rsz);
                    if (!err)
                    {
                        off += rsz;
                    }
                    return err;
                }
            );
        }
        ssize_t SegCount() const
        {
            return seg_count_;
        }
        ssize_t FreedSegCount() const
        {
            return freed_seg_count_;
        }

        Ptr IncSegCount() const
        {
            return AdjustSegCounts(1, 0);
        }
        Ptr IncFreeSegCount() const
        {
            return AdjustSegCounts(0, 1);
        }

        bool NeedGC() const
        {
            //当被释放的seg数量超出一半时视为需要GC
            return freed_seg_count_ > seg_count_ / 2;
        }
    };
    typedef ::lom::immut::AVLMap<ssize_t /*id*/, DataFile::Ptr> DataFiles;

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

        ::lom::ordered_kv::Snapshot::Ptr meta_snapshot_;
        DataFiles data_files_;

    protected:

        virtual LOM_ERR DBGet(const Str &k, std::function<void (const StrSlice *)> const &f) const override;
        virtual LOM_ERR DBGet(const Str &k, std::function<StrSlice ()> &v) const override;

        virtual ::lom::ordered_kv::Iterator::Ptr DBNewIterator() const override;

    public:

        Snapshot(const ::lom::ordered_kv::Snapshot::Ptr &meta_snapshot, const DataFiles &data_files) :
            meta_snapshot_(meta_snapshot), data_files_(data_files)
        {
        }
    };

    struct Core
    {
        typedef ::std::shared_ptr<Core> Ptr;

        std::mutex write_lock;

        Str path;
        Str serial;

        MetaDB::Ptr meta_db;

        DataFiles data_files;
        ::lom::io::BufWriter::Ptr data_writer;

        std::atomic<bool> stopped;

        Core() : stopped(false)
        {
        }
    };

    Core::Ptr core_;

    static void GCThreadMain(std::function<void (LOM_ERR)> const &handle_bg_err, Core::Ptr db_core);

public:

    DBImpl() : core_(new Core())
    {
    }

    virtual ~DBImpl();

    LOM_ERR Init(const char *path, Options opts);

    virtual LOM_ERR Write(const WriteBatch &wb, std::function<void ()> const &commit_hook) override;
    virtual ::lom::ordered_kv::Snapshot::Ptr NewSnapshot(std::function<void ()> const &new_snapshot_hook) override;
};

}

}

}

}
