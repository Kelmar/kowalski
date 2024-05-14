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

namespace str
{
    namespace 
    {
        // C defines tolower() and toupper() as int types, C++ doesn't like this.
        inline char c_tolower(char c) { return std::tolower(c); }
        inline char c_toupper(char c) { return std::toupper(c); }

        inline bool c_isspace(char c) { return std::isspace(c); }

        inline bool c_notSpace(char c) { return !std::isspace(c); }

        inline constexpr uint8_t hex_val(char c) {
            return (
                ((c >= '0') && (c <= '9')) ?
                    (c - '0') : 
                    (
                        ((c >= 'A') && (c <= 'F')) ? 
                            (c - 'A' + 10) :
                            ((c >= 'a') && (c <= 'f')) ?
                                (c - 'a' + 10) :
                                throw std::runtime_error("Invalid hex character")
                        )
                    );
        }

        // Poor man's unit testing
        static_assert(hex_val('0') == 0);
        static_assert(hex_val('9') == 9);
        static_assert(hex_val('A') == 10);
        static_assert(hex_val('F') == 15);
        static_assert(hex_val('a') == 10);
        static_assert(hex_val('f') == 15);
    }

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
            out = hex_val(s[0]);
            out <<= 4;
            out |= hex_val(s[1]);
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
            auto it = std::find_if(s.begin(), s.end(), c_notSpace);
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
            auto it = std::find_if(s.rbegin(), s.rend(), c_notSpace);
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
    struct transform : public basic_operator
    {
    private:
        std::function<char(char)> fn;

    public:
        transform(std::function<char(char)> f) : fn(f) { }

        std::string operator ()(const std::string &s) const override
        {
            std::string rval;
            rval.reserve(s.size());
            std::transform(s.begin(), s.end(), std::back_inserter(rval), fn);
            return rval;
        }
    };

    // Convert a string to all lower case
    const transform tolower(c_tolower);

    // Convert a string to all upper case
    const transform toupper(c_toupper);

    /**
     * @brief Convert a container of items into a single string separated by separator.
     */
    inline std::string join(const std::string &separator, auto &&container)
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
     * Constraint that requires class to define a ToString() method that 
     * returns a string of the object.
     */
    template <typename T>
    concept str_convertable = requires(T a)
    {
        { a.ToString() } -> std::convertible_to<std::string>;
    };
}

/**
 * @brief Serialize object to stream
 * 
 * Operator that will convert and object to a string and send it to the stream.
 * 
 * This function requires that the object's class define a ToString() method.
 */
template <str::str_convertable T>
std::ostream &operator <<(std::ostream &stream, const T &item)
{
    stream << item.ToString();
    return stream;
}

inline
std::string operator |(const std::string &s, const str::basic_operator &f)
{ return f(s); }

/*************************************************************************/

#endif /* STR_UTILS_H__ */

/*************************************************************************/
