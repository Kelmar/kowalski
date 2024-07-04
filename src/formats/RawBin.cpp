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

// TODO: This should get moved into a processor specific defintion later
#define START_ADDRESS 0x0000FFFD

/*************************************************************************/

CRawBin::CRawBin()
    : BinaryCodeTemplate()
{
}

CRawBin::~CRawBin()
{
}

/*************************************************************************/

void CRawBin::read(BinaryArchive &ar, LoadCodeState *state)
{
    ProcessorType procType = wxGetApp().m_global.GetProcType();
    size_t maxSize = procType == ProcessorType::WDC65816 ? 0x00FFFFFF : 0x0000FFFF;

    size_t sz = std::min(ar.size(), maxSize);

    ar.read(state->Memory->getSpan(0, sz));

    state->StartAddress = state->Memory->getWord(START_ADDRESS);
}

/*************************************************************************/

void CRawBin::write(BinaryArchive &ar, LoadCodeState *state)
{
    ProcessorType procType = wxGetApp().m_global.GetProcType();
    size_t sz = procType == ProcessorType::WDC65816 ? 0x00FFFFFF : 0x0000FFFF;

    ar.write(state->Memory->getSpan(0, sz));
}

/*************************************************************************/
