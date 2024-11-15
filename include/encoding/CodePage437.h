/*************************************************************************/
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
/*************************************************************************/

#ifndef CODEPAGE437_6502_H__
#define CODEPAGE437_6502_H__

/*************************************************************************/

namespace encodings
{
    class CodePage437 : public Encoding
    {
    private:
        static CodePage437 *s_self;

        // Map from 8 bit value to UTF-8 value.
        static uint8_t s_map[256][2];

    public:
        /* constructor */ CodePage437()
        {
            ASSERT(s_self == nullptr);
            s_self = this;
        }

        virtual ~CodePage437() { }

        static CodePage437 &Get()
        {
            ASSERT(s_self);
            return *s_self;
        }

        static CodePage437 *Ptr() { return s_self; }

        virtual wxString name() const { return _("Code Page 437"); }

        virtual wxString toUnicode(uint8_t c);
    };
}

/*************************************************************************/

#endif /* CODEPAGE437_6502_H__ */

/*************************************************************************/