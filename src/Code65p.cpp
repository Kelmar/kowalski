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

// Code65p.cpp: implementation of the CCode65p class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Code65p.h"
#include "resource.h"
#include "MarkArea.h"
#include "Sym6502.h"

bool CCode65p::SaveCode65p(CArchive &archive, COutputMem &mem, uint32_t start, uint32_t end)
{
    UNUSED(archive);
    UNUSED(mem);
    UNUSED(start);
    UNUSED(end);

#if 0
    if (start > 0xFFFF) // 1.3.3 support for 24-bit addressing
    {
        CString cs;
        cs.Format("The Program File format does not support memory above 65,535");
        MessageBoxA(NULL, cs, "Error", MB_OK );
    }
    else
    {
        int header = 0xFFFF;

        archive.Write(&header, 2);
        archive.Write(&start, 2);
        archive.Write(&end, 2);
        archive.Write(&mem[start], end - start + 1);
    }
#endif

    return true;
}

bool CCode65p::SaveCode65p(CArchive &archive, COutputMem &mem, CMarkArea &area, int prog_start)
{
    UNUSED(archive);
    UNUSED(mem);
    UNUSED(area);
    UNUSED(prog_start);

#if 0
    int header = 0xFFFF;
    archive.Write(&header, 2);

    for (UINT part = 0; part<area.GetSize(); part++)
    {
        int start, end;

        area.GetPartition(part, start, end);

        if (part == 0)
        {
            if (prog_start == -1)
                prog_start = start;
        }

        ASSERT(start >= 0 && start <= 0xFFFF);
        ASSERT(end >= 0 && end <= 0xFFFF);
        ASSERT(start <= end);

        if (start > 0xFFFF) // 1.3.3 support for 24-bit addressing
        {
            CString cs;
            cs.Format("The Program File format does not support memory above 65,535");
            MessageBoxA(NULL, cs, "Error", MB_OK );
        }
        else
        {
            archive.Write(&start, 2);
            archive.Write(&end, 2);
            archive.Write(&mem[start], end - start + 1);
        }
    }
#endif

    return true;
}

void CCode65p::LoadCode65p(CArchive &archive, COutputMem &mem)
{
    UNUSED(archive);
    UNUSED(mem);

#if 0
    uint16_t wTemp;
    archive >> wTemp;

    if (wTemp != 0xFFFF)
        throw new CFileException(CFileException::invalidFile);

    int nLen = static_cast<int>(archive.GetFile()->GetLength() - 2);

    do
    {
        uint16_t from, to;
        archive >> from;

        if (from == 0xFFFF)
        {
            nLen -= 2;
            archive >> from;
        }

        archive >> to;
        nLen -= 4;

        if (to < from)
            throw new CFileException(CFileException::invalidFile);

        mem.Load(archive, from, to);
        nLen -= to - from + 1;
    } while (nLen > 0);
    //while (archive.GetFile()->GetLength() > archive.GetFile()->GetPosition());
#endif
}

