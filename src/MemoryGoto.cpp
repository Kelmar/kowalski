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

// MemoryGoto.cpp : implementation file
//

#include "StdAfx.h"
//#include "6502.h"
#include "MemoryGoto.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryGoto dialog

CMemoryGoto::CMemoryGoto()
    : wxDialog()
    , m_uAddr(0)
{
}

#if REWRITE_TO_WX_WIDGET

void CMemoryGoto::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
//  DDX_Text(pDX, IDC_MEMORY_ADDR, m_uAddr);
    DDX_HexDec(pDX, IDC_MEMORY_ADDR, m_uAddr);
    //{{AFX_DATA_MAP(CMemoryGoto)
    if (theApp.m_global.m_bProc6502==2)   // 65816
        DDV_MinMaxUInt(pDX, m_uAddr, 0, 0xFFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_uAddr, 0, 0xFFFF);
    //}}AFX_DATA_MAP
}

#endif

/////////////////////////////////////////////////////////////////////////////
// CMemoryGoto message handlers
