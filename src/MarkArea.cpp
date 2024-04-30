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


#include "StdAfx.h"
#include "MarkArea.h"


void CMarkArea::SetEnd(int end)
{
    ASSERT(start >= 0); // 'SetStart' must be called before 'SetEnd'

    if (end < start) // wrong edges of the intervals?
        return;

    Pair pair = { start, end };
    
    for (UINT i = 0; i < n; i++)
    {
        if (arr[i].a > end || arr[i].b < start)
            continue; // separate intervals

        if (arr[i].a <= start)
        {
            if (arr[i].b >= end)
                return; // The new pair fits in the range
            else
            {
                arr[i].b = end; // shifting the end of the interval
                return;
            }
        }
        else if (arr[i].b <= end)
        {
            arr[i].a = start; // shifting the beginning of the interval
            return;
        }
        else
        {
            arr[i].a = start; // Widening the entire range
            arr[i].b = end;
            return;
        }
    }

    arr[n] = pair;
    n++;
}

bool CMarkArea::GetPartition(UINT no, int &a, int &b)
{
    if (no >= n)
        return false;

    a = arr[no].a;
    b = arr[no].b;
    
    return true;
}
