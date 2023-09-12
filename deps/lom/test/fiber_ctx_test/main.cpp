#include <iostream>

#include "../../include/lom.h"

static void WorkerMain()
{
    ::lom::fiber::Sem sem;
    auto err = ::lom::fiber::Sem::New(0, sem);
    if (err)
    {
        LOM_DIE("Sem.New: %s", err->Msg().CStr());
    }
    err = sem.Acquire();
    if (err)
    {
        std::cerr << "sem.Acquire failed: " << err->Msg().CStr() << std::endl;
    }
}

int main(int, char *[])
{
    ::lom::fiber::MustInit();

    ::lom::fiber::Create([] () {
        auto err = ::lom::fiber::Ctx(1000).Call(
            [&] () -> ::lom::Err::Ptr {
                ::lom::fiber::Ctx().Call(
                    [&] () -> ::lom::Err::Ptr {
                        ::lom::fiber::CreateWorker(WorkerMain);
                        return nullptr;
                    }
                );
                return ::lom::fiber::Ctx(100000).Call(
                    [&] () -> ::lom::Err::Ptr {
                        for (int i = 0; i < 3; ++ i)
                        {
                            ::lom::fiber::CreateWorker(WorkerMain);
                        }
                        ::lom::fiber::Conn conn;
                        return ::lom::fiber::ConnectTCP("1.1.1.1", 23, conn);
                    }
                );
            }
        );
        if (err)
        {
            std::cerr << "ConnectTCP failed: " << err->Msg().CStr() << std::endl;
        }
    });

    auto err = ::lom::fiber::Run();
    LOM_DIE("fiber.Run exited: %s", err->Msg().CStr());
}
