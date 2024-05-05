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

// MemoryView.h : header file
//

#ifndef _MemoryView_
#define _MemoryView_

#include "MemoryDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryView view

class CMemoryView : public wxView //CView
{
private:
    int m_nCx;      // Number of columns
    int m_nCy;      // Number of lines
    int m_nChrW;    // Character width (mono font)
    int m_nChrH;    // Character height

    int max_mem;

    void calc(wxDC *dc);
    void scroll(UINT nSBCODE, int nPos, int nRepeat = 1);
    int set_scroll_range();

    void get_view_rect(wxRect &rect)
    {
        wxWindow *window = GetFrame();

        if (window)
            rect = window->GetClientRect();
        else
            rect = wxRect(0, 0, 0, 0);
    }

    int bytes_in_line();

    int find_prev_addr(uint32_t &addr, const COutputMem &mem, int cnt = 1, int bytes = 0);
    int find_next_addr(uint32_t &addr, const COutputMem &mem, int cnt = 1, int bytes = 0);
    int find_delta(uint32_t &addr, uint32_t dest, const COutputMem &mem, int max_lines);

    void Refresh()
    {
        wxWindow *frame = GetFrame();
        
        if (frame)
            frame->Refresh();
    }

    enum Dump { FULL, HEX, TEXT } m_eDump;
protected:
    /* constructor */ CMemoryView(); // protected constructor used by dynamic creation

public:
    virtual ~CMemoryView();
    
    // Attributes
public:
    static wxFont m_Font;
    static wxFontInfo m_LogFont;
    static wxColour m_rgbTextColor;
    static wxColour m_rgbBkgndColor;

public:
    //virtual void OnPrepareDC(wxDC *dc, CPrintInfo* pInfo = NULL);
    virtual void OnInitialUpdate();

protected:
    virtual void OnDraw(wxDC *dc); // overridden to draw this view
    //virtual bool PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdate(wxView *sender, LPARAM lHint, wxObject *hint);

    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar);
    afx_msg bool OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg bool OnEraseBkgnd(wxDC *pDC);
    afx_msg void OnContextMenu(wxWindow* wnd, wxPoint point);
    afx_msg void OnUpdateMemoryGoto(CCmdUI* pCmdUI);
    afx_msg void OnMemoryGoto();
    afx_msg void OnUpdateMemoryChg(CCmdUI* pCmdUI);
    afx_msg void OnMemoryChg();
    afx_msg void OnMemoryFull();
    afx_msg void OnMemoryHex();
    afx_msg void OnMemoryText();
    afx_msg void OnUpdateMemoryFull(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMemoryHex(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMemoryText(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

#endif
