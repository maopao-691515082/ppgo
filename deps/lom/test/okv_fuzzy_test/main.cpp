#include <iostream>

#include "../../include/lom.h"

static lom::Str RandK()
{
    return lom::Sprintf("key:%zu", static_cast<size_t>(lom::RandN(10000000)));
}

int main(int argc, char *argv[])
{
    ssize_t test_count = 10000;
    if (argc >= 3)
    {
        if (!(lom::Str(argv[2]).ParseSSize(test_count) && test_count > 0))
        {
            LOM_DIE("invalid test count");
        }
    }

    lom::ordered_kv::DBBase::Ptr db;
    LOM_ERR err;
    if (argc >= 2)
    {
        if (strcmp(argv[1], "zkvm") == 0)
        {
            lom::ordered_kv::zkv::DB::Ptr zkv_db;
            err = lom::ordered_kv::zkv::DB::Open(zkv_db);
            db = std::dynamic_pointer_cast<lom::ordered_kv::DBBase>(zkv_db);
        }
    }
    if (err)
    {
        LOM_DIE("open db failed: %s", err->Msg().CStr());
    }
    if (!db)
    {
        LOM_DIE("unknown db type");
    }

    std::map<lom::Str, lom::Str> smkv;
    {
        auto db_iter = db->NewSnapshot()->NewIterator();
        while (db_iter->Valid())
        {
            auto k = db_iter->Key(), v = db_iter->Value();
            if (smkv.count(k) > 0)
            {
                LOM_DIE("load db failed, dup key [%s]", k.Repr().CStr());
            }
            smkv[k] = v;
            db_iter->Next();
        }
        if (db_iter->Err())
        {
            LOM_DIE("load db failed: %s", db_iter->Err()->Msg().CStr());
        }
        std::cout << "load ok, kv count: " << smkv.size() << std::endl;
    }

    ssize_t kv_op_count = 0;
    double last_log_time = 0.0;
    ssize_t last_i = 0, last_kv_op_count = 0;
    auto snapshot_sync_with_db = db->NewSnapshot();  //和db保持一致的snapshot修改版
    for (ssize_t i = 0; i < test_count; ++ i)
    {
        double now = lom::NowFloat();
        double tm = now - last_log_time;
        if (tm >= 1.0)
        {
            std::cout << "completed " << i << " kvs write op, " << kv_op_count << " kv write op" << std::endl;
            std::cout << "speed: " <<
                (i - last_i) / tm << "\t" << (kv_op_count - last_kv_op_count) / tm << std::endl;
            last_log_time = now;
            last_i = i;
            last_kv_op_count = kv_op_count;
        }

        lom::ordered_kv::WriteBatch wb;
        auto n = (ssize_t)lom::RandN(32);
        while (wb.Size() < n)
        {
            auto
                k = RandK(),
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
            LOM_DIE("write db failed, error [%s]", err->Msg().CStr());
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
                LOM_DIE(
                    "k %s itered from snapshot_sync_with_db after smkv iter end",
                    db_iter->Key().Repr().CStr());
            }
            if (!(db_iter->Key() == iter->first.Slice() && db_iter->Value() == iter->second.Slice()))
            {
                LOM_DIE(
                    "k %s v %s itered from snapshot_sync_with_db, but not equal to smkv %s %s",
                    db_iter->Key().Repr().CStr(), db_iter->Value().Repr().CStr(),
                    iter->first.Repr().CStr(), iter->second.Repr().CStr());
            }
            ++ iter;
            db_iter->Next();
        }
        if (db_iter->Err())
        {
            LOM_DIE("snapshot_sync_with_db iter ends with error %s", db_iter->Err()->Msg().CStr());
        }
        if (iter != smkv.end())
        {
            LOM_DIE("snapshot_sync_with_db iter ends before smkv");
        }
    }

    last_log_time = 0.0;
    last_i = 0;
    auto snapshot = db->NewSnapshot();
    for (ssize_t i = 0; i < test_count * 20; ++ i)
    {
        double now = lom::NowFloat();
        double tm = now - last_log_time;
        if (tm >= 1.0)
        {
            std::cout << "completed " << i << " by-k read op" << std::endl;
            std::cout << "speed: " << (i - last_i) / tm << " by-k read op/s" << std::endl;
            last_log_time = now;
            last_i = i;
        }

        auto k = RandK();

        err = snapshot->Get(k, [&] (const lom::StrSlice *v) {
            auto iter = smkv.find(k);
            if (v == nullptr)
            {
                if (iter != smkv.end())
                {
                    LOM_DIE("key %s not in db but in smkv", k.Repr().CStr());
                }
            }
            else
            {
                if (iter == smkv.end())
                {
                    LOM_DIE("key %s in db but not in smkv", k.Repr().CStr());
                }
                if (iter->second.Slice() != *v)
                {
                    LOM_DIE(
                        "key %s value %s != %s in smkv",
                        k.Repr().CStr(), v->Repr().CStr(), iter->second.Repr().CStr());
                }
            }
        });
        if (err)
        {
            LOM_DIE("get by k from snapshot failed: %s", err->Msg().CStr());
        }
    }

    last_log_time = 0.0;
    last_i = 0;
    for (ssize_t i = 0; i < test_count * 10; ++ i)
    {
        double now = lom::NowFloat();
        double tm = now - last_log_time;
        if (tm >= 1.0)
        {
            std::cout << "completed " << i << " by-k read-del op" << std::endl;
            std::cout << "speed: " << (i - last_i) / tm << " by-k read-del op/s" << std::endl;
            last_log_time = now;
            last_i = i;
        }

        auto k = RandK();

        //边读snapshot边删除，测试snapshot的快照机制
        lom::ordered_kv::WriteBatch wb;
        wb.Del(k);
        err = db->Write(wb);
        if (err)
        {
            LOM_DIE("write db failed, error [%s]", err->Msg().CStr());
        }

        err = snapshot->Get(k, [&] (const lom::StrSlice *v) {
            auto iter = smkv.find(k);
            if (v == nullptr)
            {
                if (iter != smkv.end())
                {
                    LOM_DIE("key %s not in db but in smkv", k.Repr().CStr());
                }
            }
            else
            {
                if (iter == smkv.end())
                {
                    LOM_DIE("key %s in db but not in smkv", k.Repr().CStr());
                }
                if (iter->second.Slice() != *v)
                {
                    LOM_DIE(
                        "key %s value %s != %s in smkv",
                        k.Repr().CStr(), v->Repr().CStr(), iter->second.Repr().CStr());
                }
            }
        });
        if (err)
        {
            LOM_DIE("get by k from snapshot failed: %s", err->Msg().CStr());
        }
    }

    {
        auto iter = smkv.begin();
        auto db_iter = snapshot->NewIterator();
        while (db_iter->Valid())
        {
            if (iter == smkv.end())
            {
                LOM_DIE(
                    "k %s itered from snapshot after smkv iter end",
                    db_iter->Key().Repr().CStr());
            }
            if (!(db_iter->Key() == iter->first.Slice() && db_iter->Value() == iter->second.Slice()))
            {
                LOM_DIE(
                    "k %s v %s itered from snapshot, but not equal to smkv %s %s",
                    db_iter->Key().Repr().CStr(), db_iter->Value().Repr().CStr(),
                    iter->first.Repr().CStr(), iter->second.Repr().CStr());
            }
            ++ iter;
            db_iter->Next();
        }
        if (db_iter->Err())
        {
            LOM_DIE("snapshot iter ends with error %s", db_iter->Err()->Msg().CStr());
        }
        if (iter != smkv.end())
        {
            LOM_DIE("snapshot iter ends before smkv");
        }
    }

    std::cout << "OK" << std::endl;
}
