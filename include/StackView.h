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

// StackView.h : header file
//

#ifndef _StackView_
#define _StackView_

#include "MemoryDoc.h"

// This looks duplicated from MemoryView. -- B.Simonds (April 27, 2024)

/////////////////////////////////////////////////////////////////////////////
// CStackView view

class CStackView : public wxView
{
private:
    int m_nCx;      // Number of columns
    int m_nCy;      // Number of lines
    int m_nChrW;    // Character width (mono font)
    int m_nChrH;    // Character height

    void calc(wxDC *pDC);
    void scroll(UINT nSBCODE, int nPos, int nRepeat = 1);
    int set_scroll_range();

    void get_view_rect(wxRect &rect)
    {
        wxWindow *window = GetFrame();

        if (window)
            rect = window->GetClientRect();
    }

    int bytes_in_line()
    {
        int lim = (m_nCx - 17) / 3; // Number of bytes displayed in one line
        return lim <= 0 ? 1 : lim;
    }

    int find_prev_addr(uint16_t &addr, const COutputMem &mem, int cnt = 1, int bytes = 0);
    int find_next_addr(uint16_t &addr, const COutputMem &mem, int cnt = 1, int bytes = 0);
    int find_delta(uint16_t &addr, uint16_t dest, const COutputMem &mem, int max_lines);

protected:
    CStackView();           // protected constructor used by dynamic creation

public:
    static wxFont m_Font;
    static wxFontInfo m_LogFont;
    static wxColour m_rgbTextColor;
    static wxColour m_rgbBkgndColor;

    virtual ~CStackView();

    //virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
    virtual void OnInitialUpdate();

protected:
    virtual void OnDraw(wxDC* pDC);      // overridden to draw this view
    //virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdate(wxView* pSender, LPARAM lHint, wxObject* pHint);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar);
    afx_msg bool OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg bool OnEraseBkgnd(wxDC* pDC);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    afx_msg void OnUpdateMemoryGoto(CCmdUI* pCmdUI);
    afx_msg void OnMemoryGoto();
    afx_msg void OnUpdateMemoryChg(CCmdUI* pCmdUI);
    afx_msg void OnMemoryChg();
};

/////////////////////////////////////////////////////////////////////////////

#endif
