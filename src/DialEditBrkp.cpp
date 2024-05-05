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

// DialEditBrkp.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DialEditBrkp.h"

/////////////////////////////////////////////////////////////////////////////
// CDialEditBreakpoint dialog


CDialEditBreakpoint::CDialEditBreakpoint(CAsm::Breakpoint bp)
    : wxDialog()
{
    m_uAddr = 0;
    m_Execute = (bp & CAsm::BPT_EXECUTE) != 0;
    m_Read = (bp & CAsm::BPT_READ) != 0;
    m_Write = (bp & CAsm::BPT_WRITE) != 0;
    m_Disabled = (bp & CAsm::BPT_DISABLED) != 0;
}

#if REWRITE_TO_WX_WIDGET

void CDialEditBreakpoint::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_HexDec(pDX, IDC_ADDR, m_uAddr);
    DDV_MinMaxUInt(pDX, m_uAddr, 0, 65535);
    //{{AFX_DATA_MAP(CDialEditBreakpoint)
    DDX_Check(pDX, IDC_EDIT_BP_DISABLED, m_Disabled);
    DDX_Check(pDX, IDC_EDIT_BP_EXEC, m_Execute);
    DDX_Check(pDX, IDC_EDIT_BP_READ, m_Read);
    DDX_Check(pDX, IDC_EDIT_BP_WRITE, m_Write);
    DDX_Text(pDX, IDC_ADDR, m_uAddr);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDialEditBreakpoint, CDialog)
//{{AFX_MSG_MAP(CDialEditBreakpoint)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialEditBreakpoint message handlers

CAsm::Breakpoint CDialEditBreakpoint::GetBreakpoint()
{
    int bp = CAsm::BPT_NONE;

    if (m_Execute)
        bp |= CAsm::BPT_EXECUTE;

    if (m_Read)
        bp |= CAsm::BPT_READ;

    if (m_Write)
        bp |= CAsm::BPT_WRITE;

    if (m_Disabled)
        bp |= CAsm::BPT_DISABLED;

    return (CAsm::Breakpoint)bp;
}
