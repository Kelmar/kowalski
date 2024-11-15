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

// Deasm6502View.h : header file
//

#ifndef DEASM_6502_VIEW_H__
#define DEASM_6502_VIEW_H__

#include "DrawMarks.h"
#include "Asm.h"

#include "Deasm6502Doc.h"

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View view

class CDeasm6502View : public wxView //, public CMarks, CAsm
{
    int m_nFontHeight;
    int m_nFontWidth;

    int CalcLineCount(const wxRect &prect);
    void Scroll(UINT nSBCODE, int nPos, int nRepeat= 1);
    void UpdateScrollRange();

    wxRect GetViewRect()
    {
        wxWindow *window = GetFrame();

        if (window)
            return window->GetClientRect();

        return wxRect(0, 0, 0, 0);
    }

    void ScrollToLine(uint32_t addr);

public:
    static wxColour m_rgbAddress;
    static wxColour m_rgbCode;
    static wxColour m_rgbInstr;
    static wxColour m_rgbBkgnd;
    static bool m_bDrawCode;
    static wxFont m_Font;
    static wxFontInfo m_LogFont;

    virtual ~CDeasm6502View();

    CDeasm6502Doc *GetDocument() { return static_cast<CDeasm6502Doc *>(wxView::GetDocument()); }

    /**
     * Invalidates the owning frame.
     */
    void Refresh()
    {
        wxWindow *window = GetFrame();

        if (window)
            window->Refresh();
    }

    void UpdateWindow()
    {
        // Stub for now. -- B.Simonds (April 27, 2024)
    }

protected:
    CDeasm6502View(); // protected constructor used by dynamic creation

public:
    //virtual void OnPrepareDC(wxDC* pDC, CPrintInfo* pInfo = NULL);
    virtual bool OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll = TRUE);
    virtual void OnInitialUpdate();

protected:
    virtual void OnDraw(wxDC* pDC);      // overridden to draw this view
    //virtual bool PreCreateWindow(CREATESTRUCT& cs);
    virtual bool OnScrollBy(wxSize sizeScroll, bool bDoScroll = true);
    virtual void OnUpdate_MFC(wxView* pSender, LPARAM lHint, wxObject* pHint);

    afx_msg LRESULT OnExitDebugger(WPARAM /* wParam */, LPARAM /* lParam */);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar);
    afx_msg bool OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg bool OnEraseBkgnd(wxDC* pDC);
    afx_msg void OnDeasmGoto();
    afx_msg void OnUpdateDeasmGoto(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

#endif /* DEASM_6502_VIEW_H__ */
