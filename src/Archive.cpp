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
#include "StrUtils.h"

#include <iostream>
#include <fstream>

#include "Archive.h"

/*************************************************************************/

Archive::Archive(
    const std::string &filename,
    Archive::Mode mode)
    : m_filename(filename)
    , m_file(nullptr)
    , m_size(0)
    , m_mode(mode)
{
    // Note that we open as binary, we'll process the line endings ourselves.
    m_file = std::fopen(m_filename.c_str(), m_mode == Mode::Read ? "rb" : "wb");

    if (m_file == nullptr)
        throw FileError(FileError::ErrorCode::SysError);

    if (m_mode == Mode::Read)
    {
        // Find size of file.
        std::fseek(m_file, 0, SEEK_END);
        m_size = std::ftell(m_file);
        std::fseek(m_file, 0, SEEK_SET);
    }
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

/*************************************************************************/
/*************************************************************************/

TextArchive::TextArchive(const std::string &filename,
    Archive::Mode mode,
    LineEnding lineEnding)
    : Archive(filename, mode)
    , m_lineEnding(lineEnding)
{
}

/*************************************************************************/

char TextArchive::getChar()
{
    if (m_buffer.empty())
    {
        uint8_t buf[1024];
        std::span<uint8_t> b(buf, sizeof(buf));
        size_t sz = read(b);

        if (sz > 0)
            m_buffer.enqueue(b.subspan(0, sz));
    }

    return static_cast<char>(m_buffer.dequeue());
}

/*************************************************************************/

char TextArchive::peek()
{
    if (m_buffer.empty())
    {
        uint8_t buf[1024];
        std::span<uint8_t> b(buf, sizeof(buf));
        size_t sz = read(b);

        if (sz > 0)
            m_buffer.enqueue(b.subspan(0, sz));
    }

    return static_cast<char>(m_buffer.peek());
}

/*************************************************************************/

std::string TextArchive::readLine()
{
    std::string rval;
    rval.reserve(1024);

    while (!eof())
    {
        char c = getChar();

        if (c == '\0')
            break;

        rval += c;

        if (c == '\r')
        {
            // Check for DOS line ending
            c = peek();

            if (c == '\n')
                rval += getChar(); // Consume line feed as well.

            break;
        }

        if (c == '\n')
            break;
    }

    return rval;
}

/*************************************************************************/

void TextArchive::writeLine(const std::string &s)
{
    std::string line = s | str::rtrim; // Make sure there is no extra line endings

    switch (m_lineEnding)
    {
    case LineEnding::DOS:
        line += "\r\n";
        break;

    case LineEnding::UNIX:
        line += "\n";
        break;

    case LineEnding::MAC:
        line += "\r";
        break;
    }

    write(line);
}

/*************************************************************************/
/*************************************************************************/

BinaryArchive::BinaryArchive(const std::string &filename,
    Archive::Mode mode,
    BinaryArchive::Endianess endianess /* = Endianess::None */)
    : Archive(filename, mode)
    , m_endianess(endianess)
    , m_swapFn(noSwap)
{
#if WORDS_BIGENDIAN
    if (m_endianess == Endianess::Little)
        m_swapFn = byteSwap;
#else
    if (m_endianess == Endianess::Big)
        m_swapFn = byteSwap;
#endif
}

/*************************************************************************/

