#include "../internal.h"

namespace lom
{

namespace io
{

static ssize_t AdjustBufSize(ssize_t sz)
{
    return std::min<ssize_t>(std::max<ssize_t>(sz, 4 * 1024), 4 * 1024 * 1024);
}

#define LOM_IO_CHECK_NON_POSITIVE_SIZE_PARAM(_sz) do {      \
        ssize_t _szv = (_sz);                               \
        if (_szv <= 0) {                                    \
            LOM_RET_ERR("non-positive size `%zd`", _szv);   \
        }                                                   \
    } while (false)

class BufReaderImpl : public BufReader
{
    BufReader::DoReadFunc do_read_;
    ssize_t buf_sz_;
    MakeBufFunc make_buf_;
    char *buf_;
    ssize_t start_ = 0;
    ssize_t len_ = 0;

    LOM_ERR err_;

    LOM_ERR Fill()
    {
        if (err_)
        {
            return err_;
        }

        if (len_ == 0)
        {
            start_ = 0;
            ssize_t rsz;
            auto err = do_read_(buf_, buf_sz_, rsz);
            if (err)
            {
                err_ = err;
                return err;
            }
            if (rsz > 0)
            {
                Assert(rsz <= buf_sz_);
                len_ = rsz;
            }
        }

        return nullptr;
    }

    LOM_ERR ReadFullOrUntil(char *buf, ssize_t sz, const char *end_ch, ssize_t &rsz)
    {
        LOM_IO_CHECK_NON_POSITIVE_SIZE_PARAM(sz);

        for (ssize_t i = 0; i < sz;)
        {
            LOM_RET_ON_ERR(Fill());
            if (len_ == 0)
            {
                //EOF
                rsz = i;
                return UnexpectedEOF();
            }

            auto fill_len = std::min(len_, sz - i);
            bool is_end_ch_found = false;
            if (end_ch != nullptr)
            {
                for (ssize_t j = 0; j < fill_len; ++ j)
                {
                    if (buf_[start_ + j] == *end_ch)
                    {
                        fill_len = j + 1;
                        is_end_ch_found = true;
                        break;
                    }
                }
            }
            memcpy(buf + i, buf_ + start_, fill_len);
            i += fill_len;
            start_ += fill_len;
            len_ -= fill_len;
            if (is_end_ch_found)
            {
                rsz = i;
                return nullptr;
            }
        }

        rsz = sz;
        return nullptr;
    }

public:

    BufReaderImpl(BufReader::DoReadFunc do_read, ssize_t buf_sz, MakeBufFunc make_buf) :
        do_read_(do_read), buf_sz_(AdjustBufSize(buf_sz)), make_buf_(make_buf)
    {
        buf_ = make_buf_ ? make_buf_(buf_sz_) : new char[buf_sz_];
    }

    virtual ~BufReaderImpl()
    {
        if (!make_buf_)
        {
            delete[] buf_;
        }
    }

    virtual LOM_ERR Wait(ssize_t &rsz)
    {
        LOM_RET_ON_ERR(Fill());
        rsz = len_;
        return nullptr;
    }

    virtual LOM_ERR Read(char *buf, ssize_t sz, ssize_t &rsz) override
    {
        LOM_IO_CHECK_NON_POSITIVE_SIZE_PARAM(sz);

        LOM_RET_ON_ERR(Fill());
        if (len_ == 0)
        {
            rsz = 0;
            return nullptr;
        }
        auto copy_len = std::min(len_, sz);
        memcpy(buf, buf_ + start_, copy_len);
        start_ += copy_len;
        len_ -= copy_len;

        rsz = copy_len;
        return nullptr;
    }

    virtual LOM_ERR ReadUntil(char end_ch, char *buf, ssize_t sz, ssize_t &rsz) override
    {
        return ReadFullOrUntil(buf, sz, &end_ch, rsz);
    }

    virtual LOM_ERR ReadFull(char *buf, ssize_t sz, ssize_t *rsz_ptr) override
    {
        ssize_t rsz;
        return ReadFullOrUntil(buf, sz, nullptr, rsz_ptr ? *rsz_ptr : rsz);
    }
};

BufReader::Ptr BufReader::New(BufReader::DoReadFunc do_read, ssize_t buf_sz, MakeBufFunc make_buf)
{
    return BufReader::Ptr(new BufReaderImpl(do_read, buf_sz, make_buf));
}

class BufWriterImpl : public BufWriter
{
    BufWriter::DoWriteFunc do_write_;
    ssize_t buf_sz_;
    MakeBufFunc make_buf_;
    char *buf_;
    ssize_t start_ = 0;
    ssize_t len_ = 0;

    LOM_ERR DoWrite()
    {
        Assert(start_ < buf_sz_ && len_ <= buf_sz_);
        auto send_len = std::min(len_, buf_sz_ - start_);
        Assert(send_len > 0);
        ssize_t wsz;
        LOM_RET_ON_ERR(do_write_(buf_ + start_, send_len, wsz));

        Assert(wsz > 0 && wsz <= send_len);
        start_ += wsz;
        len_ -= wsz;
        if (start_ == buf_sz_ || len_ == 0)
        {
            //回绕的情况下写完了半段数据，或者没有回绕时所有数据写完，复位`start_`
            start_ = 0;
        }

        return nullptr;
    }

public:

    BufWriterImpl(BufWriter::DoWriteFunc do_write, ssize_t buf_sz, MakeBufFunc make_buf) :
        do_write_(do_write), buf_sz_(AdjustBufSize(buf_sz)), make_buf_(make_buf)
    {
        buf_ = make_buf_ ? make_buf_(buf_sz_) : new char[buf_sz_];
    }

    virtual ~BufWriterImpl()
    {
        if (!make_buf_)
        {
            delete[] buf_;
        }
    }

    virtual LOM_ERR WriteAll(const char *buf, ssize_t sz) override
    {
        if (sz < 0)
        {
            LOM_RET_ERR("negative size `%zd`", sz);
        }

        while (sz > 0)
        {
            Assert(len_ <= buf_sz_);
            if (len_ == buf_sz_)
            {
                LOM_RET_ON_ERR(DoWrite());
                Assert(len_ < buf_sz_);
            }
            auto tail = start_ + len_;
            ssize_t copy_len;
            if (tail < buf_sz_)
            {
                //缓冲数据没有回绕，后半段有空间
                copy_len = std::min(sz, buf_sz_ - tail);
            }
            else
            {
                //缓冲数据回绕了
                tail -= buf_sz_;
                Assert(tail < start_);
                copy_len = std::min(sz, start_ - tail);
            }
            memcpy(buf_ + tail, buf, copy_len);
            len_ += copy_len;
            buf += copy_len;
            sz -= copy_len;
        }

        return nullptr;
    }

    virtual LOM_ERR Flush() override
    {
        while (len_ > 0)
        {
            LOM_RET_ON_ERR(DoWrite());
        }
        return nullptr;
    }
};

BufWriter::Ptr BufWriter::New(BufWriter::DoWriteFunc do_write, ssize_t buf_sz, MakeBufFunc make_buf)
{
    return BufWriter::Ptr(new BufWriterImpl(do_write, buf_sz, make_buf));
}

}

}
