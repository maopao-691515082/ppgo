#include "internal.h"

namespace lom
{

namespace fiber
{

::lom::Err::Ptr PathToUnixSockAddr(const char *path, struct sockaddr_un &addr, socklen_t &addr_len)
{
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    size_t path_len = strlen(path);
    if (path_len == 0 || path_len >= sizeof(addr.sun_path))
    {
        return ::lom::Err::Sprintf("unix socket path is empty or too long [%s]", path);
    }
    strcpy(addr.sun_path, path);
    addr_len = sizeof(addr) - sizeof(addr.sun_path) + path_len + 1;
    return nullptr;
}

::lom::Err::Ptr AbstractPathToUnixSockAddr(const Str &path, struct sockaddr_un &addr, socklen_t &addr_len)
{
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (path.Len() >= static_cast<ssize_t>(sizeof(addr.sun_path)))
    {
        return ::lom::Err::Sprintf("unix socket abstract path too long");
    }
    memcpy(addr.sun_path + 1, path.Data(), static_cast<size_t>(path.Len()));
    addr_len = sizeof(addr) - sizeof(addr.sun_path) + 1 + static_cast<size_t>(path.Len());
    return nullptr;
}

}

}
