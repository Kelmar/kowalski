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

// StackView.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "StackView.h"
#include "MemoryGoto.h"

wxFont CStackView::m_Font;
wxFontInfo CStackView::m_LogFont;
wxColour CStackView::m_rgbTextColor;
wxColour CStackView::m_rgbBkgndColor;

/////////////////////////////////////////////////////////////////////////////
// CStackView

CStackView::CStackView()
{}

CStackView::~CStackView()
{}

#if REWRITE_TO_WX_WIDGET
BEGIN_MESSAGE_MAP(CStackView, CView)
    //{{AFX_MSG_MAP(CStackView)
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
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif

/////////////////////////////////////////////////////////////////////////////
// CStackView drawing

void CStackView::OnDraw(wxDC* pDC)
{
    UNUSED(pDC);

#if REWRITE_TO_WX_WIDGET
    CMemoryDoc* pDoc= (CMemoryDoc *)GetDocument();
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CMemoryDoc)));
    CString line(_T(' '), 1 + max(m_nCx, 8));
    uint16_t addr = wxGetApp().m_global.m_bSRef - pDoc->m_uAddress;
    TCHAR hex[8];
    int lim= bytes_in_line();	// ilo�� wy�wietlanych w jednym wierszu bajt�w
    COutputMem& mem= *pDoc->m_pMem;
    CMemoryDC dcMem(*pDC, this, m_rgbBkgndColor);
    dcMem.SelectObject(&m_Font);
    dcMem.SetTextColor(m_rgbTextColor);
    dcMem.SetBkColor(m_rgbBkgndColor);

    for (int i=0, y=0; i <= m_nCy; i++, y += m_nChrH)
    {
        int nLo= mem[addr];
        int nHi= mem[(addr + 1];
        int nPtr= nLo + (nHi << 8);
        ASSERT(nPtr >= 0 && nPtr <= 0xFFFF);
        if (i == 9)
            nPtr = pDoc->m_uStackPtr;
        line.Format(_T("%03X  %02X '%c'  %04X "), int(addr), nLo, nLo > 0 ? nLo : ' ', nPtr);
        int j;
        for (j= 0; j < lim; j++)
        {
            wsprintf(hex, _T(" %02X"), int(mem[(nPtr + j) & 0xFFFF]));
            line += hex;
        }

        if (addr <= pDoc->m_uStackPtr)
            dcMem.SetTextColor(RGB(192,192,192));

        --addr;
        dcMem.TextOut(1, y, line);
        if (addr < 0x100)	// bottom of stack?
            break;
    }

    dcMem.BitBlt();
#endif
}

/////////////////////////////////////////////////////////////////////////////
// auxiliary calculations

void CStackView::calc(wxDC *pDC)
{
    UNUSED(pDC);

#if REWRITE_TO_WX_WIDGET
    wxRect rect = GetClientRect());

    pDC->SelectObject(&m_Font);
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    m_nChrH = (int)tm.tmHeight + (int)tm.tmExternalLeading;
    m_nChrW = tm.tmAveCharWidth;

    m_nCx = (rect.right-1) / m_nChrW; // number of columns
    m_nCy = rect.bottom / m_nChrH; // number of lines
    //if (rect.bottom % m_nCharH) // Is there part sticking out below?
    if (m_nCy == 0)
        m_nCy++;
#endif
}

//-----------------------------------------------------------------------------

void CStackView::scroll(UINT nSBCode, int nPos, int nRepeat)
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
    case SB_ENDSCROLL:	// End scroll
        break;

    case SB_LINEDOWN:	// Scroll one line down
        switch (find_next_addr((uint16_t &)pDoc->m_uAddress,*pDoc->m_pMem))
        {
        case 0:
            break;	// dalej ju� si� nie da
        case 1:
            RECT rect;
            get_view_rect(rect);
            UpdateWindow();	// dla unikni�cia problem�w z od�wie�aniem
            ScrollWindow(0,-m_nChrH,&rect,&rect);
            UpdateWindow();
            break;
        case 999999:
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
            break;
        }
        break;

    case SB_LINEUP:	// Scroll one line up
        switch (find_prev_addr((uint16_t &)pDoc->m_uAddress,*pDoc->m_pMem))
        {
        case 0:
            break;	// jeste�my ju� na pocz�tku
        case 1:
            RECT rect;
            get_view_rect(rect);
            UpdateWindow();	// dla unikni�cia problem�w z od�wie�aniem
            ScrollWindow(0,m_nChrH,&rect,&rect);
            UpdateWindow();
            break;
        case 999999:
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
            break;
        default:
            ASSERT(FALSE);
            break;
        }
        break;

    case SB_PAGEDOWN:	// Scroll one page down
    {
        RECT rect;
        get_view_rect(rect);
        switch (find_next_addr((uint16_t &)pDoc->m_uAddress,*pDoc->m_pMem,m_nCy))
        {
        case 0:
            break;	// dalej ju� si� nie da
        default:
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
            break;
        }
        break;
    }

    case SB_PAGEUP:	// Scroll one page up
    {
        RECT rect;
        get_view_rect(rect);
        switch (find_prev_addr((uint16_t &)pDoc->m_uAddress,*pDoc->m_pMem,m_nCy))
        {
        case 0:
            break;	// jeste�my ju� na pocz�tku
        default:
            InvalidateRect(NULL);	// przerysowanie ca�ego okna - zmieni�o si� kilka rozkaz�w
            break;
        }
        break;
    }

    case SB_TOP:	// Scroll to top
    {
        RECT rect;
        get_view_rect(rect);
        int lines= find_delta((uint16_t &)pDoc->m_uAddress,0,*pDoc->m_pMem,m_nCy);
        if (lines == 999999)
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
        else if (lines > 0)
        {
            if (lines >= m_nCy)
                InvalidateRect(NULL);	// przerysowanie ca�ego okna
            else
                ScrollWindow(0,lines*m_nChrH,&rect,&rect);
        }
        break;
    }

    case SB_BOTTOM:	// Scroll to bottom
    {
        RECT rect;
        get_view_rect(rect);
        int lines= find_delta((uint16_t &)pDoc->m_uAddress,0xFFFF,*pDoc->m_pMem,m_nCy);
        if (lines == 999999)
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
        else if (lines < 0)
        {
            if (-lines >= m_nCy)
                InvalidateRect(NULL);	// przerysowanie ca�ego okna
            else
                ScrollWindow(0,lines*m_nChrH,&rect,&rect);
        }
        break;
    }

    case SB_THUMBPOSITION:   // Scroll to the absolute position. The current position is provided in nPos
        break;
    case SB_THUMBTRACK:	// Drag scroll box to specified position. The current position is provided in nPos
    {
        RECT rect;
        get_view_rect(rect);
        uint16_t pos= (nPos); //+0x8000);
//      pos -= pos % bytes_in_line();
        int lines= find_delta((uint16_t &)pDoc->m_uAddress,pos,*pDoc->m_pMem,m_nCy);
        if (lines == 999999)
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
        else if (lines)
        {
            if (abs(lines) >= m_nCy)
                InvalidateRect(NULL);	// przerysowanie ca�ego okna
            else
                ScrollWindow(0,lines*m_nChrH,&rect,&rect);
        }
        break;
    }

    case 0x100+SB_LINERIGHT:
    {
        switch (find_next_addr((uint16_t &)pDoc->m_uAddress,*pDoc->m_pMem,1,1))
        {
        case 0:			// ju� nie ma przesuni�cia
            break;
        default:
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
            break;
        }
        break;
    }
    case 0x100+SB_LINELEFT:
    {
        switch (find_prev_addr((uint16_t &)pDoc->m_uAddress,*pDoc->m_pMem,1,1))
        {
        case 0:			// ju� nie ma przesuni�cia
            break;
        default:
            InvalidateRect(NULL);	// przerysowanie ca�ego okna
            break;
        }
        break;
    }
    default:
        break;
    }
//  set_scroll_range();
    if (nSBCode != SB_ENDSCROLL)
        SetScrollPos(SB_VERT,((int)pDoc->m_uAddress /*- 0x8000*/) /* / bytes_in_line() */ );
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CStackView message handlers

#if REWRITE_TO_WX_WIDGET
void CStackView::OnPrepareDC(wxDC* pDC, CPrintInfo* pInfo)
{
    if (pInfo)
        return;
    pDC->SetBkMode(OPAQUE);
    pDC->SelectObject(&m_Font);
    calc(pDC);

    CView::OnPrepareDC(pDC, pInfo);
}

BOOL CStackView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= /*WS_CLIPSIBLINGS |*/ WS_VSCROLL;

    return CView::PreCreateWindow(cs);
}
#endif

void CStackView::OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar)
{
    UNUSED(nSBCode);
    UNUSED(nPos);
    UNUSED(pScrollBar);

#if REWRITE_TO_WX_WIDGET
    scroll(nSBCode, nPos);
#endif
    //CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

bool CStackView::OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt) // 1.3.2 added mouse scroll wheel support
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

void CStackView::OnInitialUpdate()
{
#if REWRITE_TO_WX_WIDGET
    CView::OnInitialUpdate();

    CMemoryDoc* pDoc= (CMemoryDoc *)GetDocument();
    pDoc->m_uAddress = 0x00;

    CClientDC dc(this);
    calc(&dc);
    set_scroll_range();
#endif
}

//-----------------------------------------------------------------------------

int CStackView::set_scroll_range()
{
#if REWRITE_TO_WX_WIDGET
    CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
    if (!pDoc)
        return -1;

    int ret= 0;
    int scr= m_nCy; // * bytes_in_line();

    if (scr >= 0x100)
    {
        // the whole thing fits in the window
        SetScrollRange(SB_VERT, 0, 0);
        return -1;
    }

    int rng= 0x100;
    SCROLLINFO si;
    si.cbSize = sizeof si;
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = rng;
    si.nPage = scr;
    SetScrollInfo(SB_VERT, &si, FALSE);
    int nPos= pDoc->m_uAddress;
    if (nPos > rng - scr)
        nPos = rng - scr;
    scroll(SB_THUMBTRACK, nPos);
    return ret;
#endif

    return 0;
}

//-----------------------------------------------------------------------------

// odszukanie adresu wiersza pami�ci poprzedzaj�cego dany wiersz
int CStackView::find_prev_addr(uint16_t &addr, const COutputMem &mem, int cnt/*= 1*/, int bytes/*= 0*/)
{
    UNUSED(addr);
    UNUSED(mem);
    UNUSED(cnt);
    UNUSED(bytes);

    return 0;

#if REWRITE_TO_WX_WIDGET
    ASSERT(cnt > 0);
    if (cnt < 0)
        return 0;

    if (!bytes)
        bytes = 1; //bytes_in_line();
    int pos= addr - cnt * bytes;
    ASSERT(pos <= 0xFFFF);
    if (pos < 0)
    {
        if (addr % bytes)
            return addr=0, 999999;	// trzeba przerysowa� ca�e okno
        cnt = addr / bytes;
        addr = (uint16_t)0;
    }
    else
        addr = (uint16_t)pos;
    return cnt;			// o tyle wierszy mo�na przesun��
#endif
}


// odszukanie adresu wiersza pami�ci nast�puj�cego po danym wierszu
int CStackView::find_next_addr(uint16_t &addr, const COutputMem &mem,
                               int cnt/*= 1*/, int bytes/*= 0*/)
{
    UNUSED(addr);
    UNUSED(mem);
    UNUSED(cnt);
    UNUSED(bytes);

    return 0;

#if REWRITE_TO_WX_WIDGET
    ASSERT(cnt > 0);
    if (cnt < 0)
        return 0;

    if (!bytes)
        bytes = 1; //bytes_in_line();		// ilo�� bajt�w w wierszu
    int scr= bytes * m_nCy;		// i na ca�ym oknie
    int pos= addr + cnt * bytes;
    if (pos+scr > 0x100)
    {
        pos = 0x100 - scr;
        if (pos < 0)
            pos = 0;
        cnt = pos - addr;
        if (cnt % bytes)
            cnt = 999999;			// trzeba przerysowa� ca�e okno
        else
            cnt /= bytes;
    }
    addr = (uint16_t)pos;
    return cnt;
#endif
}


// spr. o ile wierszy nale�y przesun�� zawarto�� okna aby dotrze� od 'addr' do 'dest'
int CStackView::find_delta(uint16_t &addr, uint16_t dest, const COutputMem &mem, int max_lines)
{
    UNUSED(addr);
    UNUSED(dest);
    UNUSED(mem);
    UNUSED(max_lines);

    return 0;

#if REWRITE_TO_WX_WIDGET
    if (dest == addr)
        return 0;

    int bytes= 1; //bytes_in_line();
    if (dest < addr)
    {
        int lines= (addr - dest);
        addr = dest;
        if (lines % bytes)
            return 999999;
        return lines / bytes;
    }
    else
    {
        int scr= bytes * m_nCy;
        if (scr >= 0x100)
            if (addr)
            {
                addr = 0;
                return 999999;
            }
            else
                return 0;
        if (dest > 0x100-scr)
            dest = 0x100 - scr;
        int lines= (dest - addr);
        addr = dest;
        if (lines == 0)
            return 0;
        if (lines % bytes)
            return 999999;
        return -(lines / bytes);
    }
#endif
}

//-----------------------------------------------------------------------------

void CStackView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
        scroll(0x100+SB_LINELEFT, 0, nRepCnt);
        break;
    case VK_RIGHT:
        scroll(0x100+SB_LINERIGHT, 0, nRepCnt);
        break;
    default:
        break;
//      CView::OnKeyDown(nChar, nRepCnt, nFlags);
    }
#endif
}

void CStackView::OnSize(UINT nType, int cx, int cy)
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

bool CStackView::OnEraseBkgnd(wxDC* pDC)
{
    UNUSED(pDC);

//	CRect rect;
//	GetClientRect(rect);
//	pDC->FillSolidRect(rect, m_rgbBkgndColor);
    return true;
}

void CStackView::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

#if REWRITE_TO_WX_WIDGET
    CMenu menu;
    if (!menu.LoadMenu(IDR_POPUP_ZPMEMORY))
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

void CStackView::OnUpdateMemoryGoto(CCmdUI* pCmdUI)
{
    UNUSED(pCmdUI);

#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

void CStackView::OnMemoryGoto()
{
#if REWRITE_TO_WX_WIDGET
    static UINT addr= 0;
    CMemoryGoto dlg;
    dlg.m_uAddr = addr;

    if (dlg.DoModal() == IDOK)
    {
        addr = dlg.m_uAddr;
        scroll(SB_THUMBTRACK, 0x1ff - dlg.m_uAddr, 1);
    }
#endif
}

void CStackView::OnUpdateMemoryChg(CCmdUI* pCmdUI)
{
    UNUSED(pCmdUI);

#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

#include "MemoryChg.h"

void CStackView::OnMemoryChg()
{
#if REWRITE_TO_WX_WIDGET
    CMemoryDoc *pDoc= (CMemoryDoc *)GetDocument();
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CMemoryDoc)));

    CMemoryChg dlg(*pDoc->m_pMem, this);
    dlg.DoModal();
    InvalidateRect(NULL);
#endif
}

void CStackView::OnUpdate(wxView* pSender, LPARAM lHint, wxObject* pHint)
{
    UNUSED(pSender);
    UNUSED(lHint);
    UNUSED(pHint);

#if REWRITE_TO_WX_WIDGET
    switch (lHint)
    {
    case 'show':
    {
        CClientDC dc(this);
        calc(&dc);
        set_scroll_range();
    }
    break;

    case 'invl':
        Invalidate(false);
        break;
    }
#endif
}
