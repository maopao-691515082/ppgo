#include <iostream>

#include "../../include/lom.h"

int main(int argc, char *argv[])
{
    ssize_t test_count = 10000;
    if (argc >= 3)
    {
        int64_t n;
        if (!(lom::Str(argv[2]).ParseInt64(n) && n > 0))
        {
            lom::Die("invalid test count");
        }
        test_count = n;
    }

    lom::ordered_kv::DBBase::Ptr db;
    lom::Err::Ptr err;
    if (argc >= 2)
    {
        if (strcmp(argv[1], "zkv-mem") == 0)
        {
            lom::ordered_kv::zkv::DB::Ptr zkv_db;
            err = lom::ordered_kv::zkv::DB::Open(zkv_db);
            db = std::dynamic_pointer_cast<lom::ordered_kv::DBBase>(zkv_db);
        }
    }
    if (err)
    {
        lom::Die(lom::Sprintf("open db failed: %s", err->Msg().CStr()));
    }
    if (!db)
    {
        lom::Die("unknown db type");
    }

    std::map<lom::Str, lom::Str> smkv;
    {
        auto db_iter = db->NewSnapshot()->NewIterator();
        while (db_iter->Valid())
        {
            auto k = db_iter->Key(), v = db_iter->Value();
            if (smkv.count(k) > 0)
            {
                lom::Die(lom::Sprintf("load db failed, dup key [%s]", k.Repr().CStr()));
            }
            smkv[k] = v;
            db_iter->Next();
        }
        if (db_iter->Err())
        {
            lom::Die(lom::Sprintf("load db failed: %s", db_iter->Err()->Msg().CStr()));
        }
        std::cout << "load ok, kv count: " << smkv.size() << std::endl;
    }

    ssize_t kv_op_count = 0;
    double last_log_time = 0.0;
    auto snapshot_sync_with_db = db->NewSnapshot();  //和db保持一致的snapshot修改版
    for (ssize_t i = 0; i < test_count; ++ i)
    {
        double now = lom::NowFloat();
        if (now - last_log_time >= 1.0)
        {
            std::cout << "completed " << i << " kvs write op, " << kv_op_count << " kv write op" << std::endl;
            last_log_time = now;
        }

        lom::ordered_kv::WriteBatch wb;
        auto n = (ssize_t)lom::RandN(32);
        while (wb.Size() < n)
        {
            auto
                k = lom::Sprintf("key:%zu", static_cast<size_t>(lom::RandN(10000000))),
                v = lom::Sprintf("%zu", static_cast<size_t>(lom::RandN(10000000)));
            bool is_del = lom::RandN(4) == 0;
            if (is_del)
            {
                wb.Del(k);
                snapshot_sync_with_db->wb.Del(k);
            }
            else
            {
                wb.Set(k, v);
                snapshot_sync_with_db->wb.Set(k, v);
            }
        }
        kv_op_count += n;

        err = db->Write(wb);
        if (err)
        {
            lom::Die(lom::Sprintf("write db failed, error [%s]", err->Msg().CStr()));
        }

        for (auto const &p : wb.RawOps())
        {
            if (p.second)
            {
                smkv[p.first] = *p.second;
            }
            else
            {
                smkv.erase(p.first);
            }
        }
    }

    {
        auto iter = smkv.begin();
        auto db_iter = snapshot_sync_with_db->NewIterator();
        while (db_iter->Valid())
        {
            if (iter == smkv.end())
            {
                lom::Die(lom::Sprintf(
                    "k %s itered from snapshot_sync_with_db after smkv iter end",
                    db_iter->Key().Repr().CStr()));
            }
            if (!(db_iter->Key() == iter->first.Slice() && db_iter->Value() == iter->second.Slice()))
            {
                lom::Die(lom::Sprintf(
                    "k %s v %s itered from snapshot_sync_with_db, but not equal to smkv %s %s",
                    db_iter->Key().Repr().CStr(), db_iter->Value().Repr().CStr(),
                    iter->first.Slice().Repr().CStr(), iter->second.Slice().Repr().CStr()));
            }
            ++ iter;
            db_iter->Next();
        }
        if (db_iter->Err())
        {
            lom::Die(lom::Sprintf(
                "snapshot_sync_with_db iter ends with error %s", db_iter->Err()->Msg().CStr()));
        }
        if (iter != smkv.end())
        {
            lom::Die(lom::Sprintf("snapshot_sync_with_db iter ends before smkv"));
        }
    }

    auto snapshot = db->NewSnapshot();
    for (ssize_t i = 0; i < test_count; ++ i)
    {
        double now = lom::NowFloat();
        if (now - last_log_time >= 1.0)
        {
            std::cout << "completed " << i << " by-k read op" << std::endl;
            last_log_time = now;
        }

        auto k = lom::Sprintf("key:%zu", static_cast<size_t>(lom::RandN(10000000)));

        //边读snapshot边删除，测试snapshot的快照机制
        lom::ordered_kv::WriteBatch wb;
        wb.Del(k);
        err = db->Write(wb);
        if (err)
        {
            lom::Die(lom::Sprintf("write db failed, error [%s]", err->Msg().CStr()));
        }

        err = snapshot->Get(k, [&] (const lom::StrSlice *v) {
            auto iter = smkv.find(k);
            if (v == nullptr)
            {
                if (iter != smkv.end())
                {
                    lom::Die(lom::Sprintf("key %s not in db but in smkv", k.Repr().CStr()));
                }
            }
            else
            {
                if (iter == smkv.end())
                {
                    lom::Die(lom::Sprintf("key %s in db but not in smkv", k.Repr().CStr()));
                }
                if (iter->second.Slice() != *v)
                {
                    lom::Die(lom::Sprintf(
                        "key %s value %s != %s in smkv",
                        k.Repr().CStr(), v->Repr().CStr(), iter->second.Repr().CStr()));
                }
            }
        });
        if (err)
        {
            lom::Die(lom::Sprintf("get by k from snapshot failed: %s", err->Msg().CStr()));
        }
    }

    {
        auto iter = smkv.begin();
        auto db_iter = snapshot->NewIterator();
        while (db_iter->Valid())
        {
            if (iter == smkv.end())
            {
                lom::Die(lom::Sprintf(
                    "k %s itered from snapshot after smkv iter end",
                    db_iter->Key().Repr().CStr()));
            }
            if (!(db_iter->Key() == iter->first.Slice() && db_iter->Value() == iter->second.Slice()))
            {
                lom::Die(lom::Sprintf(
                    "k %s v %s itered from snapshot, but not equal to smkv %s %s",
                    db_iter->Key().Repr().CStr(), db_iter->Value().Repr().CStr(),
                    iter->first.Slice().Repr().CStr(), iter->second.Slice().Repr().CStr()));
            }
            ++ iter;
            db_iter->Next();
        }
        if (db_iter->Err())
        {
            lom::Die(lom::Sprintf(
                "snapshot iter ends with error %s", db_iter->Err()->Msg().CStr()));
        }
        if (iter != smkv.end())
        {
            lom::Die(lom::Sprintf("snapshot iter ends before smkv"));
        }
    }

    std::cout << "OK" << std::endl;
}
