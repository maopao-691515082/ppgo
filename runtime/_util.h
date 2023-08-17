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

namespace optional_type_method
{

template <typename T>
bool valid(const std::optional<T> &opt)
{
    return opt.has_value();
}

template <typename T>
T get(const std::optional<T> &opt)
{
    return opt.value();
}

template <typename T>
T get_or(const std::optional<T> &opt, const T &default_value)
{
    return opt.value_or(default_value);
}

template <typename T>
void set(std::optional<T> &opt, const T &value)
{
    opt = value;
}

template <typename T>
void clear(std::optional<T> &opt)
{
    opt = std::nullopt;
}

}

}

}
