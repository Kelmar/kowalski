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
#include "AtariBin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAtariBin::CAtariBin()
{
}

CAtariBin::~CAtariBin()
{
}

bool CAtariBin::LoadAtaBinFormat(CArchive &ar, COutputMem &mem, CMarkArea &area, int &prog_start)
{
    UNUSED(ar);
    UNUSED(mem);
    UNUSED(area);
    UNUSED(prog_start);

    return false;

    // TODO: Write replacement for CArchive class.

#if 0
    uint16_t temp;

    ar >> temp;

    if (temp != 0xFFFF)
        return false;

    uint16_t begin = 0;

    for (;;)
    {
        uint16_t from, to;

        ar >> from;
        ar >> to;

        if (to < from)
            return false;

        if (begin == 0)
            begin = from;

        area.SetStart(from);
        area.SetEnd(to);

        mem.Load(ar, from, to);

        if (ar.GetFile()->GetLength() >= ar.GetFile()->GetPosition() && ar.IsBufferEmpty())
            break;
    }

    int start = mem[0x2e0] + mem[0x2e1] * 256; // run address

    if (start == 0)
        start = mem[0x2e2] + mem[0x2e3] * 256; // init address

    if (start == 0)
        start = begin; // beginning of first block

    if (start != 0)
        prog_start = start;

    return true;
#endif
}
