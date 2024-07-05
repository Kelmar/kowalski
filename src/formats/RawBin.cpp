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

    if (sz <= 0xFFFF)
    {
        /*
         * We're making an assumption here that we're loading some sort of ROM with a start vector in it.
         * 
         * In which case we're going to load it into the upper part of the 64K address space.
         */
        state->LoadAddress = 0x10000 - sz;
    }

    // Anything larger than 64K is probably a dump of a 65816, we'll load from the begining.

    std::unique_ptr<LoadCodeOptionsDlg> optDlg(new LoadCodeOptionsDlg(state));

    int res = optDlg->ShowModal();

    if (res != wxID_OK)
        return false;
    
    ar.read(state->Memory->getSpan(state->StartAddress, sz));

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
