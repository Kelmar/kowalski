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

#ifndef RAWBIN_H__
#define RAWBIN_H__

#include "StdAfx.h"
#include "Archive.h"
#include "LoadCodeOptions.h"
#include "ProjectManager.h"

/**
 * @brief Loads raw binary files into simulator memory.
 * 
 * This class does not make an attempt to interpret the data in anyway.
 * It just raw loads/dumps the memory.
 */
class CRawBin : public BinaryCodeTemplate
{
protected:
    virtual void read(BinaryArchive &archive, LoadCodeState *state);
    virtual void write(BinaryArchive& archive, LoadCodeState *state);

public:
    /* constructor */ CRawBin();
    virtual          ~CRawBin();

    virtual bool canRead() const { return true; }

    virtual bool canWrite() const { return true; }

    virtual std::string getDescription() const { return std::string(_("Raw binary files")); }

    virtual std::vector<std::string> getExtensions() const { return { "*" }; }
};

#endif /* RAWBIN_H__ */

/*************************************************************************/
