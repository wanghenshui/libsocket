/**
 * Created by Jian Chen
 * @since  2016.05.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "chen/base/utf8.hpp"
#include <stdexcept>

// -----------------------------------------------------------------------------
// utf8
std::size_t chen::utf8::count(const std::string &text)
{
    auto ptr = text.data();
    auto len = text.size();

    std::size_t num  = 0;
    std::size_t pos  = utf8::bom(text) ? 3 : 0;
    std::size_t step = 0;

    while (pos < len)
    {
        if ((ptr[pos] & 0x80) == 0x00)       // 1Bytes
            step = 1;
        else if ((ptr[pos] & 0xe0) == 0xc0)  // 2Bytes
            step = 2;
        else if ((ptr[pos] & 0xf0) == 0xe0)  // 3Bytes
            step = 3;
        else if ((ptr[pos] & 0xf8) == 0xf0)  // 4Bytes
            step = 4;
        else if ((ptr[pos] & 0xfc) == 0xf8)  // 5Bytes
            step = 5;
        else if ((ptr[pos] & 0xfe) == 0xfc)  // 6Bytes
            step = 6;
        else
            break;

        ++num;
        pos += step;
    }

    return num;
}

std::string chen::utf8::substr(const std::string &text, std::size_t start, std::size_t count)
{
    if (!count)
        return "";

    auto ptr = text.data();
    auto len = text.size();

    std::string ret;

    std::size_t num  = 0;
    std::size_t pos  = utf8::bom(text) ? 3 : 0;
    std::size_t step = 0;

    while ((pos < len) && count)
    {
        if ((ptr[pos] & 0x80) == 0x00)       // 1Bytes
            step = 1;
        else if ((ptr[pos] & 0xe0) == 0xc0)  // 2Bytes
            step = 2;
        else if ((ptr[pos] & 0xf0) == 0xe0)  // 3Bytes
            step = 3;
        else if ((ptr[pos] & 0xf8) == 0xf0)  // 4Bytes
            step = 4;
        else if ((ptr[pos] & 0xfc) == 0xf8)  // 5Bytes
            step = 5;
        else if ((ptr[pos] & 0xfe) == 0xfc)  // 6Bytes
            step = 6;
        else
            return ret;

        if (num >= start)
        {
            ret.append(ptr + pos, step);
            count--;
        }

        ++num;
        pos += step;
    }

    return ret;
}

bool chen::utf8::bom(const std::string &text)
{
    return (text.size() >= 3) &&
           (static_cast<unsigned char>(text[0]) == 0xef) &&
           (static_cast<unsigned char>(text[1]) == 0xbb) &&
           (static_cast<unsigned char>(text[2]) == 0xbf);
}

std::string chen::utf8::convert(std::uint32_t ch)
{
    // I want to use C++11's codecvt to do the convert
    // but unfortunately the cocos2d-x didn't support it on Android now
    // I don't like to import any third-party library to do the simple convert
    // so I bought from http://stackoverflow.com/questions/4607413/c-library-to-convert-unicode-code-points-to-utf8
    // thanks to its author
    std::string ret;

    if (ch < 0x80)
    {
        ret += ch;
    }
    else if (ch < 0x800)
    {
        ret += 192 + ch / 64;
        ret += 128 + ch % 64;
    }
    else if (ch - 0xd800u < 0x800)
    {
        // error
        throw std::runtime_error("utf8: invalid unicode char");
    }
    else if (ch < 0x10000)
    {
        ret += 224 + ch / 4096;
        ret += 128 + ch / 64 % 64;
        ret += 128 + ch % 64;
    }
    else if (ch < 0x110000)
    {
        ret += 240 + ch / 262144;
        ret += 128 + ch / 4096 % 64;
        ret += 128 + ch / 64 % 64;
        ret += 128 + ch % 64;
    }
    else
    {
        // error
        throw std::runtime_error("utf8: invalid unicode char");
    }

    return ret;
}