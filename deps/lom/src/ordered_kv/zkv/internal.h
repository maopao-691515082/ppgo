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

        virtual LOM_ERR DBGet(const Str &k, std::function<void (const StrSlice *)> f) const override;
        virtual LOM_ERR DBGet(const Str &k, std::function<StrSlice ()> &v) const override;

        virtual ::lom::ordered_kv::Iterator::Ptr DBNewIterator() const override;

    public:

        Snapshot(const ZMap &zm) : zm_(zm)
        {
        }
    };

    struct SnapshotDumpTask
    {
        typedef std::shared_ptr<SnapshotDumpTask> Ptr;

        enum Cmd
        {
            kCmd_None   = 0,
            kCmd_Dump   = 1,
            kCmd_Exit   = 100,
        };

        std::mutex lock_;

        Cmd cmd_ = kCmd_None;

        Str path_;
        ssize_t idx_;
        Str serial_;
        ZMap zm_;
    };

    std::mutex write_lock_, update_lock_;

    ZMap zm_;

    Str path_;
    ::lom::os::File::Ptr lock_file_;
    Str serial_;
    ::lom::os::File::Ptr curr_op_log_file_;
    ssize_t curr_op_log_idx_ = 0;
    SnapshotDumpTask::Ptr dump_task_;

    static LOM_ERR GetFileIdxes(const Str &path, GoSlice<ssize_t> &snapshot_idxes, GoSlice<ssize_t> &op_log_idxes);
    LOM_ERR GetFileIdxes(GoSlice<ssize_t> &snapshot_idxes, GoSlice<ssize_t> &op_log_idxes) const;

    LOM_ERR CreateMetaFile() const;
    LOM_ERR LoadMetaFile();

    static LOM_ERR DumpSnapshotFile(const Str &path, ssize_t idx, const Str &serial, ZMap zm);
    LOM_ERR NewOpLogFile();
    LOM_ERR LoadDataFromFiles(ssize_t snapshot_idx, ssize_t max_op_log_idx);

    static void DumpThreadMain(std::function<void (LOM_ERR)> handle_bg_err, SnapshotDumpTask::Ptr task);

public:

    LOM_ERR Init(const char *path, Options opts);

    virtual LOM_ERR Write(const WriteBatch &wb) override;
    virtual ::lom::ordered_kv::Snapshot::Ptr NewSnapshot() override;
};

}

}

}
