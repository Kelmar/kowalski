/*=======================================================================*/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*=======================================================================*/

#ifndef OUTPUT_65_H__
#define OUTPUT_65_H__

/*=======================================================================*/

#include "StrUtils.h"

/*=======================================================================*/

namespace io
{
    /**
     * @brief Abstract class for writing to an output target.
     * @remarks This is usually a console.
     */
    class output
    {
    private:
        // Remove copy construction

        /* constructor */ output(const output &) = delete;
        /* constructor */ output(output &&) = delete;

    protected:
        /* constructor */ output() { }

    public:
        virtual ~output() { }

        /**
         * @brief Request to clear the output display.
         * @remarks This may not to anything for some implementations.
         */
        virtual void clear() { };

        /**
         * @brief Writes the supplied string to the output display.
         * @param str The string to write.
         * @remarks New lines are interpreted on \r\n pair.
         */
        virtual void write(std::string_view str) = 0;

        /**
         * @brief Write a formated string out to the display.
         * @tparam ...T 
         * @param fmt 
         * @param ...args 
         */
        template <typename... T>
        void writef(const std::string &fmt, T&&... args)
        {
            write(fmt::vformat(fmt, fmt::make_format_args(args...)));
        }
    };
}

inline io::output &operator <<(io::output &target, const std::string &item)
{
    target.write(item);
    return target;
}

template <str::str_convertable T>
inline io::output &operator <<(io::output &target, const T &item)
{
    target.write(item.toString());
    return target;
}

/*=======================================================================*/

#endif /* OUTPUT_65_H__ */

/*=======================================================================*/
