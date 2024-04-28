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

// IOWindow.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "IOWindow.h"
#include <memory.h>

//-----------------------------------------------------------------------------

bool CIOWindow::m_bHidden;
wxFont CIOWindow::m_Font;


wxFontInfo CIOWindow::m_LogFont 
#if 0
=
{
    13,	// LONG lfHeight;
    0,	// LONG lfWidth;
    0,	// LONG lfEscapement;
    0,	// LONG lfOrientation;
    0,	// LONG lfWeight;
    0,	// BYTE lfItalic;
    0,	// BYTE lfUnderline;
    0,	// BYTE lfStrikeOut;
    0,	// BYTE lfCharSet;
    0,	// BYTE lfOutPrecision;
    0,	// BYTE lfClipPrecision;
    0,	// BYTE lfQuality;
    FIXED_PITCH,	// BYTE lfPitchAndFamily;
    "Courier"	// CHAR lfFaceName[LF_FACESIZE];
}
#endif
;

wxPoint CIOWindow::m_WndPos = wxPoint(0, 0); // window position
int CIOWindow::m_nInitW = 40;
int CIOWindow::m_nInitH = 25;
wxColour CIOWindow::m_rgbTextColor = wxColour(0, 0, 0);
wxColour CIOWindow::m_rgbBackgndColor = wxColour(255, 255, 255);

//-----------------------------------------------------------------------------
// Window class registration

/////////////////////////////////////////////////////////////////////////////
// CIOWindow

CIOWindow::CIOWindow()
    : m_pData(nullptr)
{
    m_nWidth = m_nHeight = 0;

    m_nPosX = m_nPosY = 0; // Location of the character to print (and cursor)
    m_nCursorCount = 0;    // Counter hide cursor

    m_bHidden = false;

    m_bCursorOn = false;
    m_bCursorVisible = false;

    m_uTimer = 0;
}

CIOWindow::~CIOWindow()
{
    delete[] m_pData;
}

//-----------------------------------------------------------------------------
// New window

bool CIOWindow::Create()
{
#if 0
    ASSERT(m_hWnd == 0);
    CString title;
    title.LoadString(IDS_IO_WINDOW);

    RECT rect= {0,0,100,100};
    rect.left = m_WndPos.x;
    rect.top = m_WndPos.y;
    if (!CMiniFrameWnd::CreateEx(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE, m_strClass, title,
                                 WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_MOVEFRAME /*| MFS_SYNCACTIVE*/,
                                 rect, AfxGetMainWnd(), 0))
        return FALSE;

    ModifyStyleEx(0,WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE);
//  SetFont(m_Font,FALSE);
//  CalcFontSize();
    SetSize(m_nInitW,m_nInitH);

    m_uTimer = SetTimer(101, 250, 0);

    SetFocus();

    return TRUE;
#endif

    return false;
}

void CIOWindow::CalcFontSize() // Calculate character size
{
#if 0
    ASSERT(m_hWnd);
    CClientDC dc(this);
    dc.SelectObject(&m_Font);
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    m_nCharH = (int)tm.tmHeight + (int)tm.tmExternalLeading;
    m_nCharW = (int)tm.tmAveCharWidth;
#endif
}

//-----------------------------------------------------------------------------

void CIOWindow::SetSize(int w, int h, int resize/* =1*/)
{
    ASSERT(w > 0 && h > 0);

    m_nInitW = w;
    m_nInitH = h;

    int new_size = w * h;
    int old_size = m_nWidth * m_nHeight;
    bool change =  m_nWidth != w || m_nHeight != h;

    m_nWidth = w;
    m_nHeight = h;

    if (m_pData == nullptr || new_size != old_size)
    {
        delete[] m_pData;
        m_pData = new uint8_t[new_size];
    }

    // without changing window dimensions?
    if (resize == 0 || !change) 
        return;

    Resize();
}

void CIOWindow::Resize()
{
    CalcFontSize();

    wxRect size(0, 0, m_nCharW * m_nWidth, m_nCharH * m_nHeight);

    //CalcWindowRect(&size, CWnd::adjustOutside);
    //SetWindowPos(NULL, 0, 0, size.Width(), size.Height(), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

    Cls();
}

//-----------------------------------------------------------------------------

int CIOWindow::put(char chr, int x, int y)
{
    if (m_pData == nullptr)
        return -1;

    if (x > m_nWidth || y > m_nHeight || x < 0 || y < 0)
        return -2;

    m_pData[x + y * m_nWidth] = (uint8_t)chr;
    return 0;
}

/*
int CIOWindow::puts(const char *str, int len, int x, int y)
{
  if (m_pData == nullptr)
    return -1;

  if (x > m_nWidth || y > m_nHeight || x < 0 || y < 0)
    return -2;

  return 0;
}
*/

int CIOWindow::invalidate(int x, int y) // the area of ​​the character under (x,y) to redraw
{
    wxRect rect;

    ASSERT(m_nCharH > 0 && m_nCharW > 0);

    rect.x = x * m_nCharW;
    rect.y = y * m_nCharH;
    rect.width = m_nCharW;
    rect.height = m_nCharH;

    RefreshRect(rect);

    return 0;
}

int CIOWindow::scroll(int dy) // shift the strings by 'dy' lines
{
    if (m_pData == nullptr)
        return -1;

    if (dy > m_nHeight || dy < 0)
        return -2;

    // Line offset
    memmove(m_pData, m_pData + dy * m_nWidth, (m_nHeight - dy) * m_nWidth);

    // To the uncovered zero position
    memset(m_pData + (m_nHeight - dy) * m_nWidth, 0, dy * m_nWidth);

    // Redraw entire window
    Refresh();

    return 0;
}

//-----------------------------------------------------------------------------
int CIOWindow::PutH(int chr) // Print hex number (8 bits)
{
    int h1 = (chr >> 4) & 0x0f;
    int h2 = chr & 0x0f;
    char szBuf[4];
    szBuf[0] = h1 > 9 ? h1 + 'A' - 10 : h1 + '0';
    szBuf[1] = h2 > 9 ? h2 + 'A' - 10 : h2 + '0';
    szBuf[2] = '\0';
    return PutS(szBuf);
}

int CIOWindow::PutC(int chr) // Print the character
{
    HideCursor();

    if (chr == 0x0a) // line feed?
    {
        if (++m_nPosY >= m_nHeight)
        {
            ASSERT(m_nPosY == m_nHeight);
            m_nPosY--;
            scroll(1); // Shift the strings by one line
        }
    }
    else if (chr == 0x0d) // carriage return?
        m_nPosX = 0;
    else if (chr == 0x08) // backspace?
    {
        if (--m_nPosX < 0)
        {
            m_nPosX = m_nWidth - 1;

            if (--m_nPosY < 0)
            {
                m_nPosY = 0;
                return 0;
            }
        }

        if (put(' ', m_nPosX, m_nPosY) < 0)
            return -1;

        invalidate(m_nPosX, m_nPosY); // The area under the sign to be redrawed
    }
    else
        return PutChr(chr);
        
    return 0;
}

int CIOWindow::PutChr(int chr) // print character (verbatim)
{
    HideCursor();

    if (put(chr, m_nPosX, m_nPosY) < 0)
        return -1;

    invalidate(m_nPosX, m_nPosY); // The area under the sign to be redrawed

    if (++m_nPosX >= m_nWidth)
    {
        m_nPosX = 0;

        if (++m_nPosY >= m_nHeight)
        {
            ASSERT(m_nPosY == m_nHeight);
            m_nPosY--;
            scroll(1); // Shift the strings by one line
        }
    }

    return 0;
}

int CIOWindow::PutS(const char *str, int len/*= -1*/) // string of characters to print
{
    for (int i = 0; i < len || len == -1; i++)
    {
        if (str[i] == '\0')
            break;

        if (PutC(str[i]) < 0)
            return -1;
    }
    
    return 0;
}

bool CIOWindow::SetPosition(int x, int y) // Set the position for the text
{
    if (x > m_nWidth || y > m_nHeight || x < 0 || y < 0)
        return false;

    m_nPosX = x;
    m_nPosY = y;
    
    return true;
}

void CIOWindow::GetPosition(int &x, int &y) // read position
{
    x = m_nPosX;
    y = m_nPosY;
}

bool CIOWindow::Cls() // clear the window
{
    if (m_pData == nullptr)
        return false;

    memset(m_pData, 0, m_nHeight * m_nWidth);
    m_nPosX = m_nPosY = 0;
    Refresh(); // Redraw the entire window

    return true;
}

int CIOWindow::Input() // input
{
    m_bCursorOn = true;
//	SetFocus();

    return m_InputBuffer.GetChar(); // returns available char or 0 if buffer is empty
}

/*
int CIOWindow::Input() // input
{
  if (theApp.m_global.GetSimulator()->IsBroken())	// execution broken?
    return -1;

  m_bCursorOn = true;
  m_bCursorVisible = true;
  m_uTimer = SetTimer(1, 250, 0);
  DrawCursor();

  SetFocus();

  RunModalLoop();

  KillTimer(m_uTimer);
  m_uTimer = 0;
  if (m_bCursorVisible)
  {
    m_bCursorVisible = false;
    DrawCursor();
  }
  m_bCursorOn = false;

  if (theApp.m_global.GetSimulator()->IsBroken())	// execution broken?
    AfxGetMainWnd()->SetFocus();

  return m_nModalResult;
}
*/

/////////////////////////////////////////////////////////////////////////////
// CIOWindow message handlers

void CIOWindow::OnPaint()
{
#if 0
    CPaintDC dc(this);	// device context for painting

    if (m_pData == nullptr)
        return;

    dc.SelectObject(&m_Font);
    dc.SetBkMode(OPAQUE);
    dc.SetTextColor(m_rgbTextColor);
    dc.SetBkColor(m_rgbBackgndColor);

    CString line;
    uint8_t *src = m_pData;
    for (int y = 0, pos_y = 0; y < m_nHeight; y++, pos_y += m_nCharH)
    {
        char *dst = line.GetBuffer(m_nWidth);

        for (int i = 0; i < m_nWidth; i++) // Single line characters to 'line' buffer
        {
            if ((*dst++ = (char)*src++) == 0)
                dst[-1] = ' '; // Replace '\0' with ' '
        }
        
        line.ReleaseBuffer(m_nWidth);
        dc.TextOut(0, pos_y, line); // print the line
    }

    DrawCursor();
#endif
}

//=============================================================================

void CIOWindow::OnDestroy()
{
    wxRect rect = GetScreenRect();

    m_WndPos = rect.GetTopLeft(); // Remember window position

#if 0
    if (m_uTimer)
        KillTimer(m_uTimer);

    m_uTimer = 0;
#endif

    //CMiniFrameWnd::OnDestroy();
}

//=============================================================================

#if 0
void CIOWindow::OnGetMinMaxInfo(MINMAXINFO* pMMI)
{
    CMiniFrameWnd::OnGetMinMaxInfo(pMMI);

    CRect size(0,0,m_nCharW*m_nWidth,m_nCharH*m_nHeight);
    CalcWindowRect(&size,CWnd::adjustOutside);
//TRACE("vert %d \thorz %d\n",!!(GetStyle() & WS_VSCROLL), !!(GetStyle() & WS_HSCROLL) );
    int w= size.Width();
    if (GetStyle() & WS_VSCROLL)
        w += ::GetSystemMetrics(SM_CXVSCROLL);
//  if (client.Width() < max_size.Width())	// jest suwak?
    int h= size.Height();
    if (GetStyle() & WS_HSCROLL)
        h += ::GetSystemMetrics(SM_CYHSCROLL);
//  if (client.Height() < max_size.Height())	// jest suwak?
    /*
      SCROLLINFO si;
      if (GetScrollInfo(SB_HORZ,&si,SIF_PAGE|SIF_RANGE) && si.nMax-si.nMin > (int)si.nPage)
        w += ::GetSystemMetrics(SM_CXVSCROLL);
      if (GetScrollInfo(SB_VERT,&si,SIF_PAGE|SIF_RANGE) && si.nMax-si.nMin > (int)si.nPage)
        h += ::GetSystemMetrics(SM_CYHSCROLL);
    */
    pMMI->ptMaxSize.x = w;
    pMMI->ptMaxSize.y = h;
    pMMI->ptMaxTrackSize.x = w;
    pMMI->ptMaxTrackSize.y = h;
//  pMMI->ptMinTrackSize.x = 1;
//  pMMI->ptMinTrackSize.y = 1;
//  pMMI->ptMaxPosition.x = 0;
//  pMMI->ptMaxPosition.y = 0;

//TRACE("mx %d \tmy %d \ttx %d \ty %d\n", pMMI->ptMaxSize.x, pMMI->ptMaxSize.y, pMMI->ptMaxTrackSize.x, pMMI->ptMaxTrackSize.y);

//  CMiniFrameWnd::OnGetMinMaxInfo(pMMI);
}
#endif

//=============================================================================

void CIOWindow::OnSize(UINT nType, int cx, int cy)
{
#if 0
    CMiniFrameWnd::OnSize(nType,cx,cy);

    if (nType == SIZE_RESTORED)
    {
        int w = (GetStyle() & WS_VSCROLL) ? ::GetSystemMetrics(SM_CXVSCROLL) : 0;
        int h = (GetStyle() & WS_HSCROLL) ? ::GetSystemMetrics(SM_CYHSCROLL) : 0;

        wxSize size = wxSize(m_nCharW * m_nWidth, m_nCharH * m_nHeight);

        bool remove = (cx + w >= size.cx && cy + h >= size.cy);

        SCROLLINFO si_horz =
        {
            sizeof si_horz,
            SIF_PAGE | SIF_RANGE,
            0, size.cx - 1, // min i max
            remove ? size.cx : cx,
            0, 0
        };

        SCROLLINFO si_vert=
        {
            sizeof si_vert,
            SIF_PAGE | SIF_RANGE,
            0, size.cy - 1, // min i max
            remove ? size.cy : cy,
            0, 0
        };

        SetScrollInfo(SB_HORZ, &si_horz);
        SetScrollInfo(SB_VERT, &si_vert);
    }
    else if (nType == SIZE_MAXIMIZED)
        cx = 0;
#endif
}

//-----------------------------------------------------------------------------

void CIOWindow::SetColors(wxColour text, wxColour backgnd)
{
    m_rgbBackgndColor = backgnd;
    m_rgbTextColor = text;
    Refresh();
}

void CIOWindow::GetColors(wxColour &text, wxColour &backgnd)
{
    text = m_rgbTextColor;
    backgnd = m_rgbBackgndColor;
}

//=============================================================================

afx_msg LRESULT CIOWindow::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    VERIFY(Cls());

    if (!m_bHidden) // Was the window visible?
        Show();

    return 1;
}


afx_msg LRESULT CIOWindow::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    if (IsShown())
    {
        m_bHidden = false; // info -the window was displayed
        Hide();
    }
    else
        m_bHidden = true; // info -the window was hidden
        
    return 1;
}

//=============================================================================

afx_msg LRESULT CIOWindow::OnCls(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    VERIFY(Cls());
    return 1;
}

afx_msg LRESULT CIOWindow::OnPutC(WPARAM wParam, LPARAM lParam)
{
    if (lParam == 0)
        VERIFY(PutC(int(uint8_t(wParam))) == 0);
    else if (lParam == 1)
        VERIFY(PutChr(int(uint8_t(wParam))) == 0);
    else if (lParam == 2)
        VERIFY(PutH(int(uint8_t(wParam))) == 0);
    else
        ASSERT(false);

    return 1;
}

afx_msg LRESULT CIOWindow::OnInput(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    return Input();
}

afx_msg LRESULT CIOWindow::OnPosition(WPARAM wParam, LPARAM lParam)
{
    bool bXPos = !!(wParam & 1);

    if (wParam & 2) // get pos?
    {
        return bXPos ? m_nPosX : m_nPosY;
    }
    else // set pos
    {
        int x= m_nPosX;
        int y= m_nPosY;

        if (bXPos)
            x = lParam;
        else
            y = lParam;

        if (x >= m_nWidth)
            x = m_nWidth - 1;

        if (y >= m_nHeight)
            y = m_nHeight - 1;

        if (x != m_nPosX || y != m_nPosY)
        {
            if (m_bCursorVisible && m_bCursorOn)
                DrawCursor(m_nPosX, m_nPosY, false);

            m_nPosX = x;
            m_nPosY = y;

            if (m_bCursorVisible && m_bCursorOn)
                DrawCursor(m_nPosX, m_nPosY, true);
        }
    }

    return 0;
}

//=============================================================================

void CIOWindow::HideCursor()
{
    if (m_bCursorVisible)
    {
        DrawCursor(m_nPosX, m_nPosY, false);
        m_bCursorVisible = false;
    }
    m_bCursorOn = false;
}

// draw cursor
//
void CIOWindow::DrawCursor()
{
    if (m_bCursorOn)
        DrawCursor(m_nPosX, m_nPosY, m_bCursorVisible);
}


void CIOWindow::DrawCursor(int nX, int nY, bool bVisible)
{
#if 0
    if (m_pData == nullptr)
        return;

    if (nX > m_nWidth || nY > m_nHeight || nX < 0 || nY < 0)
    {
        ASSERT(false);
        return;
    }

    // character under the cursor
    char szBuf[2]= { m_pData[nX + nY * m_nWidth], '\0' };
    if (szBuf[0] == '\0')
        szBuf[0] = ' ';

    CClientDC dc(this);

    dc.SelectObject(&m_Font);
    dc.SetBkMode(OPAQUE);

    if (bVisible)
    {
        dc.SetTextColor(m_rgbBackgndColor);
        dc.SetBkColor(m_rgbTextColor);
    }
    else
    {
        dc.SetTextColor(m_rgbTextColor);
        dc.SetBkColor(m_rgbBackgndColor);
    }

    // cursor pos & size
    CPoint ptPos(nX * m_nCharW, nY * m_nCharH);
    CRect rect(ptPos, CSize(m_nCharW, m_nCharH));

    dc.DrawText(szBuf, 1, rect, DT_TOP | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);
#endif
}

void CIOWindow::OnTimer(UINT nIDEvent)
{
    m_bCursorVisible = !m_bCursorVisible;

    DrawCursor();

    if (!m_bCursorVisible)
        m_bCursorOn = false;

//  CMiniFrameWnd::OnTimer(nIDEvent);
}

void CIOWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    char c = (char)nChar;

    if (c)
        m_InputBuffer.PutChar(c);

    //  EndModalLoop(nChar);
}

bool CIOWindow::ContinueModal()
{
    if (wxGetApp().m_global.GetSimulator()->IsBroken()) // execution broken?
        return false;

    //return CMiniFrameWnd::ContinueModal();
    return true;
}

void CIOWindow::OnClose()
{
    if (IsWaiting())
    {
        //EndModalLoop(-1); // break
        return;
    }

    Show();
//  CMiniFrameWnd::OnClose();
}

bool CIOWindow::IsWaiting() const
{
    return false;
//  return (m_nFlags & WF_MODALLOOP) != 0;	// in modal loop waiting for input?
}

void CIOWindow::ExitModalLoop()
{
#if 0
    if (IsWaiting())
        EndModalLoop(-1);
#endif
}

void CIOWindow::Paste()
{
#if 0
    if (!::IsClipboardFormatAvailable(CF_TEXT))
        return;

    if (!OpenClipboard())
        return;

    if (HANDLE hGlb = ::GetClipboardData(CF_TEXT))
    {
        if (VOID* pStr= ::GlobalLock(hGlb))
        {
            m_InputBuffer.Paste(reinterpret_cast<char*>(pStr));
            GlobalUnlock(hGlb);
        }
    }

    CloseClipboard();
#endif
}

///////////////////////////////////////////////////////////////////////////////

char CInputBuffer::GetChar() // get next available character (returns 0 if there are no chars)
{
    char c = 0;

    if (m_pHead != m_pTail)
    {
        c = *m_pTail++;

        if (m_pTail >= m_vchBuffer + BUF_SIZE)
            m_pTail = m_vchBuffer;
    }

    return c;
}

void CInputBuffer::PutChar(char c) // places char in the buffer (char is ignored if there is no space)
{
    char* pNext = m_pHead + 1;

    if (pNext >= m_vchBuffer + BUF_SIZE)
        pNext = m_vchBuffer;

    if (pNext != m_pTail)	// is there a place in buffer?
    {
        *m_pHead = c;
        m_pHead = pNext;
    }
}

void CInputBuffer::Paste(const char* pcText)
{
    int nMax = std::min(strlen(pcText), BUF_SIZE);

    for (int i = 0; i < nMax; ++i)
        PutChar(pcText[i]);
}

void CIOWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
#if 0
    if (nChar == VK_INSERT)
        Paste();
    else
        CMiniFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
#endif
}

#if 0
BOOL CIOWindow::PreTranslateMessage(MSG* pMsg)
{
    if (GetFocus() == this)
    {
        if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
        {
            if (pMsg->wParam >= VK_SPACE && pMsg->wParam <= 'Z')
            {
                if (::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_SHIFT) >= 0)
                {
                    // skip the rest of PreTranslateMessage() functions, cause they will
                    // eat some of those messages (as accel shortcuts); translate and
                    // dispatch them now

                    ::TranslateMessage(pMsg);
                    ::DispatchMessage(pMsg);
                    return true;
                }
            }
        }
    }

    return CMiniFrameWnd::PreTranslateMessage(pMsg);
}
#endif

#if 0
void CIOWindow::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CMenu menu;

    if (!menu.LoadMenu(IDR_POPUP_TERMINAL))
        return;

    CMenu *pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup);

    if (point.x == -1 && point.y == -1)		// menu wywo�ane przy pomocy klawiatury?
    {
        wxRect rect = GetClientRect();
        ClientToScreen(rect);
        point = rect.CenterPoint();
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}
#endif

void CIOWindow::OnPaste()
{
    Paste();
}
