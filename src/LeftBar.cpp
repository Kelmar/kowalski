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

// LeftBar.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "LeftBar.h"
#include "MemoryDC.h"
#include "6502View.h"

/////////////////////////////////////////////////////////////////////////////
// CLeftBar

CLeftBar::CLeftBar()
    : m_barWidth(0)
    , m_editView(nullptr)
{
}

CLeftBar::~CLeftBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLeftBar message handlers

wxSize CLeftBar::MySize()
{
    wxRect rect = GetParent()->GetClientRect();
    return wxSize(m_barWidth, rect.Height());
}

bool CLeftBar::Create(dxWindow *parent, CSrc6502View* view)
{
    m_editView = view;

    wxRect rect(0, 0, 0, 0);

/*
    bool bRet = !!CWnd::CreateEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE,
                                rect, parent, AFX_IDW_CONTROLBAR_LAST);

    m_dwStyle = CBRS_ALIGN_LEFT;

    return bRet;
*/

    return false;
}

static const int LEFT_MARGIN = 1;       // margin for markers
static const int LEFT_ERR_MARGIN = 4;   // margin for error marker

void CLeftBar::DoPaint(wxPaintEvent &event)
{
    wxPaintDC dc(this);

    wxRect rect = GetClientRect();

    wxBitmap offScreen;
    offScreen.CreateWithDPISize(GetClientSize(), GetDPIScaleFactor());

    {
        wxMemoryDC dcMem(offScreen);

        if (m_pEditView != 0)
        {
            int topLine = 0, lineCount = 0, lineHeight = 0;

            m_editView->GetDispInfo(topLine, lineCount, lineHeight);

            if (lineHeight > 0)
            {
                //int ctrlBottom = (rect.Height() + lineHeight - 1) / lineHeight;

                int pointerLine = m_editView->GetPointerLine();
                int errMarkLine = m_editView->GetErrorMarkLine();

                int line = topLine;

                for (int y = 0; y < rect.Height(); y += lineHeight, ++line)
                {
                    if (line > lineCount)
                        break;

                    if (uint8_t bp = m_editView->GetBreakpoint(line))
                        CMarks::draw_breakpoint(dcMem, LEFT_MARGIN, y, nLineHeight, !(bp & CAsm::BPT_DISABLED));

                    if (pointerLine == line)
                        CMarks::draw_pointer(dcMem, LEFT_MARGIN, y, nLineHeight);

                    if (errMarkLine == line)
                        CMarks::draw_mark(dcMem, LEFT_ERR_MARGIN, y, nLineHeight);
                }
            }
        }
    } // End of memDC

    dc.DrawBitmap(offScreen, wxPoint(0, 0));
}

void CLeftBar::SetWidth(int width)
{
    m_barWidth = width + 1;
    Refresh();
}

void CLeftBar::RedrawLine(int nLine)
{
    Refresh();
    //CClientDC dc(this);
    //DoPaint(&dc);
}
