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
#include "6502.h"

#include "formats/RawBin.h"

/*************************************************************************/

CRawBin::CRawBin()
    : BinaryCodeTemplate()
{
}

CRawBin::~CRawBin()
{
}

/*************************************************************************/

bool CRawBin::read(BinaryArchive &ar, LoadCodeState *state)
{
    ProcessorType procType = wxGetApp().m_global.GetProcType();
    size_t maxSize = procType == ProcessorType::WDC65816 ? 0x00FFFFFF : 0x0000FFFF;

    size_t sz = std::min(ar.size(), maxSize);
    std::vector<uint8_t> data(sz);
    std::span<uint8_t> span(data);

    size_t rd = ar.read(span);

    if (rd < sz)
    {
        // Should have been able to read the full block size.
        throw FileError(FileError::ErrorCode::SysError);
    }

    if (sz <= 0xFFFF)
    {
        /*
         * We're making an assumption here that we're loading some sort of ROM with a start vector in it.
         * 
         * In which case we're going to load it into the upper part of the 64K address space.
         */
        state->LoadAddress = 0x10000 - sz;

        // Try reading start address from the reset vector.
        state->StartAddress = span[sz - 3] << 8 | span[sz - 4];
    }

    // Anything larger than 64K is probably a dump of a 65816, we'll load from the beginning.

    std::unique_ptr<LoadCodeOptionsDlg> optDlg(new LoadCodeOptionsDlg(state));

    int res = optDlg->ShowModal();

    if (res != wxID_OK)
        return false;
    
    state->Memory->copy(state->LoadAddress, span);

    return true;
}

/*************************************************************************/

bool CRawBin::write(BinaryArchive &ar, LoadCodeState *state)
{
    ProcessorType procType = wxGetApp().m_global.GetProcType();
    size_t sz = procType == ProcessorType::WDC65816 ? 0x00FFFFFF : 0x0000FFFF;

    ar.write(state->Memory->getSpan(0, sz));

    return true;
}

/*************************************************************************/
