/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

// AtariBin.cpp: implementation of the CAtariBin class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "formats/AtariBin.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAtariBin::CAtariBin()
    : BinaryCodeTemplate()
{
}

CAtariBin::~CAtariBin()
{
}

/*************************************************************************/

bool CAtariBin::read(BinaryArchive &ar, LoadCodeState *state)
{
    uint16_t header = 0;
    ar & header;

    if (header != 0xFFFF)
        throw new FileError(FileError::Corrupt);

    uint16_t begin = 0;

    while (!ar.eof())
    {
        uint16_t from, to;

        ar & from;
        ar & to;

        if (to < from)
            throw new FileError(FileError::Corrupt);

        if (begin == 0)
            begin = from;

        state->Marks.SetStart(from);
        state->Marks.SetEnd(to);

        ar.read(state->Memory->getSpan(from, to));
        state->Memory->invalidate();
    }

    int start = (*state->Memory)[0x2e0] + (*state->Memory)[0x2e1] * 256; // run address

    if (start == 0)
        start = (*state->Memory)[0x2e2] + (*state->Memory)[0x2e3] * 256; // init address

    if (start == 0)
        start = begin; // beginning of first block

    if (start != 0)
        state->StartAddress = start;

    return true;
}

/*************************************************************************/
