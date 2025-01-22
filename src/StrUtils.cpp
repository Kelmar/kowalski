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

#include "StdAfx.h"
#include "StrUtils.h"

/*=======================================================================*/

/**
 * @brief Extract the directory information from a path.
 */
std::string file::getDirectory(const std::string &path)
{
    size_t idx = path.find_last_of('/');

#if WIN32
    if (idx == std::string::npos)
        idx = path.find_last_of('\\');
#endif

    if (idx == std::string::npos)
        return str::empty;

    return path.substr(0, idx);
}

/*=======================================================================*/
/**
 * @brief Extract the filename from a path.
 */
std::string file::getFilename(const std::string &path)
{
    size_t idx = path.find_last_of('/');

#if WIN32
    if (idx == std::string::npos)
        idx = path.find_last_of('\\');
#endif

    if (idx == std::string::npos)
        return path;

    return path.substr(idx);
}

/*=======================================================================*/
/**
 * @brief Extract the extension of a file or path.
 */
std::string file::getExtension(const std::string &path)
{
    std::string filename = getFilename(path);

    size_t idx = filename.find_last_of('.');

    if (idx != std::string::npos)
        return filename.substr(idx);

    return str::empty;
}

/*=======================================================================*/
