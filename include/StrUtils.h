/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*************************************************************************/

#ifndef STR_UTILS_H__
#define STR_UTILS_H__

/*************************************************************************/

#include <algorithm>
#include <string>
#include <sstream>
#include <cctype>
#include <stdexcept>

/*************************************************************************/
/**
 * @brief Namespace containing character functions.
 */
namespace chr
{
    // C defines tolower() and toupper() as int types, C++ doesn't like this.
    inline char toLower(char c) { return std::tolower(c); }
    inline char toUpper(char c) { return std::toupper(c); }

    inline bool isAlpha(char c) { return std::isalpha(c); }
    inline bool isDigit(char c) { return std::isdigit(c); }
    inline bool isAlNum(char c) { return std::isalnum(c); }
    inline bool isSpace(char c) { return std::isspace(c); }

    inline bool notSpace(char c) { return !std::isspace(c); }

    inline constexpr bool isHex(char c) { return ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')); }

    inline constexpr uint8_t hexVal(char c)
    {
        return (
            ((c >= '0') && (c <= '9')) ?
            (c - '0') :
            (
                ((c >= 'A') && (c <= 'F')) ?
                (c - 'A' + 10) :
                ((c >= 'a') && (c <= 'f')) ?
                (c - 'a' + 10) :
                0xFF
            )
        );
    }

    // Poor man's unit testing
    static_assert(hexVal('0') == 0);
    static_assert(hexVal('9') == 9);
    static_assert(hexVal('A') == 10);
    static_assert(hexVal('F') == 15);
    static_assert(hexVal('a') == 10);
    static_assert(hexVal('f') == 15);
}

/*************************************************************************/

namespace str
{
    /**
     * @brief Convert string of 2 hex character into single byte.
     */
    inline uint8_t tryByteFromHex(const std::string &s, _Out_ uint8_t &out)
    {
        out = 0;

        if (s.length() < 2)
            return false;

        try
        {
            out = chr::hexVal(s[0]);
            out <<= 4;
            out |= chr::hexVal(s[1]);
        }
        catch (...)
        {
            // Invalid hex character
            return false;
        }

        return true;
    }

    /**
     * @brief Base class for string manipulation operators.
     */
    struct basic_operator
    {
        virtual std::string operator()(const std::string &s) const = 0;
    };

    /**
     * @brief Trims space on the left side of a string.
     */
    struct ltrim_t : public basic_operator
    {
        std::string operator ()(const std::string &s) const override
        {
            auto it = std::find_if(s.begin(), s.end(), chr::notSpace);
            std::size_t dist = it - s.begin();
            return s.substr(dist);
        }
    };

    /**
     * @brief Trims space on the right side of a string.
     */
    struct rtrim_t : public basic_operator
    {
        std::string operator ()(const std::string &s) const override
        {
            auto it = std::find_if(s.rbegin(), s.rend(), chr::notSpace);
            std::size_t dist = it - s.rbegin();
            return s.substr(0, s.length() - dist);
        }
    };

    // Trims space on the left side of the string.
    const ltrim_t ltrim;

    // Trims space on the right side of the string.
    const rtrim_t rtrim;

    /**
     * @brief Trims both sides of a string.
     */
    struct trim_t : public basic_operator
    {
        std::string operator ()(const std::string &s) const override
        {
            return ltrim(rtrim(s));
        }
    };

    // Trims both sides of a string
    const trim_t trim;

    /**
     * @brief Runs a conversion function over a string and returns the result.
     */
    struct transform_t : public basic_operator
    {
    private:
        std::function<char(char)> fn;

    public:
        transform_t(std::function<char(char)> f) : fn(f) { }

        std::string operator ()(const std::string &s) const override
        {
            std::string rval;
            rval.reserve(s.size());
            std::transform(s.begin(), s.end(), std::back_inserter(rval), fn);
            return rval;
        }
    };

    // Convert a string to all lower case
    const transform_t toLower(chr::toLower);

    // Convert a string to all upper case
    const transform_t toUpper(chr::toUpper);

    /**
     * @brief Convert a container of items into a single string separated by separator.
     */
    template <std::ranges::forward_range TRange>
    inline std::string join(const std::string &separator, TRange &&container)
    {
        std::stringstream rval;
        bool first = true;

        for (auto i : container)
        {
            if (first)
                first = false;
            else
                rval << separator;

            rval << i;
        }

        return rval.str();
    }

    /**
     * Constraint that requires class to define a toString() method that 
     * returns a string of the object.
     */
    template <typename T>
    concept str_convertable = requires(T a)
    {
        { a.toString() } -> std::convertible_to<std::string>;
    };

    const std::string empty;
}

namespace file
{
    /**
     * @brief Extract the directory information from a path.
     */
    std::string getDirectory(const std::string &path);

    /**
     * @brief Extract the filename from a path.
     */
    std::string getFilename(const std::string &path);

    /**
     * @brief Extract the extension of a file or path.
     */
    std::string getExtension(const std::string &path);
}

/**
 * @brief Serialize object to stream
 * 
 * Operator that will convert and object to a string and send it to the stream.
 * 
 * This function requires that the object's class define a toString() method.
 */
template <str::str_convertable T>
std::ostream &operator <<(std::ostream &stream, const T &item)
{
    stream << item.toString();
    return stream;
}

inline
std::string operator |(const std::string &s, const str::basic_operator &f)
{ return f(s); }

/*************************************************************************/

#endif /* STR_UTILS_H__ */

/*************************************************************************/
