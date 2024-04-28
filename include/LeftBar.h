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

#ifndef LEFT_BAR_H__
#define LEFT_BAR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeftBar.h : header file
//
#include "DrawMarks.h"
class CSrc6502View;

/////////////////////////////////////////////////////////////////////////////
// CLeftBar window

/**
 * Draws breakpoints and current line markers for debgging runs.
 */
//class CLeftBar : public CControlBar, public CMarks
class CLeftBar : public wxControl
{
private:
    int m_barWidth;
    CSrc6502View* m_editView;

public:
    CLeftBar();

    bool Create(wxWindow* parent, CSrc6502View* view);

    void SetWidth(int nWidth);

    void RedrawLine(int nLine);

// Implementation
public:
    virtual ~CLeftBar();

    virtual wxSize CalcFixedLayout(bool, bool) { return MySize(); }
    virtual wxSize CalcDynamicLayout(int, uint32_t) { return MySize(); }

#if 0
    virtual void OnUpdateCmdUI(CFrameWnd *target, bool disableIfNoHndler)
    {
    }
#endif

    wxSize MySize();

    virtual void DoPaint(wxPaintEvent &event);
};

#endif /* LEFT_BAR_H__ */
