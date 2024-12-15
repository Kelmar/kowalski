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

#include "StdAfx.h"
#include "encodings.h"

#if 0

/*************************************************************************/

using namespace encodings;

/*************************************************************************/

CodePage437 *CodePage437::s_self;

uint8_t CodePage437::s_map[256][2] =
{
    { 0x2E, 0x00 }, // NUL

    // Map control characters to their UTF-8 printable variants
    { 0x3A, 0x26 }, // 1
    { 0x3B, 0x26 },
    { 0x65, 0x26 },
    { 0x66, 0x26 },
    { 0x63, 0x26 }, // 5
    { 0x60, 0x26 },
    { 0x22, 0x20 },
    { 0xD8, 0x25 },
    { 0xCB, 0x25 },
    { 0xD9, 0x25 }, // 10 (0A)
    { 0x42, 0x26 },
    { 0x40, 0x26 },
    { 0x6A, 0x26 },
    { 0x6B, 0x26 },
    { 0x3C, 0x26 }, // 15 (0F)
    { 0xBA, 0x25 },
    { 0xC4, 0x25 },
    { 0x95, 0x21 },
    { 0x3C, 0x20 },
    { 0xB6, 0x00 }, // 20 (14)
    { 0xA7, 0x00 },
    { 0xAC, 0x25 },
    { 0xA8, 0x21 },
    { 0x91, 0x21 },
    { 0x93, 0x21 }, // 25 (19)
    { 0x92, 0x21 },
    { 0x90, 0x21 },
    { 0x1F, 0x22 },
    { 0x94, 0x21 },
    { 0xB2, 0x25 }, // 30 (1E)
    { 0xBC, 0x25 }, // 31 (1F)

    // Normal characters map as per usual up to 127
    { 0x20, 0 }, { 0x21, 0 }, { 0x22, 0 }, { 0x23, 0 }, { 0x24, 0 }, { 0x25, 0 }, { 0x26, 0 }, { 0x27, 0 },
    { 0x28, 0 }, { 0x29, 0 }, { 0x2A, 0 }, { 0x2B, 0 }, { 0x2C, 0 }, { 0x2D, 0 }, { 0x2E, 0 }, { 0x2F, 0 },

    { 0x30, 0 }, { 0x31, 0 }, { 0x32, 0 }, { 0x33, 0 }, { 0x34, 0 }, { 0x35, 0 }, { 0x36, 0 }, { 0x37, 0 },
    { 0x38, 0 }, { 0x39, 0 }, { 0x3A, 0 }, { 0x3B, 0 }, { 0x3C, 0 }, { 0x3D, 0 }, { 0x3E, 0 }, { 0x3F, 0 },

    { 0x40, 0 }, { 0x41, 0 }, { 0x42, 0 }, { 0x43, 0 }, { 0x44, 0 }, { 0x45, 0 }, { 0x46, 0 }, { 0x47, 0 },
    { 0x48, 0 }, { 0x49, 0 }, { 0x4A, 0 }, { 0x4B, 0 }, { 0x4C, 0 }, { 0x4D, 0 }, { 0x4E, 0 }, { 0x4F, 0 },

    { 0x50, 0 }, { 0x51, 0 }, { 0x52, 0 }, { 0x53, 0 }, { 0x54, 0 }, { 0x55, 0 }, { 0x56, 0 }, { 0x57, 0 },
    { 0x58, 0 }, { 0x59, 0 }, { 0x5A, 0 }, { 0x5B, 0 }, { 0x5C, 0 }, { 0x5D, 0 }, { 0x5E, 0 }, { 0x5F, 0 },

    { 0x60, 0 }, { 0x61, 0 }, { 0x62, 0 }, { 0x63, 0 }, { 0x64, 0 }, { 0x65, 0 }, { 0x66, 0 }, { 0x67, 0 },
    { 0x68, 0 }, { 0x69, 0 }, { 0x6A, 0 }, { 0x6B, 0 }, { 0x6C, 0 }, { 0x6D, 0 }, { 0x6E, 0 }, { 0x6F, 0 },

    { 0x70, 0 }, { 0x71, 0 }, { 0x72, 0 }, { 0x73, 0 }, { 0x74, 0 }, { 0x75, 0 }, { 0x76, 0 }, { 0x77, 0 },
    { 0x78, 0 }, { 0x79, 0 }, { 0x7A, 0 }, { 0x7B, 0 }, { 0x7C, 0 }, { 0x7D, 0 }, { 0x7E, 0 },

    { 0x02, 0x23 }, // 127 (7F)

    { 0xC7, 0x00 }, // 128 (80)
    { 0xFC, 0x00 },
    { 0xE9, 0x00 }, // 130 (82)
    { 0xE2, 0x00 },
    { 0xE4, 0x00 },
    { 0xE0, 0x00 },
    { 0xE5, 0x00 },
    { 0xE7, 0x00 }, // 135 (87)
    { 0xEA, 0x00 },
    { 0xEB, 0x00 },
    { 0xE8, 0x00 },
    { 0xEF, 0x00 },
    { 0xEE, 0x00 }, // 140 (8C)
    { 0xEC, 0x00 },
    { 0xC4, 0x00 },
    { 0xC5, 0x00 }, // 143 (8F)

    { 0xC9, 0x00 }, // 144 (90)
    { 0xE6, 0x00 }, // 145 (91)
    { 0xC6, 0x00 },
    { 0xF4, 0x00 },
    { 0xF6, 0x00 },
    { 0xF2, 0x00 },
    { 0xFB, 0x00 }, // 150 (96)
    { 0xF9, 0x00 },
    { 0xFF, 0x00 },
    { 0xD6, 0x00 },
    { 0xDC, 0x00 },
    { 0xA2, 0x00 }, // 155 (9B)
    { 0xA3, 0x00 },
    { 0xA5, 0x00 },
    { 0xA7, 0x20 },
    { 0x92, 0x01 }, // 159 (9F)

    { 0xE1, 0x00 }, // 160 (A0)
    { 0xED, 0x00 },
    { 0xF3, 0x00 },
    { 0xFA, 0x00 },
    { 0xF1, 0x00 },
    { 0xD1, 0x00 }, // 165 (A5)
    { 0xAA, 0x00 },
    { 0xBA, 0x00 },
    { 0xBF, 0x00 },
    { 0x10, 0x23 },
    { 0xAC, 0x00 }, // 170 (AA)
    { 0xBD, 0x00 },
    { 0xBC, 0x00 },
    { 0xA1, 0x00 },
    { 0xAB, 0x00 },
    { 0xBB, 0x00 }, // 175 (AF)

    { 0x91, 0x25 }, // 176 (B0)
    { 0x92, 0x25 },
    { 0x93, 0x25 },
    { 0x02, 0x25 },
    { 0x24, 0x25 }, // 180 (B5)
    { 0x61, 0x25 },
    { 0x62, 0x25 },
    { 0x56, 0x25 },
    { 0x55, 0x25 },
    { 0x63, 0x25 }, // 185 (B9)
    { 0x21, 0x25 },
    { 0x57, 0x25 },
    { 0x5D, 0x25 },
    { 0x5C, 0x25 },
    { 0x5B, 0x25 }, // 190 (BE)
    { 0x10, 0x25 }, // 191 (BF)

    { 0x14, 0x25 }, // 192 (C0)
    { 0x34, 0x25 },
    { 0x2C, 0x25 },
    { 0x1C, 0x25 }, // 195 (C3)
    { 0x00, 0x25 },
    { 0x3C, 0x25 },
    { 0x5E, 0x25 },
    { 0x5F, 0x25 },
    { 0x5A, 0x25 }, // 200 (C8)
    { 0x54, 0x25 },
    { 0x69, 0x25 },
    { 0x66, 0x25 },
    { 0x60, 0x25 },
    { 0x50, 0x25 }, // 205 (CD)
    { 0x6C, 0x25 },
    { 0x67, 0x25 }, // 207 (CF)

    { 0x68, 0x25 }, // 208 (D0)
    { 0x64, 0x25 },
    { 0x65, 0x25 }, // 210 (D2)
    { 0x59, 0x25 },
    { 0x58, 0x25 },
    { 0x52, 0x25 },
    { 0x53, 0x25 },
    { 0x6B, 0x25 }, // 215 (D7)
    { 0x6A, 0x25 },
    { 0x18, 0x25 },
    { 0x0C, 0x25 },
    { 0x88, 0x25 },
    { 0x84, 0x25 }, // 220 (DC)
    { 0x8C, 0x25 },
    { 0x90, 0x25 },
    { 0x80, 0x25 }, // 223 (DF)

    { 0xB1, 0x03 }, // 224 (E0)
    { 0xDF, 0x00 }, // 225 (E1)
    { 0x93, 0x03 },
    { 0xC0, 0x03 },
    { 0xA3, 0x03 },
    { 0xC3, 0x03 },
    { 0xB5, 0x03 }, // 230 (E6)
    { 0xC4, 0x03 },
    { 0xA6, 0x03 },
    { 0x98, 0x03 },
    { 0xA9, 0x03 },
    { 0xB4, 0x03 }, // 235 (EB)
    { 0xE1, 0x22 },
    { 0xC6, 0x03 },
    { 0xB5, 0x03 },
    { 0x29, 0x22 }, // 239 (EF)

    { 0x61, 0x22 }, // 240 (F0)
    { 0xB1, 0x00 },
    { 0x65, 0x22 },
    { 0x64, 0x22 },
    { 0x20, 0x23 },
    { 0x21, 0x23 }, // 245 (F5)
    { 0xF7, 0x00 },
    { 0x48, 0x22 },
    { 0xB0, 0x00 },
    { 0x19, 0x22 },
    { 0xB7, 0x00 }, // 250 (FA)
    { 0x1A, 0x22 },
    { 0x7F, 0x20 },
    { 0xB2, 0x00 },
    { 0xA0, 0x25 },
    { 0x20,    0 }  // 255 (FF) (Non breaking space, maping to space)
};

/*************************************************************************/

wxString CodePage437::toUnicode(uint8_t c)
{
    uint8_t t1 = s_map[c][1];
    uint8_t t0 = s_map[c][0];

    (void)(t1);
    (void)(t0);

    int i = c & 0x000000FF; // Convert to unsigned w/o sign extend.
    wchar_t wc = s_map[i][1] << 8 | s_map[i][0];
    return wxString(wc);
}

/*************************************************************************/
#endif