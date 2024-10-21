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

#include "StdAfx.h"
#include <wx/dcbuffer.h>

#include <memory.h>

#include "FontController.h"

#include "resource.h"
#include "IOWindow.h"

/*************************************************************************/

wxColour CIOWindow::m_rgbTextColor = wxColour(0, 0, 0);
wxColour CIOWindow::m_rgbBackgndColor = wxColour(255, 255, 255);

/*************************************************************************/

CIOWindow::CIOWindow(wxWindow *parent)
    : wxFrame(parent, wxID_ANY, _("IO Window"))
    , m_memory(nullptr)
    , m_charCnt(40, 25)
    , m_cursorPos()
{
    // We're going to override the saved size anyhow with our own calculations.

    wxPersistentRegisterAndRestore(this, REG_ENTRY_IOWINDOW);

    m_nCursorCount = 0;    // Counter hide cursor

    m_showCursor = false;
    m_bCursorVisible = false;

    m_uTimer = 0;

    // Allocates our initial memory block
    size_t sz = m_charCnt.x * m_charCnt.y;
    m_memory = new uint8_t[sz];
    memset(m_memory, 0, sz);

    // Remove ability for user to resize us with the mouse.
    int style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    SetWindowStyle(style);

    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &CIOWindow::OnPaint, this);
    Bind(wxEVT_CLOSE_WINDOW, &CIOWindow::OnClose, this);
}

CIOWindow::~CIOWindow()
{
    delete[] m_memory;
}

/*************************************************************************/

void CIOWindow::SetSize(int w, int h, bool resize/* = true*/)
{
    ASSERT(w > 0 && h > 0);

    int new_size = w * h;
    int old_size = m_charCnt.x * m_charCnt.y;
    bool change = m_charCnt.x != w || m_charCnt.y != h;

    m_charCnt.Set(w, h);

    if (m_memory == nullptr || new_size != old_size)
    {
        delete[] m_memory;
        m_memory = new uint8_t[new_size];
        memset(m_memory, 0, new_size);
    }

    // without changing window dimensions?
    if (!resize || !change)
        return;
}

/*************************************************************************/

int CIOWindow::put(char chr, int x, int y)
{
    if (x > m_charCnt.x || y > m_charCnt.y || x < 0 || y < 0)
        return -2;

    m_memory[x + y * m_charCnt.x] = (uint8_t)chr;
    Refresh();
    return 0;
}

/*************************************************************************/

void CIOWindow::Invalidate(int x, int y) // the area of ​​the character under (x,y) to redraw
{
    wxSize cellSize = FontController::Get().getCellSize();

    // TODO: Redundant assertion? -- B.Simonds (Oct 20, 2024)
    ASSERT(cellSize.x > 0 && cellSize.y > 0);

    wxRect area(x, y, cellSize.x, cellSize.y);
    RefreshRect(area);
}

/*************************************************************************/

int CIOWindow::scroll(int dy) // shift the strings by 'dy' lines
{
    if (dy > m_charCnt.y || dy < 0)
        return -2;

    // Line offset
    memmove(m_memory, m_memory + dy * m_charCnt.y, (m_charCnt.y - dy) * m_charCnt.x);

    // To the uncovered zero position
    memset(m_memory + (m_charCnt.y - dy) * m_charCnt.x, 0, dy * m_charCnt.x);

    // Redraw entire window
    Refresh();

    return 0;
}

/*************************************************************************/

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

/*************************************************************************/

int CIOWindow::PutC(int chr) // Print the character
{
    if (chr == 0x0a) // line feed?
    {
        if (++m_cursorPos.y >= m_charCnt.y)
        {
            ASSERT(m_cursorPos.y == m_charCnt.y);
            m_cursorPos.y--;
            scroll(1); // Shift the strings by one line
        }
    }
    else if (chr == 0x0d) // carriage return?
        m_cursorPos.x = 0;
    else if (chr == 0x08) // backspace?
    {
        if (--m_cursorPos.x < 0)
        {
            m_cursorPos.x = m_charCnt.x - 1;

            if (--m_cursorPos.y < 0)
            {
                m_cursorPos.y = 0;
                return 0;
            }
        }

        if (put(' ', m_cursorPos.x, m_cursorPos.y) < 0)
            return -1;

        Invalidate(m_cursorPos.x, m_cursorPos.y); // The area under the sign to be redrawn
    }
    else
        return PutChr(chr);

    return 0;
}

/*************************************************************************/

int CIOWindow::PutChr(int chr) // print character (verbatim)
{
    if (put(chr, m_cursorPos.x, m_cursorPos.y) < 0)
        return -1;

    Invalidate(m_cursorPos.x, m_cursorPos.y); // The area under the sign to be redrawn

    if (++m_cursorPos.x >= m_charCnt.x)
    {
        m_cursorPos.x = 0;

        if (++m_cursorPos.y >= m_charCnt.y)
        {
            ASSERT(m_cursorPos.y == m_charCnt.y);
            m_cursorPos.y--;
            scroll(1); // Shift the strings by one line
        }
    }

    return 0;
}

/*************************************************************************/

int CIOWindow::PutS(const char *str, int len/*= -1*/) // string of characters to print
{
    Freeze();

    // Ensure Thaw() is called when we exit.
    auto _ = defer([this] (...) { Thaw(); });

    for (int i = 0; i < len || len == -1; ++i)
    {
        if (str[i] == '\0')
            break;

        if (PutC(str[i]) < 0)
            return -1;
    }

    return 0;
}

/*************************************************************************/

bool CIOWindow::SetPosition(int x, int y) // Set the position for the text
{
    if (x > m_charCnt.x || y > m_charCnt.y || x < 0 || y < 0)
        return false;

    m_cursorPos.x = x;
    m_cursorPos.y = y;

    return true;
}

/*************************************************************************/

void CIOWindow::GetPosition(int &x, int &y) // read position
{
    x = m_cursorPos.x;
    y = m_cursorPos.y;
}

/*************************************************************************/

bool CIOWindow::Cls() // clear the window
{
    memset(m_memory, 0, m_charCnt.y * m_charCnt.x);
    m_cursorPos.x = m_cursorPos.y = 0;
    Refresh(); // Redraw the entire window

    return true;
}

/*************************************************************************/

int CIOWindow::Input() // input
{
    m_showCursor = true;
    //SetFocus();

    return m_InputBuffer.GetChar(); // returns available char or 0 if buffer is empty
}

/*************************************************************************/

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

/*************************************************************************/
// CIOWindow message handlers

void CIOWindow::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC dc(this);

    PrepareDC(dc);
    Draw(dc);
}

void CIOWindow::Draw(wxDC &dc)
{
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    if (m_memory == nullptr)
        return;

    FontController &fontController = FontController::Get();
    wxSize cellSize = fontController.getCellSize();
    dc.SetFont(fontController.getMonoFont());

    // TODO: Allow map to other character sets.
    encodings::Encoding &encoding = encodings::CodePage437::Get();
    wxString lineBuffer;

    lineBuffer.reserve(m_charCnt.x + 1);
    int offset = 0;

    for (int y = 0, pos_y = 0; y < m_charCnt.y; ++y, pos_y += cellSize.y)
    {
        lineBuffer.clear();

        for (int x = 0; x < m_charCnt.x; ++x, ++offset)
        {
            uint8_t c = m_memory[offset];
            lineBuffer += encoding.toUnicode(c == 0 ? ' ' : c);
        }

        dc.DrawText(lineBuffer, 0, pos_y);
    }

    DrawCursor(dc);
}

/*************************************************************************/

void CIOWindow::DrawCursor(wxDC &dc)
{
    if (!m_showCursor)
        return;

    wxSize cellSize = FontController::Get().getCellSize();

    wxPoint loc(m_cursorPos.x * cellSize.x, m_cursorPos.y * cellSize.y);
    wxRect area(loc, cellSize);

    // BUG: Doesn't seem to be working under Linux.... - B.Simonds (Oct 20, 2024)

    // Invert area for cursor.
    auto oldFunc = dc.GetLogicalFunction();
    dc.SetLogicalFunction(wxINVERT);
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.DrawRectangle(area);

    dc.SetLogicalFunction(oldFunc);
}

/*************************************************************************/

void CIOWindow::OnClose(wxCloseEvent &)
{
    // Don't destroy, just hide.
    Show(false);
}

/*************************************************************************/

void CIOWindow::SetColors(wxColour text, wxColour background)
{
    m_rgbBackgndColor = background;
    m_rgbTextColor = text;
    Refresh();
}

/*************************************************************************/

void CIOWindow::GetColors(wxColour &text, wxColour &background)
{
    text = m_rgbTextColor;
    background = m_rgbBackgndColor;
}

/*************************************************************************/

afx_msg LRESULT CIOWindow::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    VERIFY(Cls());

#if false

    if (!m_bHidden) // Was the window visible?
        Show();
#endif

    return 1;
}

/*************************************************************************/

afx_msg LRESULT CIOWindow::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
#if false
    if (IsShown())
    {
        m_bHidden = false; // info -the window was displayed
        Hide();
    }
    else
        m_bHidden = true; // info -the window was hidden
#endif

    return 1;
}

/*************************************************************************/

afx_msg LRESULT CIOWindow::OnCls(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    VERIFY(Cls());
    return 1;
}

/*************************************************************************/

afx_msg LRESULT CIOWindow::OnPutC(WPARAM wParam, LPARAM lParam)
{
    UNUSED(wParam);

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

/*************************************************************************/

afx_msg LRESULT CIOWindow::OnInput(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    return Input();
}

/*************************************************************************/

afx_msg LRESULT CIOWindow::OnPosition(WPARAM wParam, LPARAM lParam)
{
    bool bXPos = !!(wParam & 1);

    if (wParam & 2) // get pos?
    {
        return bXPos ? m_cursorPos.x : m_cursorPos.y;
    }
    else // set pos
    {
        int x = m_cursorPos.x;
        int y = m_cursorPos.y;

        if (bXPos)
            x = lParam;
        else
            y = lParam;

        if (x >= m_charCnt.x)
            x = m_charCnt.x - 1;

        if (y >= m_charCnt.y)
            y = m_charCnt.y - 1;

        if (x != m_cursorPos.x || y != m_cursorPos.y)
        {
            if (m_bCursorVisible && m_showCursor)
            {
                Refresh();
            }

            m_cursorPos.x = x;
            m_cursorPos.y = y;

            if (m_bCursorVisible && m_showCursor)
            {
                Refresh();
            }
        }
    }

    return 0;
}

/*************************************************************************/

void CIOWindow::OnTimer(UINT nIDEvent)
{
    UNUSED(nIDEvent);

    m_bCursorVisible = !m_bCursorVisible;

    Refresh();

    if (!m_bCursorVisible)
        m_showCursor = false;

    //  CMiniFrameWnd::OnTimer(nIDEvent);
}

/*************************************************************************/

void CIOWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    UNUSED(nChar);
    UNUSED(nRepCnt);
    UNUSED(nFlags);

    char c = (char)nChar;

    if (c)
        m_InputBuffer.PutChar(c);

    //  EndModalLoop(nChar);
}

/*************************************************************************/

bool CIOWindow::ContinueModal()
{
    if (wxGetApp().m_global.GetSimulator()->IsBroken()) // execution broken?
        return false;

    //return CMiniFrameWnd::ContinueModal();
    return true;
}

/*************************************************************************/

void CIOWindow::Paste()
{
#if 0
    if (!::IsClipboardFormatAvailable(CF_TEXT))
        return;

    if (!OpenClipboard())
        return;

    if (HANDLE hGlb = ::GetClipboardData(CF_TEXT))
    {
        if (VOID *pStr = ::GlobalLock(hGlb))
        {
            m_InputBuffer.Paste(reinterpret_cast<char *>(pStr));
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
    char *pNext = m_pHead + 1;

    if (pNext >= m_vchBuffer + BUF_SIZE)
        pNext = m_vchBuffer;

    if (pNext != m_pTail)	// is there a place in buffer?
    {
        *m_pHead = c;
        m_pHead = pNext;
    }
}

void CInputBuffer::Paste(const char *pcText)
{
    int nMax = std::min(strlen(pcText), BUF_SIZE);

    for (int i = 0; i < nMax; ++i)
        PutChar(pcText[i]);
}

void CIOWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    UNUSED(nChar);
    UNUSED(nRepCnt);
    UNUSED(nFlags);

#if 0
    if (nChar == VK_INSERT)
        Paste();
    else
        CMiniFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
#endif
}

#if 0
BOOL CIOWindow::PreTranslateMessage(MSG *pMsg)
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
void CIOWindow::OnContextMenu(CWnd *pWnd, CPoint point)
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
