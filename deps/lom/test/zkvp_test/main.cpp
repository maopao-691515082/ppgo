#include <iostream>

#include "../../include/lom.h"

static lom::Str RandK()
{
    return lom::Sprintf("key:%zu", static_cast<size_t>(lom::RandN(10000000)));
}

static void HandleBgErr(LOM_ERR err)
{
    std::cerr << "BG ERR: " << err->Msg().CStr() << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        LOM_DIE("Usage: %s <DB_PATH> [<TEST_COUNT>]", argv[0]);
    }

    ssize_t test_count = 10000;
    if (argc >= 3)
    {
        if (!(lom::Str(argv[2]).ParseSSize(test_count) && test_count > 0))
        {
            LOM_DIE("invalid test count");
        }
    }

    lom::ordered_kv::zkv::DB::Ptr db;
    double ts = lom::NowFloat();
    auto err = lom::ordered_kv::zkv::DB::Open(
        argv[1], db,
        lom::ordered_kv::zkv::DB::Options{
            .create_if_missing  = true,
            .handle_bg_err      = HandleBgErr,
        }
    );
    if (err)
    {
        LOM_DIE("open db failed: %s", err->FmtWithTB().CStr());
    }
    std::cout << "open db ok, cost: " << (lom::NowFloat() - ts) << " sec" << std::endl;

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
            }
            else
            {
                wb.Set(k, v);
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
    std::cout << "write ok" << std::endl;

    {
        auto snapshot = db->NewSnapshot();
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
    std::cout << "check ok" << std::endl;

    db = nullptr;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    ts = lom::NowFloat();
    err = lom::ordered_kv::zkv::DB::Open(
        argv[1], db,
        lom::ordered_kv::zkv::DB::Options{
            .handle_bg_err  = HandleBgErr,
        }
    );
    if (err)
    {
        LOM_DIE("open db failed: %s", err->Msg().CStr());
    }
    std::cout << "reopen db ok, cost " << (lom::NowFloat() - ts) << " sec" << std::endl;

    {
        auto snapshot = db->NewSnapshot();
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
    std::cout << "check ok" << std::endl;
}
