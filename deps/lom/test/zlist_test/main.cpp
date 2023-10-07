#include <iostream>

#include "../../include/lom.h"

int main(int, char *[])
{
    lom::immut::ZList zl0;
    auto zl1 = zl0.Append("abc");
    auto zl2 = zl1.Extend(lom::GoSlice<lom::StrSlice>{"def", "ghi"});
    auto zl3 = zl2.Extend(zl2);

    {
        auto iter = zl3.NewForwardIterator();
        for (;;)
        {
            lom::StrSlice s;
            if (!iter(s))
            {
                break;
            }
            std::cout << s.Repr().CStr() << " ";
        }
        std::cout << std::endl;
    }

    {
        lom::immut::ZList::Iterator iter(zl3);
        iter.Seek(iter.StrCount() - 1);
        while (iter.Valid())
        {
            std::cout << iter.Get().Repr().CStr() << " ";
            iter.Prev();
        }
        std::cout << std::endl;
    }

    {
        lom::Str::Buf b;
        auto err = zl3.DumpTo(lom::io::BufWriter::New(
            [&b] (const char *buf, ssize_t sz, ssize_t &wsz) -> LOM_ERR {
                b.Append(buf, sz);
                wsz = sz;
                return nullptr;
            }
        ));
        if (err)
        {
            LOM_DIE("DumpTo failed: %s", err->Msg().CStr());
        }
        std::cout << "dump len: " << b.Len() << std::endl;
        lom::immut::ZList zl;
        ssize_t idx = 0;
        err = lom::immut::ZList::LoadFrom(
            lom::io::BufReader::New(
                [&b, &idx] (char *buf, ssize_t sz, ssize_t &rsz) -> LOM_ERR {
                    auto p = b.Data() + idx;
                    rsz = std::min(sz, b.Len() - idx);
                    lom::Assert(rsz >= 0);
                    memcpy(buf, p, rsz);
                    idx += rsz;
                    return nullptr;
                }
            ),
            zl
        );
        if (err)
        {
            LOM_DIE("LoadFrom failed: %s", err->Msg().CStr());
        }
        auto iter = zl.NewForwardIterator();
        for (;;)
        {
            lom::StrSlice s;
            if (!iter(s))
            {
                break;
            }
            std::cout << s.Repr().CStr() << " ";
        }
        std::cout << std::endl;

        ssize_t b_len_shorter = b.Len() - 1;
        idx = 0;
        err = lom::immut::ZList::LoadFrom(
            lom::io::BufReader::New(
                [&b, b_len_shorter, &idx] (char *buf, ssize_t sz, ssize_t &rsz) -> LOM_ERR {
                    auto p = b.Data() + idx;
                    rsz = std::min(sz, b_len_shorter - idx);
                    lom::Assert(rsz >= 0);
                    memcpy(buf, p, rsz);
                    idx += rsz;
                    return nullptr;
                }
            ),
            zl
        );
        if (err)
        {
            if (err == lom::io::UnexpectedEOF())
            {
                std::cout << "UnexpectedEOF detected" << std::endl;
            }
            else
            {
                LOM_DIE("LoadFrom failed: %s", err->Msg().CStr());
            }
        }
    }
}
