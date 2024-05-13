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

// ColorButton.cpp : implementation file
//

#include "StdAfx.h"
#include "ColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CColorButton

CColorButton::CColorButton()
{
#if 0
    m_nDx = ::GetSystemMetrics(SM_CXEDGE);

    if (m_nDx == 0)
        m_nDx = 2;

    m_nDy = ::GetSystemMetrics(SM_CYEDGE);

    if (m_nDy == 0)
        m_nDy = 2;

    m_nDx += 3;
    m_nDy += 3;
#endif
}

void CColorButton::PaintIt(int offset)
{
    UNUSED(offset);

#if 0
    wxRECT rect = GetClientRect();

    rect.bottom -= m_nDy - offset;
    rect.right -= m_nDx - offset;
    rect.top += m_nDy + offset;
    rect.left += m_nDx + offset;

    CClientDC dc(this);
    wxBrush brush(::GetSysColor(COLOR_BTNTEXT));
    dc.FillSolidRect(&rect, m_color);
    dc.FrameRect(&rect, &brush);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CColorButton message handlers

void CColorButton::OnPaint()
{
#if 0
//  CPaintDC dc(this); // device context for painting
    CButton::OnPaint(); // call Default() -draw a button

    int x = SendMessage(BM_GETSTATE, 0, 0);
    PaintIt(x & BST_PUSHED ? 1 : 0);
#endif
}

#if 0
afx_msg LRESULT CColorButton::OnSetState(WPARAM wParam, LPARAM /* lParam */)
{
    CButton::Default();
    PaintIt(wParam ? 1 : 0);
    return 0;
}
#endif