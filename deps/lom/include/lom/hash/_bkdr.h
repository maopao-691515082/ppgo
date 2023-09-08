#pragma once

#include "../_internal.h"

#include "../limit.h"
#include "../str.h"

namespace lom
{

namespace hash
{

class BKDRHash
{
    size_t factor_ = 889425222951875843ULL;
    size_t h_ = 0;

public:

    BKDRHash() = default;
    BKDRHash(ssize_t init_h) : h_(static_cast<size_t>(init_h) * factor_)
    {
    }
    BKDRHash(ssize_t init_h, ssize_t init_factor) :
        factor_(static_cast<size_t>(init_factor)), h_(static_cast<size_t>(init_h) * factor_)
    {
    }

    ssize_t Hash() const
    {
        return static_cast<ssize_t>(h_) & kSSizeMax;
    }

    void Update(const uint8_t *data, ssize_t len)
    {
        for (; len > 0; ++ data, -- len)
        {
            h_ = (h_ + *data) * factor_;
        }
    }
    void Update(const char *data, ssize_t len)
    {
        Update(reinterpret_cast<const uint8_t *>(data), len);
    }
    void Update(StrSlice s)
    {
        Update(s.Data(), s.Len());
    }

    //采用默认算法的快捷接口
    static ssize_t Hash(const uint8_t *data, ssize_t len)
    {
        BKDRHash h(len);
        h.Update(data, len);
        return h.Hash();
    }
    static ssize_t Hash(const char *data, ssize_t len)
    {
        return Hash(reinterpret_cast<const uint8_t *>(data), len);
    }
    static ssize_t Hash(StrSlice s)
    {
        return Hash(s.Data(), s.Len());
    }
};

}

}
