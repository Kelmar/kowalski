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

namespace str
{
    namespace 
    {
        // C defines tolower() and toupper() as int types, C++ doesn't like this.
        inline char c_tolower(char c) { return std::tolower(c); }
        inline char c_toupper(char c) { return std::toupper(c); }
    }

    /**
     * @brief Runs a conversion function over a string and returns the result.
     */
    inline std::string transform(const std::string &s, std::function<char(char)> mutator)
    {
        std::string rval;
        rval.reserve(s.size());
        std::transform(s.begin(), s.end(), std::back_inserter(rval), mutator);
        return rval;
    }

    /**
     * @brief Converts a string in place
     */
    inline std::string &mutate(std::string &s, std::function<char(char)> mutator)
    {
        std::transform(s.begin(), s.end(), s.begin(), mutator);
        return s;
    }

    /**
     * @brief Converts a string to all lower case.
     */
    inline std::string getLower(const std::string &s)
    {
        return transform(s, c_tolower);
    }

    /**
     * @brief Converts a string to all upper case.
     */
    inline std::string getUpper(const std::string &s)
    {
        return transform(s, c_toupper);
    }

    /**
     * @brief Converts a string to all lower in place.
     */
    inline std::string &toLower(std::string &s)
    {
        return mutate(s, c_tolower);
    }

    /**
     * @brief Converts a string to all upper in place.
     */
    inline std::string &toUpper(std::string &s)
    {
        return mutate(s, c_toupper);
    }

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

/*************************************************************************/

#endif /* STR_UTILS_H__ */

/*************************************************************************/
