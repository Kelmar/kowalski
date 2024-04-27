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
#include "DebugInfo.h"

CAsm::Breakpoint CDebugInfo::SetBreakpoint(int line, FileUID file, int bp)
{
    ASSERT( (bp & ~BPT_MASK) == 0 ); // Illegal combination of breakpoint bits

    CDebugLine dl;
    GetAddress(dl, line, file); // Find the address corresponding to the line

    if (dl.flags == DBG_EMPTY || (dl.flags & DBG_MACRO))
        return BPT_NO_CODE; // There is no code in line 'line'

    if (bp == BPT_NONE) // Breakpoint type not specified?
        bp = dl.flags & DBG_CODE ? BPT_EXECUTE : BPT_READ | BPT_WRITE | BPT_EXECUTE;

    return m_breakpoints.Set(dl.addr,bp);
}

CAsm::Breakpoint CDebugInfo::ModifyBreakpoint(int line, FileUID file, int bp)
{
    // Illegal combination of breakpoint bits
    ASSERT((bp & ~(BPT_MASK | BPT_DISABLED)) == 0);

    CDebugLine dl;
    GetAddress(dl, line, file); // Finding the address corresponding to the line

    if (dl.flags == DBG_EMPTY || (dl.flags & DBG_MACRO))
        return BPT_NO_CODE; // There is no code in line 'line'

    if ((bp & BPT_MASK) == BPT_NONE)
    {
        m_breakpoints.ClrBrkp(dl.addr); // Clearing the breakpoint
        return BPT_NONE;
    }

    bp = m_breakpoints.Set(dl.addr,bp & ~BPT_DISABLED);
    m_breakpoints.Enable(dl.addr, !(bp & BPT_DISABLED));

    return (Breakpoint)bp;
}

CAsm::Breakpoint CDebugInfo::GetBreakpoint(int line, FileUID file)
{
    CDebugLine dl;
    GetAddress(dl, line, file); // Finding the address corresponding to the line

    if (dl.flags == DBG_EMPTY || (dl.flags & DBG_MACRO))
    {
        //ASSERT(false);
        return BPT_NO_CODE; // There is no code in line 'line'
    }

    return m_breakpoints.Get(dl.addr);
}

CAsm::Breakpoint CDebugInfo::ToggleBreakpoint(int line, FileUID file)
{
    CDebugLine dl;
    GetAddress(dl, line, file); // Find the address corresponding to the line

    if (dl.flags == DBG_EMPTY || (dl.flags & DBG_MACRO))
        return BPT_NO_CODE; // There is no code in line 'line'

    if (m_breakpoints.Get(dl.addr) != BPT_NONE) // Is the interrupt already set?
        return m_breakpoints.Clr(dl.addr);
    else
        return m_breakpoints.Set(dl.addr, dl.flags & DBG_CODE ? BPT_EXECUTE : BPT_READ | BPT_WRITE | BPT_EXECUTE);
}

void CDebugInfo::ClrBreakpoint(int line, FileUID file)
{
    CDebugLine dl;
    GetAddress(dl, line, file);// Find the address corresponding to the line

    if (dl.flags == DBG_EMPTY || (dl.flags & DBG_MACRO))
    {
        ASSERT(false); // There is no code in line 'line'
        return;
    }

    m_breakpoints.Clr(dl.addr);
}
