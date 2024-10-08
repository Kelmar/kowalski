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

#include "stdafx.h"
#include "resource.h"
#include "MemoryView.h"
#include "MemoryGoto.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFont CMemoryView::m_Font;
LOGFONT CMemoryView::m_LogFont;
COLORREF CMemoryView::m_rgbTextColor;
COLORREF CMemoryView::m_rgbBkgndColor;

/////////////////////////////////////////////////////////////////////////////
// CMemoryView

IMPLEMENT_DYNCREATE(CMemoryView, CView)

CMemoryView::CMemoryView()
{
  m_eDump = FULL;
}

CMemoryView::~CMemoryView()
{
}


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

int CMemoryView::bytes_in_line()
{
  int lim= 0;

  switch (m_eDump)
  {
  case FULL:  // hex i tekst
    lim = (m_nCx - 9) / 4;	// ilo�� wy�wietlanych w jednym wierszu bajt�w
    break;
  case HEX:   // tylko hex
    lim = (m_nCx - 5) / 3;	// ilo�� wy�wietlanych w jednym wierszu bajt�w
    break;
  case TEXT:  // tylko tekst
    lim = (m_nCx - 8) / 1;	// ilo�� wy�wietlanych w jednym wierszu bajt�w
    break;
  default:
    ASSERT(false);
  }
  return lim <= 0 ? 1 : lim;
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryView drawing

void CMemoryView::OnDraw(CDC* pDC)
{
  CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
  ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CMemoryDoc)));
  const COutputMem& mem= *pDoc->m_pMem;
  CString line(_T(' '), 1 + max(m_nCx, 8));
  UINT32 addr= pDoc->m_uAddress;
  TCHAR hex[8];
  int lim= bytes_in_line();	// ilo�� wy�wietlanych w jednym wierszu bajt�w

  for (int i= 0, y= 0; i <= m_nCy; i++, y += m_nChrH)
  {
    if (theApp.m_global.m_bProc6502==2)	
      line.Format(_T("%06X "), int(addr));
    else
      line.Format(_T("%04X "), int(addr));

    int j= 0;
    switch (m_eDump)
    {
    case FULL:  // hex i tekst -----------------------------
      {
        for (j = 0; j < lim && (int)addr + j < max_mem; j++)
        {
          wsprintf(hex, _T(" %02X"), int(mem[addr + j]));
          line += hex;
        }
        line += _T("  >");
        for (j = 0; j < lim && (int)addr + j < max_mem; j++)
          line += TCHAR(mem[addr + j]);
        line += _T("<");
      }
      break;
    case HEX:  // hex ---------------------------------
      {
        for (j = 0; j < lim && (int)addr + j < max_mem; j++)
        {
          wsprintf(hex, _T(" %02X"), int(mem[addr + j]));
          line += hex;
        }
      }
      break;
    case TEXT: // tekst -------------------------------
      {
        line += _T(" >");
        for (j = 0; j < lim && (int)addr + j < max_mem; j++)
          line += TCHAR(mem[addr + j]);
        line += _T("<");
      }
      break;
    }

    addr += UINT32(j);
    pDC->TextOut(1, y, line);
    if (addr == 0)	// doszli�my do ko�ca pami�ci?
      break;
  }

}

/////////////////////////////////////////////////////////////////////////////
// CMemoryView diagnostics

#ifdef _DEBUG
void CMemoryView::AssertValid() const
{
  CView::AssertValid();
}

void CMemoryView::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// obliczenia pomocnicze

void CMemoryView::calc(CDC *pDC)
{
  RECT rect;
  GetClientRect(&rect);

  pDC->SelectObject(&m_Font);
  TEXTMETRIC tm;
  pDC->GetTextMetrics(&tm);
  m_nChrH = (int)tm.tmHeight + (int)tm.tmExternalLeading;
  m_nChrW = tm.tmAveCharWidth;

  m_nCx = (rect.right-1) / m_nChrW;	// ilo�� kolumn
  m_nCy = rect.bottom / m_nChrH;	// ilo�� wierszy
//  if (rect.bottom % m_nCharH)	// na dole wystaje kawa�ek wiersza?
  if (m_nCy == 0)
    m_nCy++;
}

//-----------------------------------------------------------------------------

void CMemoryView::scroll(UINT nSBCode, int nPos, int nRepeat)
{
  CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
  if (pDoc == NULL)
    return;

  switch (nSBCode)
  {
    case SB_ENDSCROLL:	// End scroll
      break;

    case SB_LINEDOWN:	// Scroll one line down
      switch (find_next_addr(pDoc->m_uAddress,*pDoc->m_pMem))
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
	case 9999999:     // 1.3.3 changed to be beyond scroll value from 999999
	  InvalidateRect(NULL);	// przerysowanie ca�ego okna
	  break;
      }
      break;

    case SB_LINEUP:	// Scroll one line up
      switch (find_prev_addr(pDoc->m_uAddress,*pDoc->m_pMem))
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
	case 9999999:	// 1.3.3 changed to be beyond scroll value from 999999
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
      switch (find_next_addr(pDoc->m_uAddress,*pDoc->m_pMem,m_nCy))
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
      switch (find_prev_addr(pDoc->m_uAddress,*pDoc->m_pMem,m_nCy))
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
      int lines= find_delta(pDoc->m_uAddress,0,*pDoc->m_pMem,m_nCy);
      if (lines == 9999999)    // 1.3.3 changed to be beyond scroll value from 999999
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
      int lines= find_delta(pDoc->m_uAddress,max_mem-1,*pDoc->m_pMem,m_nCy);
      if (lines == 9999999)     // 1.3.3 changed to be beyond scroll value from 999999
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

      SCROLLINFO si;		// 1.3.3 added si structure to get 32 bit resolution on the scrollbar
      ZeroMemory(&si, sizeof(si));
      si.cbSize = sizeof si;
      si.fMask = SIF_TRACKPOS;
      if (!(GetScrollInfo(SB_VERT, &si)))
		break;

      UINT32 pos = (si.nTrackPos);
	  if (nRepeat==2) pos = (nPos); // 1.3.3 for goto memory cmd use passed value
      pos -= pos % bytes_in_line();
      int lines= find_delta(pDoc->m_uAddress, pos, *pDoc->m_pMem, m_nCy);
      if (lines == 9999999)     // 1.3.3 changed to be beyond scroll value from 999999
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
      switch (find_next_addr(pDoc->m_uAddress,*pDoc->m_pMem,1,1))
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
      switch (find_prev_addr(pDoc->m_uAddress,*pDoc->m_pMem,1,1))
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
  SetScrollPos(SB_VERT, ((int)pDoc->m_uAddress) /* / bytes_in_line() */ );
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryView message handlers

void CMemoryView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
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


BOOL CMemoryView::PreCreateWindow(CREATESTRUCT& cs) 
{
  cs.style |= /*WS_CLIPSIBLINGS |*/ WS_VSCROLL;

  return CView::PreCreateWindow(cs);
}


void CMemoryView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  scroll(nSBCode,nPos);

  // CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CMemoryView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) // 1.3.2 added mouse scroll wheel support
{
	if (zDelta > 0)
		scroll(SB_LINEUP,0,0);
	else	
		scroll(SB_LINEDOWN,0,0);
  return true; 	
}

void CMemoryView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CClientDC dc(this);
	calc(&dc);
	set_scroll_range();
}

//-----------------------------------------------------------------------------

int CMemoryView::set_scroll_range()
{
  CMemoryDoc *pDoc = (CMemoryDoc *)GetDocument();
  if (!pDoc)
    return -1;

  max_mem = 0x10000;
  if (theApp.m_global.m_bProc6502 == 2)  // 65816
	  max_mem = 0x1000000;


  int ret= 0;
  int scr= m_nCy * bytes_in_line();
  if (scr >= max_mem)
  {
    SetScrollRange(SB_VERT,0,0);	// ca�o�� mie�ci si� w oknie
    return -1;
  }
  
  SCROLLINFO si;
  si.cbSize = sizeof si;
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;   // 65816 added SIF_POS
  si.nMin = 0;
  si.nMax = max_mem-1;
  si.nPage = scr;
  int nPos= pDoc->m_uAddress;
  si.nPos = nPos;							// 65816 save position also
  SetScrollInfo(SB_VERT, &si, FALSE);
  scroll(SB_THUMBTRACK, nPos);
  return ret;
}

//-----------------------------------------------------------------------------

// odszukanie adresu wiersza pami�ci poprzedzaj�cego dany wiersz
int CMemoryView::find_prev_addr(UINT32 &addr, const COutputMem &mem, int cnt/*= 1*/, int bytes/*= 0*/)
{
  ASSERT(cnt > 0);
  if (cnt < 0)
    return 0;

  if (!bytes)
    bytes = bytes_in_line();
  int pos= addr - cnt * bytes;
  ASSERT(pos <= 0xFFFF);
  if (pos < 0)
  {
    if (addr % bytes)
      return addr=0, 9999999;	// trzeba przerysowa� ca�e okno - 1.3.3 changed to be beyond scroll value from 999999
    cnt = addr / bytes;
    addr = (UINT32)0;
  }
  else
    addr = (UINT32)pos;
  return cnt;			// o tyle wierszy mo�na przesun��
}


// odszukanie adresu wiersza pami�ci nast�puj�cego po danym wierszu
int CMemoryView::find_next_addr(UINT32 &addr, const COutputMem &mem,
				int cnt/*= 1*/, int bytes/*= 0*/)
{
  ASSERT(cnt > 0);
  if (cnt < 0)
    return 0;

  if (!bytes)
    bytes = bytes_in_line();		// ilo�� bajt�w w wierszu
  int scr= bytes * m_nCy;		// i na ca�ym oknie
  int pos= addr + cnt * bytes;
  if (pos+scr > max_mem)
  {
    pos = max_mem - scr;
    if (pos < 0)
      pos = 0;
    cnt = pos - addr;
    if (cnt % bytes)
      cnt = 9999999;			// trzeba przerysowa� ca�e okno - 1.3.3 changed to be beyond scroll value from 999999
    else
      cnt /= bytes;
  }
  addr = (UINT32)pos;
  return cnt;
}


// spr. o ile wierszy nale�y przesun�� zawarto�� okna aby dotrze� od 'addr' do 'dest'
int CMemoryView::find_delta(UINT32 &addr, UINT32 dest, const COutputMem &mem, int max_lines)
{
  if (dest == addr)
    return 0;

  int bytes= bytes_in_line();
  if (dest < addr)
  {
    int lines= (addr - dest);
    addr = dest;
    if (lines % bytes)
      return 9999999;   // 1.3.3 changed to be beyond scroll value from 999999
    return lines / bytes;
  }
  else
  {
    int scr= bytes * m_nCy;
    if (scr >= max_mem)
      if (addr)
      {
	addr = 0;
	return 9999999;   // 1.3.3 changed to be beyond scroll value from 999999
      }
      else
	return 0;
    if ((int)dest > max_mem-scr)
      dest = max_mem - scr;
    int lines= (dest - addr);
    addr = dest;
    if (lines == 0)
      return 0;
    if (lines % bytes)
      return 9999999;   // 1.3.3 changed to be beyond scroll value from 999999
    return -(lines / bytes);
  }
}

//-----------------------------------------------------------------------------

void CMemoryView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  switch (nChar)
  {
    case VK_DOWN:
      scroll(SB_LINEDOWN,0,nRepCnt);
      break;
    case VK_UP:
      scroll(SB_LINEUP,0,nRepCnt);
      break;
    case VK_NEXT:
      scroll(SB_PAGEDOWN,0,nRepCnt);
      break;
    case VK_PRIOR:
      scroll(SB_PAGEUP,0,nRepCnt);
      break;
    case VK_HOME:
      scroll(SB_TOP,0,nRepCnt);
      break;
    case VK_END:
      scroll(SB_BOTTOM,0,nRepCnt);
      break;
    case VK_LEFT:
      scroll(0x100+SB_LINELEFT,0,nRepCnt);
      break;
    case VK_RIGHT:
      scroll(0x100+SB_LINERIGHT,0,nRepCnt);
      break;
    default:
      break;
//      CView::OnKeyDown(nChar, nRepCnt, nFlags);
  }
}


void CMemoryView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CClientDC dc(this);
	calc(&dc);
	set_scroll_range();
}


BOOL CMemoryView::OnEraseBkgnd(CDC* pDC)
{
  CRect rect;
  GetClientRect(rect);
  pDC->FillSolidRect(rect,m_rgbBkgndColor);
  return TRUE;
//  return CView::OnEraseBkgnd(pDC);
}


void CMemoryView::OnContextMenu(CWnd* pWnd, CPoint point)
{
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
}

//-----------------------------------------------------------------------------

void CMemoryView::OnUpdateMemoryGoto(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(true);
}

void CMemoryView::OnMemoryGoto() 
{
  static UINT addr= 0;
  CMemoryGoto dlg;
  dlg.m_uAddr = addr;

  if (dlg.DoModal() == IDOK)
  {
    addr = dlg.m_uAddr;
    scroll(SB_THUMBTRACK,dlg.m_uAddr,2);  //1.3.3 change ,1 to ,2 to allow passing nPos
  }
}


void CMemoryView::OnUpdateMemoryChg(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(true);
}

#include "MemoryChg.h"

void CMemoryView::OnMemoryChg()
{
  CMemoryDoc *pDoc= (CMemoryDoc *)GetDocument();
  ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CMemoryDoc)));

  CMemoryChg dlg(*pDoc->m_pMem, this);
  dlg.DoModal();
  InvalidateRect(NULL);
}


void CMemoryView::OnMemoryFull()
{
  m_eDump = FULL;
  InvalidateRect(NULL);
}


void CMemoryView::OnMemoryHex()
{
  m_eDump = HEX;
  InvalidateRect(NULL);
}


void CMemoryView::OnMemoryText()
{
  m_eDump = TEXT;
  InvalidateRect(NULL);
}


void CMemoryView::OnUpdateMemoryFull(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(true);
  pCmdUI->SetRadio(m_eDump == FULL);
}

void CMemoryView::OnUpdateMemoryHex(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(true);
  pCmdUI->SetRadio(m_eDump == HEX);
}

void CMemoryView::OnUpdateMemoryText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetRadio(m_eDump == TEXT);
}


void CMemoryView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (lHint == 'show')
	{
		CClientDC dc(this);
		calc(&dc);
		set_scroll_range();
	}
}
