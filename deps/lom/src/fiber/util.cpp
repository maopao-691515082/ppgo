#include "internal.h"

namespace lom
{

namespace fiber
{

LOM_ERR PathToUnixSockAddr(const Str &path_s, struct sockaddr_un &addr, socklen_t &addr_len)
{
    const char *path;
    LOM_RET_ON_ERR(path_s.AsCStr(path));
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    size_t path_len = strlen(path);
    if (path_len == 0 || path_len >= sizeof(addr.sun_path))
    {
        LOM_RET_ERR("unix socket path is empty or too long: `%s`", path);
    }
    strcpy(addr.sun_path, path);
    addr_len = sizeof(addr) - sizeof(addr.sun_path) + path_len + 1;
    return nullptr;
}

LOM_ERR AbstractPathToUnixSockAddr(const Str &path, struct sockaddr_un &addr, socklen_t &addr_len)
{
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (path.Len() >= static_cast<ssize_t>(sizeof(addr.sun_path)))
    {
        LOM_RET_ERR("unix socket abstract path too long: %s", path.Repr().CStr());
    }
    memcpy(addr.sun_path + 1, path.Data(), static_cast<size_t>(path.Len()));
    addr_len = sizeof(addr) - sizeof(addr.sun_path) + 1 + static_cast<size_t>(path.Len());
    return nullptr;
}

}

}
