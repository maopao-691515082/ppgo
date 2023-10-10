#include "internal.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

static const Str
    kMetaFileName = "META",
    kSnapshotFileNamePrefix = "SNAPSHOT-",
    kOpLogFileNamePrefix = "OP_LOG-";

LOM_ERR DBImpl::GetFileIdxes(
    const Str &path, GoSlice<ssize_t> &ret_snapshot_idxes, GoSlice<ssize_t> &ret_op_log_idxes)
{
    GoSlice<ssize_t> snapshot_idxes, op_log_idxes;
    GoSlice<Str> file_names;
    LOM_RET_ON_ERR(::lom::os::ListDir(path.CStr(), file_names));
    for (auto const &file_name : file_names)
    {
        if (file_name.HasPrefix(kSnapshotFileNamePrefix))
        {
            auto snapshot_idx_str = file_name.SubStr(kSnapshotFileNamePrefix.Len());
            int64_t si;
            if (snapshot_idx_str.ParseInt64(si, 10) &&
                si > 0 && si < kSSizeSoftMax &&
                snapshot_idx_str == ::lom::Str::FromInt64(si))
            {
                snapshot_idxes = snapshot_idxes.Append(si);
            }
        }
        if (file_name.HasPrefix(kOpLogFileNamePrefix))
        {
            auto op_log_idx_str = file_name.SubStr(kOpLogFileNamePrefix.Len());
            int64_t oi;
            if (op_log_idx_str.ParseInt64(oi, 10) &&
                oi > 0 && oi < kSSizeSoftMax &&
                op_log_idx_str == ::lom::Str::FromInt64(oi))
            {
                op_log_idxes = op_log_idxes.Append(oi);
            }
        }
    }
    snapshot_idxes.Sort().Reverse();
    op_log_idxes.Sort().Reverse();

    ret_snapshot_idxes = snapshot_idxes;
    ret_op_log_idxes = op_log_idxes;
    return nullptr;
}

LOM_ERR DBImpl::GetFileIdxes(GoSlice<ssize_t> &snapshot_idxes, GoSlice<ssize_t> &op_log_idxes) const
{
    return GetFileIdxes(path_, snapshot_idxes, op_log_idxes);
}

LOM_ERR DBImpl::CreateMetaFile() const
{
    auto meta_file_path = path_.Concat("/").Concat(kMetaFileName);
    auto tmp_meta_file_path = meta_file_path.Concat(".tmp");
    ::lom::os::File::Ptr meta_file;
    LOM_RET_ON_ERR(::lom::os::File::Open(tmp_meta_file_path.CStr(), meta_file, "w", 0600));
    auto bw = ::lom::io::BufWriter::New(
        [meta_file] (const char *buf, ssize_t sz, ssize_t &wsz) -> LOM_ERR {
            return meta_file->Write(buf, sz, wsz);
        }
    );
    LOM_RET_ON_ERR(::lom::immut::ZList().Append(serial_).DumpTo(bw));
    LOM_RET_ON_ERR(::lom::os::Rename(tmp_meta_file_path.CStr(), meta_file_path.CStr()));
    return nullptr;
}

LOM_ERR DBImpl::LoadMetaFile()
{
    auto meta_file_path = path_.Concat("/").Concat(kMetaFileName);
    ::lom::os::File::Ptr meta_file;
    LOM_RET_ON_ERR(::lom::os::File::Open(meta_file_path.CStr(), meta_file));
    auto br = ::lom::io::BufReader::New(
        [meta_file] (char *buf, ssize_t sz, ssize_t &rsz) -> LOM_ERR {
            return meta_file->Read(buf, sz, rsz);
        }
    );
    ::lom::immut::ZList zl;
    LOM_RET_ON_ERR(::lom::immut::ZList::LoadFrom(br, zl));
    if (zl.StrCount() < 1)
    {
        LOM_RET_ERR("invalid meta file");
    }
    serial_ = zl.FirstStr();
    return nullptr;
}

void DBImpl::DumpThreadMain(std::function<void (LOM_ERR)> handle_bg_err, SnapshotDumpTask::Ptr task)
{
    for (;;)
    {
        bool ok = false;
        Str path;
        ssize_t idx;
        Str serial;
        ZMap zm;
        {
            std::lock_guard<std::mutex> lg(task->lock_);
            if (task->cmd_ == SnapshotDumpTask::kCmd_Exit)
            {
                return;
            }
            if (task->cmd_ == SnapshotDumpTask::kCmd_Dump)
            {
                //pop task
                path = task->path_;
                idx = task->idx_;
                serial = task->serial_;
                zm = task->zm_;
                task->zm_ = ZMap();

                ok = true;
                task->cmd_ = SnapshotDumpTask::kCmd_None;
            }
        }
        if (!ok)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        auto dump_and_purge = [&] () -> LOM_ERR {
            LOM_RET_ON_ERR(DumpSnapshotFile(path, idx, serial, zm));

            GoSlice<ssize_t> snapshot_idxes, op_log_idxes;
            LOM_RET_ON_ERR(GetFileIdxes(path, snapshot_idxes, op_log_idxes));
            if (snapshot_idxes.Len() == 0)
            {
                LOM_RET_ERR("snapshot lost");
            }

            auto purge_file = [&] (const Str &name) {
                auto path_name = path.Concat("/").Concat(name);
                auto err = ::lom::os::RemoveTree(path_name.CStr());
                if (err && handle_bg_err)
                {
                    handle_bg_err(err);
                }
            };

            auto snapshot_idx = snapshot_idxes.At(0);
            for (auto si : snapshot_idxes)
            {
                if (si < snapshot_idx)
                {
                    purge_file(kSnapshotFileNamePrefix.Concat(Str::FromInt64(si)));
                }
            }
            for (auto oi : op_log_idxes)
            {
                if (oi < snapshot_idx)
                {
                    purge_file(kOpLogFileNamePrefix.Concat(Str::FromInt64(oi)));
                }
            }

            return nullptr;
        };

        auto err = dump_and_purge();
        if (err && handle_bg_err)
        {
            handle_bg_err(err);
        }
    }
}

LOM_ERR DBImpl::Init(const char *path_str, Options opts)
{
    if (path_str == nullptr)
    {
        //memory mode
        return nullptr;
    }

    //disk mode, init

    path_ = ::lom::os::Path(path_str).Str();

    //open and lock
    bool new_db_created = false;
    {
        auto lock_file_path = path_.Concat("/LOCK");
        ::lom::os::FileStat fst;
        LOM_RET_ON_ERR(::lom::os::FileStat::Stat(lock_file_path.CStr(), fst));
        if (fst.Exists())
        {
            LOM_RET_ON_ERR(::lom::os::File::Open(lock_file_path.CStr(), lock_file_, "r+"));
        }
        else
        {
            if (!opts.create_if_missing)
            {
                LOM_RET_ERR("zkv db not found at `%s`", path_.CStr());
            }
            LOM_RET_ON_ERR(::lom::os::MakeDirs(path_.CStr()));
            LOM_RET_ON_ERR(::lom::os::File::Open(lock_file_path.CStr(), lock_file_, "w+x", 0600));
            new_db_created = true;
        }
        bool ok;
        LOM_RET_ON_ERR(lock_file_->TryLock(ok));
        if (!ok)
        {
            LOM_RET_ERR("zkv db `%s` is opened by another", path_.CStr());
        }
    }

    if (new_db_created)
    {
        //init
        serial_ = ::lom::Sprintf(
            "ZKV-%zu-%zu",
            static_cast<size_t>(::lom::NowUS()),
            static_cast<size_t>(::lom::RandU64()));
        LOM_RET_ON_ERR(NewOpLogFile());
        LOM_RET_ON_ERR(DumpSnapshotFile(path_, curr_op_log_idx_, serial_, zm_));
        LOM_RET_ON_ERR(CreateMetaFile());
    }
    else
    {
        //load data
        LOM_RET_ON_ERR(LoadMetaFile());
        GoSlice<ssize_t> snapshot_idxes, op_log_idxes;
        LOM_RET_ON_ERR(GetFileIdxes(snapshot_idxes, op_log_idxes));
        if (snapshot_idxes.Len() == 0)
        {
            LOM_RET_ERR("no snapshot file found");
        }
        if (op_log_idxes.Len() == 0)
        {
            LOM_RET_ERR("no op-log file found");
        }
        ssize_t snapshot_idx = snapshot_idxes.At(0), max_op_log_idx = op_log_idxes.At(0);
        if (snapshot_idx > max_op_log_idx)
        {
            LOM_RET_ERR("no op-log file found since snapshot #%zd", snapshot_idx);
        }
        LOM_RET_ON_ERR(LoadDataFromFiles(snapshot_idx, max_op_log_idx));
    }

    //start dump thread
    {
        dump_task_ = std::make_shared<SnapshotDumpTask>();
        std::thread([handle_bg_err = opts.handle_bg_err, task = dump_task_] () {
            DumpThreadMain(handle_bg_err, task);
        }).detach();
    }

    return nullptr;
}

LOM_ERR DBImpl::Write(const WriteBatch &wb)
{
    static const ssize_t
        kZLDataLenMax = 1024,
        kZLDataLenMergeThreshold = kZLDataLenMax / 2;

    auto const &wb_ops = wb.RawOps();
    if (wb_ops.empty())
    {
        return nullptr;
    }

    //保存一下老的`zm_`，在锁外释放空间
    ZMap old_zm;

    {
        std::lock_guard<std::mutex> wlg(write_lock_);

        //这里已经保证了没有人修改`zm_`，所以不加`update_lock_`直接读
        auto zm = zm_;
        old_zm = zm;

        //在`zm`上执行写操作
        auto wb_ops_iter = wb_ops.begin();
        while (wb_ops_iter != wb_ops.end())
        {
            //根据WB中当前要处理的K找到对应的需要合并的ZL，以及其后继的ZL信息
            ::lom::immut::ZList zl;
            std::optional<Str> next_zl_first_k;
            ::lom::immut::ZList next_zl;
            ssize_t merge_idx = 0;  //保持指向当前合并的ZL应该插入的位置
            if (zm.Size() > 0)
            {
                auto v_ptr = zm.Get(wb_ops_iter->first, &merge_idx);
                if (v_ptr == nullptr)
                {
                    //找到插入位置，使用前驱的ZL，若无前驱则使用当前ZL
                    if (merge_idx > 0)
                    {
                        -- merge_idx;
                    }
                    auto pp = zm.GetByIdx(merge_idx);
                    zl = *pp.second;
                }
                else
                {
                    //恰好有一个以WB的当前K开头的ZL
                    zl = *v_ptr;
                }

                //删掉这个ZL
                zm = zm.DelByIdx(merge_idx);

                //删除后，当前`merge_idx`是ZL的后继，获取后继的信息
                if (merge_idx < zm.Size())
                {
                    auto pp = zm.GetByIdx(merge_idx);
                    next_zl_first_k = *pp.first;
                    next_zl = *pp.second;
                }
            }

            //进行合并
            Assert(zl.StrCount() % 2 == 0);
            ::lom::immut::ZList::Iterator zl_iter(zl);
            GoSlice<StrSlice> merged_kvs;
            ssize_t merged_kvs_data_len = 0;
            for (;;)
            {
                StrSlice k, v;

                if (wb_ops_iter != wb_ops.end())
                {
                    auto wb_ops_k = wb_ops_iter->first.Slice();
                    if (zl_iter.Valid())
                    {
                        //两边都有值
                        int cmp_result = wb_ops_k.Cmp(zl_iter.Get());
                        if (cmp_result < 0)
                        {
                            if (!wb_ops_iter->second)
                            {
                                ++ wb_ops_iter;
                                continue;
                            }
                            k = wb_ops_k;
                            v = wb_ops_iter->second->Slice();
                            ++ wb_ops_iter;
                        }
                        else if (cmp_result > 0)
                        {
                            k = zl_iter.Get();
                            zl_iter.Next();
                            Assert(zl_iter.Valid());
                            v = zl_iter.Get();
                            zl_iter.Next();
                        }
                        else
                        {
                            zl_iter.Next();
                            Assert(zl_iter.Valid());
                            zl_iter.Next();
                            if (!wb_ops_iter->second)
                            {
                                ++ wb_ops_iter;
                                continue;
                            }
                            k = wb_ops_k;
                            v = wb_ops_iter->second->Slice();
                            ++ wb_ops_iter;
                        }
                    }
                    else
                    {
                        if (next_zl_first_k && wb_ops_k >= next_zl_first_k->Slice())
                        {
                            //当前ZL已经merge完毕
                            break;
                        }
                        if (!wb_ops_iter->second)
                        {
                            ++ wb_ops_iter;
                            continue;
                        }
                        k = wb_ops_k;
                        v = wb_ops_iter->second->Slice();
                        ++ wb_ops_iter;
                    }
                }
                else
                {
                    if (zl_iter.Valid())
                    {
                        k = zl_iter.Get();
                        zl_iter.Next();
                        Assert(zl_iter.Valid());
                        v = zl_iter.Get();
                        zl_iter.Next();
                    }
                    else
                    {
                        //both end
                        break;
                    }
                }

                ssize_t add_data_len = 4 + k.Len() + v.Len();
                if (merged_kvs_data_len > 0 && merged_kvs_data_len + add_data_len > kZLDataLenMax)
                {
                    //即将超限，新生成ZL
                    Str new_zl_first_k = merged_kvs.At(0);
                    ssize_t len_soft_max_kv_count = merged_kvs.Len() / 2;
                    if (len_soft_max_kv_count >= 3)
                    {
                        len_soft_max_kv_count = len_soft_max_kv_count * 2 / 3;
                    }
                    auto new_zl = ::lom::immut::ZList().Extend(
                        merged_kvs.Slice(0, len_soft_max_kv_count * 2));
                    merged_kvs = merged_kvs.Slice(len_soft_max_kv_count * 2);
                    merged_kvs_data_len = 0;
                    for (ssize_t i = 0; i < merged_kvs.Len(); ++ i)
                    {
                        merged_kvs_data_len += 2 + merged_kvs.At(i).Len();
                    }

                    //插入
                    zm = zm.AddByIdx(merge_idx, new_zl_first_k, new_zl);
                    ++ merge_idx;
                }
                if (merged_kvs.Len() > 0)
                {
                    Assert(k > merged_kvs.At(merged_kvs.Len() - 2));
                }
                merged_kvs = merged_kvs.Append(k).Append(v);
                merged_kvs_data_len += add_data_len;
            }

            //尾部处理
            if (merged_kvs_data_len > 0)
            {
                Str new_zl_first_k = merged_kvs.At(0);
                auto new_zl = ::lom::immut::ZList().Extend(merged_kvs);
                merged_kvs = merged_kvs.Nil();
                merged_kvs_data_len = 0;

                //计算一下是否需要和后继进行合并
                if (next_zl_first_k && new_zl.SpaceCost() + next_zl.SpaceCost() < kZLDataLenMergeThreshold)
                {
                    //需要合并，删掉后继ZL，链接到新的ZL后面
                    zm = zm.DelByIdx(merge_idx);
                    new_zl = new_zl.Extend(next_zl);
                }

                //插入
                zm = zm.AddByIdx(merge_idx, new_zl_first_k, new_zl);
                ++ merge_idx;
            }
        }

        //更新`zm_`
        {
            std::lock_guard<std::mutex> ulg(update_lock_);
            zm_ = zm;
        }
    }

    return nullptr;
}

::lom::ordered_kv::Snapshot::Ptr DBImpl::NewSnapshot()
{
    ZMap zm;
    {
        std::lock_guard<std::mutex> lg(update_lock_);
        zm = zm_;
    }
    return ::lom::ordered_kv::Snapshot::Ptr(new Snapshot(zm));
}

LOM_ERR DB::Open(const char *path, DB::Ptr &db, DB::Options opts)
{
    auto new_db = std::make_shared<DBImpl>();
    LOM_RET_ON_ERR(new_db->Init(path, opts));
    db = std::static_pointer_cast<DB>(new_db);
    return nullptr;
}

}

}

}
