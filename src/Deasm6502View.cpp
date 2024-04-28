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

// Deasm6502View.cpp : implementation file
//

#include "StdAfx.h"
//#include "6502.h"
#include "Deasm6502View.h"
#include "Deasm6502Doc.h"
#include "resource.h"
#include "DeasmGoto.h"
#include "Deasm.h"

wxFont CDeasm6502View::m_Font;
wxFontInfo CDeasm6502View::m_LogFont;
wxColour CDeasm6502View::m_rgbBkgnd;
wxColour CDeasm6502View::m_rgbAddress = wxColour(127, 127, 127);
wxColour CDeasm6502View::m_rgbCode = wxColour(191, 191, 191);
wxColour CDeasm6502View::m_rgbInstr = wxColour(0, 0, 0);
bool CDeasm6502View::m_bDrawCode = true;

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View

CDeasm6502View::CDeasm6502View()
{
    m_nFontHeight = 0;
    m_nFontWidth = 0;
}

CDeasm6502View::~CDeasm6502View()
{
}

#if 0
BEGIN_MESSAGE_MAP(CDeasm6502View, CView)
    ON_WM_CONTEXTMENU()
//{{AFX_MSG_MAP(CDeasm6502View)
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()  // 1.3.2 added mouse scroll wheel support
    ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_DEASM_GOTO, OnDeasmGoto)
    ON_UPDATE_COMMAND_UI(ID_DEASM_GOTO, OnUpdateDeasmGoto)
    ON_WM_CONTEXTMENU()
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
    ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebugger)
END_MESSAGE_MAP()
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View drawing

int CDeasm6502View::CalcLineCount(const wxRect &rect) // calc. number of lines in a rect
{
    if (m_nFontHeight == 0)
        return 1;	// not yet ready

    int h = rect.GetHeight();

    if (h >= m_nFontHeight)
        return h / m_nFontHeight; // no of rows in window
    else
        return 1; // always at least one row
}

void CDeasm6502View::OnDraw(wxDC *dc) // Disassembled program -display instructions
{
#if 0
    CDeasm6502Doc *pDoc = GetDocument();

    if (pDoc == NULL)
        return;

    wxRect rect = GetViewRect();
    int lines = CalcLineCount(rect);

    rect.bottom = rect.top + m_nFontHeight;
    wxRect mark = rect; // Space for indicators

    rect.left += m_nFontHeight; // Left margin
    
    if (rect.left >= rect.right)
        rect.right = rect.left;

    CDeasm deasm;

    int ptr = pDoc->m_uStartAddr;

    for (int i = 0; i <= lines; i++)
    {
        if (dc->RectVisible(&mark)) // Refresh pointer fields?
        {
            Breakpoint bp = wxGetApp().m_global.GetBreakpoint(uint16_t(ptr));

            if (bp & BPT_MASK) // Is there a break point?
                draw_breakpoint(*dc, mark.left, mark.top, m_nFontHeight, bp & BPT_DISABLED ? false : true);

            if (pDoc->m_nPointerAddr == ptr) // Arrow in this line?
                draw_pointer(*dc, mark.left, mark.top, m_nFontHeight);
        }

        const std::string &str =
            deasm.DeasmInstr(*pDoc->m_pCtx, CDeasm::DeasmFmt(CDeasm::DF_ADDRESS | CDeasm::DF_CODE_BYTES), ptr);

        if (dc->RectVisible(&rect)) // Statement line to refresh?
        {
            dc->SetTextColor(m_rgbAddress);
            //pDC->TextOut(rect.left, rect.top, LPCTSTR(str), 4);
            dc->TextOut(rect.left, rect.top, LPCTSTR(str), 6); // 65816
            
            if (m_bDrawCode)
            {
                dc->SetTextColor(m_rgbCode);
                dc->TextOut(rect.left + m_nFontWidth * 6, rect.top, LPCTSTR(str) + 6, 11); // 65816
            }

            dc->SetTextColor(m_rgbInstr);
            dc->TextOut(rect.left + m_nFontWidth * 19, rect.top, LPCTSTR(str) + 19, str.GetLength() - 19); // 65816
        }

        rect.top += m_nFontHeight;
        rect.bottom += m_nFontHeight;
        mark.top += m_nFontHeight;
        mark.bottom += m_nFontHeight;
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502View message handlers

#if 0
void CDeasm6502View::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    if (pInfo || pDC->IsPrinting())
        return;

    pDC->SetBkMode(OPAQUE);
    pDC->SelectObject(&m_Font);
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    m_nFontHeight = (int)tm.tmHeight + (int)tm.tmExternalLeading;
    m_nFontWidth = tm.tmAveCharWidth;
    pDC->SetBkColor(m_rgbBkgnd);

    CView::OnPrepareDC(pDC, pInfo);
}

BOOL CDeasm6502View::PreCreateWindow(CREATESTRUCT& cs)
{
    bool ret= CView::PreCreateWindow(cs);
    cs.style |= WS_VSCROLL;
    return ret;
}
#endif

//=============================================================================

void CDeasm6502View::ScrollToLine(uint32_t addr)
{
    CDeasm6502Doc *pDoc = (CDeasm6502Doc *)GetDocument();

    if (pDoc == NULL)
        return;

    if (pDoc->m_uStartAddr == addr) // Is the given address at the current start?
        return;

    if (pDoc->m_uStartAddr > addr) // The desired address is before the current start
    {
        wxRect rect = GetViewRect();
        int lines = CalcLineCount(rect);

        CDeasm deasm;

        uint32_t start = addr;
        bool redraw = true;
        
        for (int i = 0; i < lines; i++) // We go down from 'addr' until we meet 'm_uStartAddr'
        {
            // or the number of available lines runs out
            if (deasm.FindNextAddr(start,*pDoc->m_pCtx) == 0)
                break; // "scroll" the address

            if (start > pDoc->m_uStartAddr)
                break; // Orders don't match each other, we redraw everything

            if (start == pDoc->m_uStartAddr)
            {
                int y= (i + 1) * m_nFontHeight;
                /*
                	if ((rect.bottom -= y) <= 0)
                	{
                	  ASSERT(false);
                	  break;
                	}
                */
                UpdateWindow(); // To avoid problems with refreshing the anime
                pDoc->m_uStartAddr = addr;
                //ScrollWindow(0, y, &rect, &rect);
                UpdateWindow();
                redraw = false;
                break;
            }
        }

        if (redraw)
        {
            pDoc->m_uStartAddr = addr;
            Refresh();
            UpdateWindow();
        }
    }
    else // The requested address is after the current start
    {
        wxRect rect = GetViewRect();
        int lines = CalcLineCount(rect);

        CDeasm deasm;

        uint32_t start = pDoc->m_uStartAddr;
        bool redraw = true;

        int i;
        for (i = 1; i < lines; i++) // We go down from 'm_uStartAddr' until we meet 'addr'
        {
            // or the number of available lines runs out
            if (deasm.FindNextAddr(start, *pDoc->m_pCtx) == 0)
                break; // "scroll" the address

            if (start > addr)
                break; // Orders don't match each other, we redraw everything

            if (start == addr)
                return; // The arrow fits in the window
        }

        if (i == lines) // previous loop ended normally (not with 'break')?
        {
            for (int i = 0; i < lines; i++) // We go from 'start' down until we meet 'addr'
            {
                // Or the number of available lines runs out
                if (deasm.FindNextAddr(start, *pDoc->m_pCtx) == 0)
                    break; // "scroll" the address

                if (start > addr)
                    break; // Orders don't match each other, we redraw everything

                if (start == addr)
                {
                    int y = (i + 1) * m_nFontHeight;
                    /*
                    	  if ((rect.top += y) >= rect.bottom)
                    	  {
                    	    ASSERT(false);
                    	    break;
                    	  }
                    */
                    UpdateWindow(); // To avoid refreshing problems

                    for (int j = 0; j <= i; j++) // Designating a new address for the start of the window
                        deasm.FindNextAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx);

                    //ScrollWindow(0, -y, &rect, &rect);
                    UpdateWindow();
                    redraw = false;
                    break;
                }
            }
        }

        if (redraw)
        {
            pDoc->m_uStartAddr = addr;
            Refresh();
            UpdateWindow();
        }
    }

    return;
}

bool CDeasm6502View::OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll)
{
    if (bDoScroll)
        return true;

    //return CView::OnScroll(nScrollCode, nPos, bDoScroll);
    return false;
}

bool CDeasm6502View::OnScrollBy(wxSize sizeScroll, bool bDoScroll)
{
    //return CView::OnScrollBy(sizeScroll, bDoScroll);
    return false;
}

void CDeasm6502View::OnInitialUpdate()
{
    //CView::OnInitialUpdate();

    UpdateScrollRange();

#if 0
    CDeasm6502Doc *doc = GetDocument();

    if (doc)
        SetScrollPos(SB_VERT, (int)doc->m_uStartAddr - 0x8000);  // 65816 fix

    m_Font.DeleteObject();
    m_Font.CreateFontIndirect(&m_LogFont);
#endif
}

//-----------------------------------------------------------------------------
// Handle changes to the scroll bar values
//
void CDeasm6502View::Scroll(UINT nSBCode, int nPos, int nRepeat)
{
#if 0

    CDeasm6502Doc *pDoc = (CDeasm6502Doc*)GetDocument();
    if (pDoc == NULL)
        return;
//  UINT8 cmd;
    CDeasm deasm;

    switch (nSBCode)
    {
    case SB_ENDSCROLL:	// End scroll
        break;

    case SB_LINEDOWN:	// Scroll one line down
        switch (deasm.FindNextAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx))
        {
        case 0:
            break;	// dalej ju� si� nie da

        case 1:
            wxRect rect = GetViewRect();
            //UpdateWindow(); // To avoid refreshing problems
            ScrollWindow(0, -m_nFontHeight, &rect, &rect);
            UpdateWindow();
            break;
        }
        /*
              cmd = pDoc->m_pCtx->mem[pDoc->m_uStartAddr];	// pierwszy rozkaz w oknie
              pDoc->m_uStartAddr = (pDoc->m_uStartAddr + mode_to_len[code_to_mode[cmd]]) & pDoc->m_pCtx->mem_mask;
              wxRect rect = GetViewRect();
              UpdateWindow(); // To avoid refreshing problems
              ScrollWindow(0, -m_nFontHeight, &rect, &rect);
        */
        break;

    case SB_LINEUP:	// Scroll one line up
        switch (deasm.FindPrevAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx))
        {
        case 0:
            break;	// jeste�my ju� na pocz�tku

        case 1:
            wxRect rect = GetViewRect();
            //if ((rect.bottom -= m_nFontHeight) <= 0)
            //  break;
            //UpdateWindow(); // To avoid refreshing problems
            ScrollWindow(0, m_nFontHeight, &rect, &rect);
            UpdateWindow();
            break;

        case -1:
            Refresh(); // Redrawing the entire window -several commands have changed
            break;

        default:
            ASSERT(false);
            break;
        }
        break;

    case SB_PAGEDOWN: // Scroll one page down
    {
        wxRect rect = GetViewRect();

        switch (deasm.FindNextAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx, CalcLineCount(rect)))
        {
        case 0:
            break;	// It can't go any further

        case 1:
            Refresh(); // Redraw the entire window
            break;
        }
        break;
    }

    case SB_PAGEUP: // Scroll one page up
    {
        wxRect rect = GetViewRect();

        switch (deasm.FindPrevAddr(pDoc->m_uStartAddr, *pDoc->m_pCtx, CalcLineCount(rect)))
        {
        case 0:
            break; // We are already at the beginning

        case 1:
        case -1:
            Refresh(); // Redrawing the entire window -several commands have changed
            break;

        default:
            ASSERT(false);
            break;
        }
        break;
    }

    case SB_TOP:	// Scroll to top
    {
        wxRect rect = GetViewRect();
        int dy = CalcLineCount(rect); // Number of lines in the window

        int lines = deasm.FindDelta(pDoc->m_uStartAddr, 0, *pDoc->m_pCtx, dy);

        if (lines < 0)
            Refresh(); // Redraw the entire window
        else if (lines > 0)
        {
            if (lines >= dy)
                Refresh(); // Redraw the entire window
            else
            {
                //Refresh(); // Redraw the entire window
                ScrollWindow(0,lines * m_nFontHeight,&rect,&rect);
            }
        }
        break;
    }

    case SB_BOTTOM:	// Scroll to bottom
    {
        wxRect rect = GetViewRect();
        int dy = CalcLineCount(rect); // Number of lines in the window

        int lines = deasm.FindDelta(pDoc->m_uStartAddr, 0xFFF0, *pDoc->m_pCtx, dy);

        if (lines < 0)
            Refresh(); // Redraw the entire window
        else if (lines > 0)
        {
            if (lines >= dy)
                Refresh(); // Redraw the entire window
            else
            {
                //Refresh(); // Redraw the entire window
                ScrollWindow(0, -lines * m_nFontHeight, &rect, &rect);
            }
        }
        break;
    }

    case SB_THUMBPOSITION: // Scroll to the absolute position. The current position is provided in nPos
        break;
        
    case SB_THUMBTRACK:	// Drag scroll box to specified position. The current position is provided in nPos
    {
        wxRect rect = GetViewRect();
        int dy = CalcLineCount(rect); // Number of lines in the window

        int lines = deasm.FindDelta(pDoc->m_uStartAddr, uint16_t(nPos + 0x8000), *pDoc->m_pCtx, d); // 65816 fix

        if (lines < 0)
            Refresh(); // Redraw the entire window
        else if (lines > 0)
        {
            if (lines >= dy)
                Refresh(); // Redraw the entire window
            else
            {
                Refresh(); // Redraw the entire window
                //ScrollWindow(0, dir * lines *m_nFontHeight, &rect, &rect);
            }
        }
        break;
    }

    default:
        break;
    }

    SetScrollPos(SB_VERT, (int)pDoc->m_uStartAddr - 0x8000);
#endif
}

void CDeasm6502View::OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar)
{
    Scroll(nSBCode, nPos);

    //int SetScrollPos(int nBar, int nPos);

    //CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

bool CDeasm6502View::OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt) // 1.3.2 addes Mouse scroll wheel support
{
#if 0
    if (zDelta > 0)
        Scroll(SB_LINEUP, 0, 0);
    else
        Scroll(SB_LINEDOWN, 0, 0);
#endif

    return true;
}

void CDeasm6502View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
#if 0
    switch (nChar)
    {
    case VK_DOWN:
        Scroll(SB_LINEDOWN, 0, nRepCnt);
        break;

    case VK_UP:
        Scroll(SB_LINEUP, 0, nRepCnt);
        break;

    case VK_NEXT:
        Scroll(SB_PAGEDOWN, 0, nRepCnt);
        break;

    case VK_PRIOR:
        Scroll(SB_PAGEUP, 0, nRepCnt);
        break;

    case VK_HOME:
        Scroll(SB_TOP, 0, nRepCnt);
        break;

    case VK_END:
        Scroll(SB_BOTTOM, 0, nRepCnt);
        break;

    default:
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
    }
#endif
}

//=============================================================================

void CDeasm6502View::OnUpdate(wxView* pSender, LPARAM lHint, wxObject* pHint)
{
    int item = 0; //LOWORD(lHint);

    switch (item)
    {
    case 1:	// Redraw the indicators?
    {
        //wxRect rect = GetViewRect();
        //rect.right = m_nFontHeight; // The size of the left margin
        Refresh();
        //UpdateWindow();
        break;
    }

    case 2:	// Move the contents of the window?
        //ScrollToLine(uint16_t(HIWORD(lHint)));
        ScrollToLine(-1);
        break;

    case 0:	// Redraw the contents of the window?
        Refresh();
        break;
    }
}

//=============================================================================

afx_msg LRESULT CDeasm6502View::OnExitDebugger(WPARAM /* wParam */, LPARAM /* lParam */)
{
    GetDocument()->OnCloseDocument();
    return 0;
}

//-----------------------------------------------------------------------------

bool CDeasm6502View::OnEraseBkgnd(wxDC* dc)
{
    wxRect rect = GetViewRect();
    //dc->SetBrush(m_rgbBkgnd);
    dc->DrawRectangle(rect);
    return true;
}

//-----------------------------------------------------------------------------

void CDeasm6502View::OnDeasmGoto()
{
    static UINT addr = 0;

    CDeasmGoto dlg;
    dlg.m_uAddress = addr;

    if (dlg.ShowModal() == wxID_OK)
    {
        addr = dlg.m_uAddress;
        //scroll(SB_THUMBTRACK, dlg.m_uAddress - 0x8000, 1);
    }
}

void CDeasm6502View::OnUpdateDeasmGoto(CCmdUI* pCmdUI)
{
    //pCmdUI->Enable(true);
}

//-----------------------------------------------------------------------------
// Popup menu

void CDeasm6502View::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
#if 0
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_POPUP_DEASM6502));

    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);
    /*
      CWnd* pWndPopupOwner = this;
      while (pWndPopupOwner->GetStyle() & WS_CHILD)
        pWndPopupOwner = pWndPopupOwner->GetParent();
    */
    if (point.x == -1 && point.y == -1) // menu accessed via keyboard?
    {
        CRect rect = GetViewRect();

        point = rect.TopLeft();
        wxPoint ptTopLeft(0, 0);
        ClientToScreen(&ptTopLeft);
        point.x = ptTopLeft.x + rect.Width() / 2; // position ourselves in the middle of the window
        point.y = ptTopLeft.y + rect.Height() / 2;
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
//    pWndPopupOwner);
#endif
}


void CDeasm6502View::OnSize(UINT nType, int cx, int cy)
{
    //wxView::OnSize(nType, cx, cy);

    UpdateScrollRange();
}

void CDeasm6502View::UpdateScrollRange()  // 65816 fix
{
    wxRect rect = GetViewRect();
    int lines = CalcLineCount(rect);

#if 0
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = -0x8000;
    si.nMax = 0x7fff;
    si.nPage = lines; // estimate only

    SetScrollInfo(SB_VERT, &si, FALSE);
#endif
}
