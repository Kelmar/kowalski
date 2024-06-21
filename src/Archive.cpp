/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the �Software�),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*************************************************************************/

#include "StdAfx.h"

#include <iostream>
#include <fstream>

#include "Archive.h"

/*************************************************************************/

Archive::Archive(
    const std::string &filename,
    Archive::Mode mode,
    Archive::Endianess endianess /* = Archive::Endianess::Platform */,
    CheckFn checkFn /* = noChecksum */)
    : m_filename(filename)
    , m_file(nullptr)
    , m_mode(mode)
    , m_endianess(endianess)
    , m_checkFn(checkFn ? checkFn : noCheck)
    , m_swapFn(noSwap)
{
    m_file = std::fopen(m_filename.c_str(), m_mode == Mode::Read ? "rb" : "wb");

#if WORDS_BIGENDIAN
    if (m_endianess == Archive::Endianess::Little)
        m_swapFn = byteSwap;
#else
    if (m_endianess == Archive::Endianess::Big)
        m_swapFn = byteSwap;
#endif
}

Archive::~Archive()
{
    if (m_mode == Mode::Write)
        std::fflush(m_file);

    std::fclose(m_file);
}

/*************************************************************************/

size_t Archive::read(std::span<uint8_t> buffer)
{
    return std::fread(buffer.data(), 1, buffer.size(), m_file);
}

void Archive::write(const std::span<uint8_t> &buffer)
{
    std::fwrite(buffer.data(), 1, buffer.size(), m_file);
}

/*************************************************************************/