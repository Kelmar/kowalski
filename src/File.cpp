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

#include "StdAfx.h"

#include "File.h"

/*************************************************************************/

File::File(const std::string &name)
    : m_file(nullptr)
    , m_eof(false)
{
    UNUSED(name);
}

File::~File(void)
{
    if (m_file)
        std::fclose(m_file);
}

/*************************************************************************/

void File::Flush(void)
{
    std::fflush(m_file);
}

/*************************************************************************/

std::string File::ReadLine()
{
    char buf[512];
    return std::fgets(buf, sizeof(buf), m_file);
}

/*************************************************************************/

char File::ReadChar()
{
    return 0;
}

/*************************************************************************/

void File::WriteLine(const std::string &line)
{
    std::fputs(line.c_str(), m_file);
}

/*************************************************************************/
