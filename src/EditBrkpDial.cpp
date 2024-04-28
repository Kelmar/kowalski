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

// EditBrkpDial.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "EditBrkpDial.h"

/////////////////////////////////////////////////////////////////////////////
// CDialEditBreakpoint dialog


CDialEditBreakpoint::CDialEditBreakpoint(CAsm::Breakpoint bp)
    : wxDialog()
{
    m_Execute = (bp & CAsm::BPT_EXECUTE) != 0;
    m_Read = (bp & CAsm::BPT_READ) != 0;
    m_Write = (bp & CAsm::BPT_WRITE) != 0;
    m_Disabled = (bp & CAsm::BPT_DISABLED) != 0;
}

