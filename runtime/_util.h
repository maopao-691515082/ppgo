#pragma once

namespace ppgo
{

namespace util
{

template <typename T>
std::shared_ptr<T> CopyPtr(const std::shared_ptr<T> &ptr)
{
    return ptr;
}

template <typename T>
T *CopyPtr(T *ptr)
{
    return ptr;
}

}

}
