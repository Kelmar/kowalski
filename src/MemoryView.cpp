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

// MemoryView.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "MemoryView.h"
#include "MemoryGoto.h"
#include "MemoryChg.h"

wxFont CMemoryView::m_Font;
wxFontInfo CMemoryView::m_LogFont;
wxColour CMemoryView::m_rgbTextColor;
wxColour CMemoryView::m_rgbBkgndColor;

/////////////////////////////////////////////////////////////////////////////
// CMemoryView

CMemoryView::CMemoryView()
{
    m_eDump = FULL;
}

CMemoryView::~CMemoryView()
{
}

#if REWRITE_TO_WX_WIDGET

BEGIN_MESSAGE_MAP(CMemoryView, CView)
    //{{AFX_MSG_MAP(CMemoryView)
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()
    ON_WM_KEYDOWN()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_CONTEXTMENU()
    ON_UPDATE_COMMAND_UI(ID_MEMORY_GOTO, OnUpdateMemoryGoto)
    ON_COMMAND(ID_MEMORY_GOTO, OnMemoryGoto)
    ON_UPDATE_COMMAND_UI(ID_MEMORY_CHG, OnUpdateMemoryChg)
    ON_COMMAND(ID_MEMORY_CHG, OnMemoryChg)
    ON_COMMAND(ID_MEMORY_FULL, OnMemoryFull)
    ON_COMMAND(ID_MEMORY_HEX, OnMemoryHex)
    ON_COMMAND(ID_MEMORY_TEXT, OnMemoryText)
    ON_UPDATE_COMMAND_UI(ID_MEMORY_FULL, OnUpdateMemoryFull)
    ON_UPDATE_COMMAND_UI(ID_MEMORY_HEX, OnUpdateMemoryHex)
    ON_UPDATE_COMMAND_UI(ID_MEMORY_TEXT, OnUpdateMemoryText)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

int CMemoryView::bytes_in_line()
{
    int lim = 0;

    switch (m_eDump)
    {
    case FULL: // hex and text
        lim = (m_nCx - 9) / 4; // the number of bytes displayed in one line
        break;

    case HEX: // Hex only
        lim = (m_nCx - 5) / 3; // the number of bytes displayed in one line
        break;

    case TEXT: // Text only
        lim = (m_nCx - 8) / 1; // the number of bytes displayed in one line
        break;

    default:
        ASSERT(false);
        break;
    }

    return lim <= 0 ? 1 : lim;
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryView drawing

void CMemoryView::OnDraw(wxDC *pDC)
{
    UNUSED(pDC);

#if REWRITE_TO_WX_WIDGET
    CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CMemoryDoc)));
    const COutputMem &mem = *pDoc->m_pMem;
    uint32_t addr = pDoc->m_uAddress;
    wxString line;
    char hex[8];

    int lim = bytes_in_line(); // the number of bytes displayed in one line

    for (int i = 0, y = 0; i <= m_nCy; i++, y += m_nChrH)
    {
        if (theApp.m_global.m_bProc6502 == 2)
            line.Printf("%06X ", int(addr));
        else
            line.Printf("%04X ", int(addr));

        int j = 0;
        switch (m_eDump)
        {
        case FULL:  // hex i tekst -----------------------------
        {
            for (j = 0; j < lim && (int)addr + j < max_mem; j++)
            {
                snprintf(hex, sizeof(hex), " %02X", int(mem[addr + j]));
                line += hex;
            }
            line += "  >";

            for (j = 0; j < lim && (int)addr + j < max_mem; j++)
                line += char(mem[addr + j]);

            line += _T("<");
        }
        break;

        case HEX:  // hex ---------------------------------
        {
            for (j = 0; j < lim && (int)addr + j < max_mem; j++)
            {
                snprintf(hex, sizeof(hex), " %02X", int(mem[addr + j]));
                line += hex;
            }
        }
        break;

        case TEXT: // tekst -------------------------------
        {
            line += " >";
            for (j = 0; j < lim && (int)addr + j < max_mem; j++)
                line += char(mem[addr + j]);
            line += "<";
        }
        break;
        }

        addr += uint32_t(j);
        pDC->TextOut(1, y, line);

        if (addr == 0) // Have we reached the end of memory?
            break;
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////
// obliczenia pomocnicze

void CMemoryView::calc(wxDC *pDC)
{
    UNUSED(pDC);

#if REWRITE_TO_WX_WIDGET
    wxRect rect = GetClientRect();

    pDC->SelectObject(&m_Font);
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    m_nChrH = (int)tm.tmHeight + (int)tm.tmExternalLeading;
    m_nChrW = tm.tmAveCharWidth;

    m_nCx = (rect.right - 1) / m_nChrW;	// ilo�� kolumn
    m_nCy = rect.bottom / m_nChrH;	// ilo�� wierszy
    //  if (rect.bottom % m_nCharH)	// na dole wystaje kawa�ek wiersza?
    if (m_nCy == 0)
        m_nCy++;
#endif
}

//-----------------------------------------------------------------------------

void CMemoryView::scroll(UINT nSBCode, int nPos, int nRepeat)
{
    UNUSED(nSBCode);
    UNUSED(nPos);
    UNUSED(nRepeat);

#if REWRITE_TO_WX_WIDGET
    CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();

    if (pDoc == NULL)
        return;

    switch (nSBCode)
    {
    case SB_ENDSCROLL: // End scroll
        break;

    case SB_LINEDOWN: // Scroll one line down
        switch (find_next_addr(pDoc->m_uAddress, *pDoc->m_pMem))
        {
        case 0:
            break; // Cannot scroll any further

        case 1:
            RECT rect;
            get_view_rect(rect);
            UpdateWindow();	// To avoid refreshing problems
            ScrollWindow(0, -m_nChrH, &rect, &rect);
            UpdateWindow();
            break;

        case 9999999: // 1.3.3 changed to be beyond scroll value from 999999
            Refresh();
            break;
        }
        break;

    case SB_LINEUP:	// Scroll one line up
        switch (find_prev_addr(pDoc->m_uAddress, *pDoc->m_pMem))
        {
        case 0:
            break; // Cannot scroll any further

        case 1:
            RECT rect;
            get_view_rect(rect);
            UpdateWindow();	// To avoid refreshing problems
            ScrollWindow(0, m_nChrH, &rect, &rect);
            UpdateWindow();
            break;

        case 9999999: // 1.3.3 changed to be beyond scroll value from 999999
            Refresh();
            break;

        default:
            ASSERT(false);
            break;
        }

        break;

    case SB_PAGEDOWN: // Scroll one page down
    {
        switch (find_next_addr(pDoc->m_uAddress, *pDoc->m_pMem, m_nCy))
        {
        case 0:
            break; // Cannot scroll any further

        default:
            Refresh();
            break;
        }
        break;
    }

    case SB_PAGEUP: // Scroll one page up
    {
        switch (find_prev_addr(pDoc->m_uAddress, *pDoc->m_pMem, m_nCy))
        {
        case 0:
            break; // Already at the beginning

        default:
            Refresh(); // przerysowanie ca�ego okna - zmieni�o si� kilka rozkaz�w
            break;
        }
        break;
    }

    case SB_TOP: // Scroll to top
    {
        wxRect rect = GetViewRect();

        int lines = find_delta(pDoc->m_uAddress, 0, *pDoc->m_pMem, m_nCy);

        if (lines == 9999999) // 1.3.3 changed to be beyond scroll value from 999999
            Refresh();
        else if (lines > 0)
        {
            if (lines >= m_nCy)
                Refresh();
            else
                ScrollWindow(0, lines * m_nChrH, &rect, &rect);
        }
        break;
    }

    case SB_BOTTOM:	// Scroll to bottom
    {
        wxRect rect = GetViewRect();

        int lines = find_delta(pDoc->m_uAddress, max_mem - 1, *pDoc->m_pMem, m_nCy);

        if (lines == 9999999) // 1.3.3 changed to be beyond scroll value from 999999
            Refresh();
        else if (lines < 0)
        {
            if (-lines >= m_nCy)
                Refresh();
            else
                ScrollWindow(0, lines * m_nChrH, &rect, &rect);
        }
        break;
    }

    case SB_THUMBPOSITION: // Scroll to the absolute position. The current position is provided in nPos
        break;

    case SB_THUMBTRACK: // Drag scroll box to specified position. The current position is provided in nPos
    {
        wxRect rect = GetViewRect();

        SCROLLINFO si; // 1.3.3 added si structure to get 32 bit resolution on the scrollbar
        ZeroMemory(&si, sizeof(si));

        si.cbSize = sizeof si;
        si.fMask = SIF_TRACKPOS;

        if (!(GetScrollInfo(SB_VERT, &si)))
            break;

        uint32_t pos = si.nTrackPos;

        if (nRepeat == 2)
            pos = nPos; // 1.3.3 for goto memory cmd use passed value

        pos -= pos % bytes_in_line();
        int lines = find_delta(pDoc->m_uAddress, pos, *pDoc->m_pMem, m_nCy);

        if (lines == 9999999) // 1.3.3 changed to be beyond scroll value from 999999
            Refresh();
        else if (lines)
        {
            if (abs(lines) >= m_nCy)
                Refresh();
            else
                ScrollWindow(0, lines * m_nChrH, &rect, &rect);
        }
        break;
    }

    // What message is this!?  -- B.Simonds (April 29, 2024)
    case 0x100 + SB_LINERIGHT:
    {
        switch (find_next_addr(pDoc->m_uAddress, *pDoc->m_pMem, 1, 1))
        {
        case 0:	// Already at the beginning
            break;

        default:
            Refresh();
            break;
        }
        break;
    }

    // What message is this!?  -- B.Simonds (April 29, 2024)
    case 0x100 + SB_LINELEFT:
    {
        switch (find_prev_addr(pDoc->m_uAddress, *pDoc->m_pMem, 1, 1))
        {
        case 0:	// Cannot scroll any further
            break;

        default:
            Refresh();
            break;
        }
        break;
    }

    default:
        break;
    }
    //  set_scroll_range();
    SetScrollPos(SB_VERT, (int)pDoc->m_uAddress /* / bytes_in_line() */);

#endif
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryView message handlers

#if REWRITE_TO_WX_WIDGET
void CMemoryView::OnPrepareDC(CDC *pDC, CPrintInfo *pInfo)
{
    if (pInfo)
        return;

    pDC->SetBkMode(OPAQUE);
    pDC->SelectObject(&m_Font);
    pDC->SetTextColor(m_rgbTextColor);
    pDC->SetBkColor(m_rgbBkgndColor);
    calc(pDC);

    CView::OnPrepareDC(pDC, pInfo);
}

BOOL CMemoryView::PreCreateWindow(CREATESTRUCT &cs)
{
    cs.style |= /*WS_CLIPSIBLINGS |*/ WS_VSCROLL;

    return CView::PreCreateWindow(cs);
}
#endif

void CMemoryView::OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar *pScrollBar)
{
    UNUSED(pScrollBar);

    scroll(nSBCode, nPos);

    // CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

bool CMemoryView::OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt) // 1.3.2 added mouse scroll wheel support
{
    UNUSED(nFlags);
    UNUSED(zDelta);
    UNUSED(pt);

#if REWRITE_TO_WX_WIDGET
    if (zDelta > 0)
        scroll(SB_LINEUP, 0, 0);
    else
        scroll(SB_LINEDOWN, 0, 0);
#endif

    return true;
}

void CMemoryView::OnInitialUpdate()
{
#if REWRITE_TO_WX_WIDGET
    CView::OnInitialUpdate();

    CClientDC dc(this);
    calc(&dc);
    set_scroll_range();
#endif
}

//-----------------------------------------------------------------------------

int CMemoryView::set_scroll_range()
{
#if REWRITE_TO_WX_WIDGET
    CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
    if (!pDoc)
        return -1;

    max_mem = 0x10000;

    if (theApp.m_global.m_bProc6502 == 2) // 65816
        max_mem = 0x1000000;

    int ret = 0;
    int scr = m_nCy * bytes_in_line();

    if (scr >= max_mem)
    {
        // The whole thing fits in the window
        SetScrollRange(SB_VERT, 0, 0);
        return -1;
    }

    SCROLLINFO si;
    memset(&si, 0, sizeof(SCROLLINFO));

    si.cbSize = sizeof(si);

    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS; // 65816 added SIF_POS
    si.nMin = 0;
    si.nMax = max_mem - 1;
    si.nPage = scr;

    int nPos = pDoc->m_uAddress - rng;
    si.nPos = nPos; // 65816 save position also

    SetScrollInfo(SB_VERT, &si, FALSE);
    scroll(SB_THUMBTRACK, nPos);

    return ret;
#endif

    return 0;
}

//-----------------------------------------------------------------------------

// Finding the address of a memory line preceding a given line
int CMemoryView::find_prev_addr(uint32_t &addr, const COutputMem &mem, int cnt/*= 1*/, int bytes/*= 0*/)
{
    ASSERT(cnt > 0);

    UNUSED(mem);

    if (cnt < 0)
        return 0;

    if (!bytes)
        bytes = bytes_in_line();

    int pos = addr - cnt * bytes;
    ASSERT(pos <= 0xFFFF);

    if (pos < 0)
    {
        if (addr % bytes)
            return addr = 0, 9999999; // Redraw whole window - 1.3.3 changed to be beyond scroll value from 999999

        cnt = addr / bytes;
        addr = (uint32_t)0;
    }
    else
        addr = (uint32_t)pos;

    return cnt; // This is how many rows can be moved
}

// Finding the address of a memory line following a given line
int CMemoryView::find_next_addr(uint32_t &addr, const COutputMem &mem, int cnt/*= 1*/, int bytes/*= 0*/)
{
    ASSERT(cnt > 0);

    UNUSED(mem);

    if (cnt < 0)
        return 0;

    if (!bytes)
        bytes = bytes_in_line(); // number of bytes in a line

    int scr = bytes * m_nCy; // and on the entire window
    int pos = addr + cnt * bytes;

    if (pos + scr > max_mem)
    {
        pos = max_mem - scr;

        if (pos < 0)
            pos = 0;

        cnt = pos - addr;

        if (cnt % bytes)
            cnt = 9999999; // Redraw whole window - 1.3.3 changed to be beyond scroll value from 999999
        else
            cnt /= bytes;
    }

    addr = (uint32_t)pos;
    return cnt;
}

// Check how many lines should the window content be moved to reach from 'addr' to 'dest'
int CMemoryView::find_delta(uint32_t &addr, uint32_t dest, const COutputMem &mem, int max_lines)
{
    UNUSED(mem);
    UNUSED(max_lines);

    if (dest == addr)
        return 0;

    int bytes = bytes_in_line();

    if (dest < addr)
    {
        int lines = (addr - dest);
        addr = dest;

        if (lines % bytes)
            return 9999999; // 1.3.3 changed to be beyond scroll value from 999999

        return lines / bytes;
    }
    else
    {
        int scr = bytes * m_nCy;

        if (scr >= max_mem)
        {
            if (addr)
            {
                addr = 0;
                return 9999999; // 1.3.3 changed to be beyond scroll value from 999999
            }
            else
                return 0;
        }

        if ((int)dest > max_mem - scr)
            dest = max_mem - scr;

        int lines = (dest - addr);
        addr = dest;

        if (lines == 0)
            return 0;

        if (lines % bytes)
            return 9999999; // 1.3.3 changed to be beyond scroll value from 999999

        return -(lines / bytes);
    }
}

//-----------------------------------------------------------------------------

void CMemoryView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    UNUSED(nChar);
    UNUSED(nRepCnt);
    UNUSED(nFlags);

#if REWRITE_TO_WX_WIDGET
    switch (nChar)
    {
    case VK_DOWN:
        scroll(SB_LINEDOWN, 0, nRepCnt);
        break;

    case VK_UP:
        scroll(SB_LINEUP, 0, nRepCnt);
        break;

    case VK_NEXT:
        scroll(SB_PAGEDOWN, 0, nRepCnt);
        break;

    case VK_PRIOR:
        scroll(SB_PAGEUP, 0, nRepCnt);
        break;

    case VK_HOME:
        scroll(SB_TOP, 0, nRepCnt);
        break;

    case VK_END:
        scroll(SB_BOTTOM, 0, nRepCnt);
        break;

    case VK_LEFT:
        scroll(0x100 + SB_LINELEFT, 0, nRepCnt);
        break;

    case VK_RIGHT:
        scroll(0x100 + SB_LINERIGHT, 0, nRepCnt);
        break;

    default:
        break;
        //      CView::OnKeyDown(nChar, nRepCnt, nFlags);
    }
#endif
}


void CMemoryView::OnSize(UINT nType, int cx, int cy)
{
    UNUSED(nType);
    UNUSED(cx);
    UNUSED(cy);

#if REWRITE_TO_WX_WIDGET
    CView::OnSize(nType, cx, cy);

    CClientDC dc(this);
    calc(&dc);
    set_scroll_range();
#endif
}

bool CMemoryView::OnEraseBkgnd(wxDC *pDC)
{
    UNUSED(pDC);

#if REWRITE_TO_WX_WIDGET
    wxRect rect = GetClientRect();
    pDC->FillSolidRect(rect, m_rgbBkgndColor);
#endif
    return true;
}

void CMemoryView::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

#if REWRITE_TO_WX_WIDGET
    CMenu menu;
    if (!menu.LoadMenu(IDR_POPUP_MEMORY))
        return;
    CMenu *pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    if (point.x == -1 && point.y == -1)     // menu wywo�ane przy pomocy klawiatury?
    {
        CRect rect;
        GetClientRect(rect);

        point = rect.TopLeft();
        CPoint ptTopLeft(0, 0);
        ClientToScreen(&ptTopLeft);
        point.x = ptTopLeft.x + rect.Width() / 2;   // ustawiamy si� na �rodku okna
        point.y = ptTopLeft.y + rect.Height() / 2;
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
#endif
}

//-----------------------------------------------------------------------------

void CMemoryView::OnUpdateMemoryGoto(CCmdUI *pCmdUI)
{
    UNUSED(pCmdUI);

#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

void CMemoryView::OnMemoryGoto()
{
    static uint32_t addr = 0; // TODO: Remove static variable
    CMemoryGoto dlg;
    dlg.m_uAddr = addr;

    if (dlg.ShowModal() == wxID_OK)
    {
        addr = dlg.m_uAddr;
#if REWRITE_TO_WX_WIDGET
        scroll(SB_THUMBTRACK, addr, 2);  //1.3.3 change ,1 to ,2 to allow passing nPos
#endif
}
}

void CMemoryView::OnUpdateMemoryChg(CCmdUI *pCmdUI)
{
    UNUSED(pCmdUI);

#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}



void CMemoryView::OnMemoryChg()
{
#if REWRITE_TO_WX_WIDGET
    CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CMemoryDoc)));

    CMemoryChg dlg(*pDoc->m_pMem, this);
    dlg.ShowModal();
    Refresh();
#endif
}

void CMemoryView::OnMemoryFull()
{
    m_eDump = FULL;
    Refresh();
}

void CMemoryView::OnMemoryHex()
{
    m_eDump = HEX;
    Refresh();
}

void CMemoryView::OnMemoryText()
{
    m_eDump = TEXT;
    Refresh();
}

void CMemoryView::OnUpdateMemoryFull(CCmdUI *pCmdUI)
{
    UNUSED(pCmdUI);
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
    pCmdUI->SetRadio(m_eDump == FULL);
#endif
}

void CMemoryView::OnUpdateMemoryHex(CCmdUI *pCmdUI)
{
    UNUSED(pCmdUI);
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
    pCmdUI->SetRadio(m_eDump == HEX);
#endif
}

void CMemoryView::OnUpdateMemoryText(CCmdUI *pCmdUI)
{
    UNUSED(pCmdUI);
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
    pCmdUI->SetRadio(m_eDump == TEXT);
#endif
}

#if REWRITE_TO_WX_WIDGET
void CMemoryView::OnUpdate(wxView *pSender, LPARAM lHint, wxObject *pHint)
{
    if (lHint == 'show')
    {
        CClientDC dc(this);
        calc(&dc);
        set_scroll_range();
    }
}
#endif
