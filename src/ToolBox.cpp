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

// ToolBox.cpp : implementation file
//

#include "StdAfx.h"
//#include "6502.h"
#include "ToolBox.h"

/////////////////////////////////////////////////////////////////////////////
// CToolBox dialog


CToolBox::CToolBox()
    : wxDialog()
{
}

CToolBox::~CToolBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CToolBox message handlers

bool CToolBox::Create(int rscId, wxWindow *pParentWnd)
{
    UNUSED(rscId);
    UNUSED(pParentWnd);

#if REWRITE_TO_WX_WIDGET
    rsc_id = rscId;

    int ret = CDialog::Create(IDD, pParentWnd);
    if (!ret)
        return ret;

    CString regs;
    if (regs.LoadString(rsc_id))
        SetWindowText(regs); // name (title) of the window

    created = true;
#endif

    return true;
}
