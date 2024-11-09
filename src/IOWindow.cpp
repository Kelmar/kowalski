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
#include "encodings.h"

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
    , m_timer(this)
{
    // We're going to override the saved size anyhow with our own calculations.

    wxPersistentRegisterAndRestore(this, REG_ENTRY_IOWINDOW);

    m_showCursor = false;
    m_cursorBlinkState = false;

    // Allocates our initial memory block
    size_t sz = m_charCnt.x * m_charCnt.y;
    m_memory = new uint8_t[sz];
    memset(m_memory, 0, sz);

    // Remove ability for user to resize us with the mouse.
    int style = wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX);
    SetWindowStyle(style);

    SetBackgroundStyle(wxBG_STYLE_PAINT);

    InitEvents();

    m_timer.Start(250, false);
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

void CIOWindow::Invalidate(const wxPoint &location)
{
    wxSize cellSize = FontController::Get().getCellSize();

    // TODO: Redundant assertion? -- B.Simonds (Oct 20, 2024)
    ASSERT(cellSize.x > 0 && cellSize.y > 0);

    wxRect area(location, cellSize);
    RefreshRect(area);
}

/*************************************************************************/

void CIOWindow::Put(char chr, int x, int y)
{
    if (x > m_charCnt.x || y > m_charCnt.y || x < 0 || y < 0)
        return;

    m_memory[x + y * m_charCnt.x] = (uint8_t)chr;
    Invalidate(m_cursorPos);
}

/*************************************************************************/

void CIOWindow::PutChar(int chr)
{
    switch (chr)
    {
    case 0x0A: // Line Feed
        if (++m_cursorPos.y >= m_charCnt.y)
        {
            ASSERT(m_cursorPos.y == m_charCnt.y);
            m_cursorPos.y--;
            Scroll(1); // Shift the strings by one line
        }
        break;

    case 0x0D: // Carriage return
        m_cursorPos.x = 0;
        break;

    case 0x08: // Backspace
        if (--m_cursorPos.x < 0)
        {
            m_cursorPos.x = m_charCnt.x - 1;

            if (--m_cursorPos.y < 0)
            {
                m_cursorPos.y = 0;
                return;
            }
        }

        Put(' ', m_cursorPos.x, m_cursorPos.y);
        break;

    default:
        RawChar(chr);
        break;
    }
}

/*************************************************************************/

void CIOWindow::RawChar(int chr) // print character (verbatim)
{
    Put(chr, m_cursorPos.x, m_cursorPos.y);

    if (++m_cursorPos.x >= m_charCnt.x)
    {
        m_cursorPos.x = 0;

        if (++m_cursorPos.y >= m_charCnt.y)
        {
            ASSERT(m_cursorPos.y == m_charCnt.y);
            m_cursorPos.y--;
            Scroll(1);
        }
    }
}

/*************************************************************************/

void CIOWindow::Scroll(int dy)
{
    if (dy > m_charCnt.y || dy < 0)
        return;

    // Line offset
    memmove(m_memory, m_memory + dy * m_charCnt.y, (m_charCnt.y - dy) * m_charCnt.x);

    // To the uncovered zero position
    memset(m_memory + (m_charCnt.y - dy) * m_charCnt.x, 0, dy * m_charCnt.x);

    // Redraw entire window
    Refresh();
}

/*************************************************************************/

void CIOWindow::PutStr(const wxString &str)
{
    Freeze();

    // Ensure Thaw() is called when we exit.
    auto _ = defer([this] () { Thaw(); });

    for (size_t i = 0; i < str.Length(); ++i)
        PutChar(str[i]);
}

/*************************************************************************/

void CIOWindow::SetCursorPosition(const wxPoint &location)
{
    m_cursorPos.x = std::clamp(location.x, 0, m_charCnt.x - 1);
    m_cursorPos.y = std::clamp(location.y, 0, m_charCnt.y - 1);
}

/*************************************************************************/

void CIOWindow::Cls()
{
    memset(m_memory, 0, m_charCnt.y * m_charCnt.x);
    m_cursorPos.x = m_cursorPos.y = 0;
    Refresh(); // Redraw the entire window
}

/*************************************************************************/

void CIOWindow::Paste()
{
    if (wxTheClipboard->Open())
    {
        // Ensure clipboard is closed when we exit.
        auto _ = defer([this] () { wxTheClipboard->Close(); });

        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        m_InputBuffer.Paste(data.GetText());
    }
}

/*************************************************************************/

int CIOWindow::Input()
{
    m_showCursor = true;
    //SetFocus();

    return m_InputBuffer.GetChar(); // returns available char or 0 if buffer is empty
}

/*************************************************************************/

int CIOWindow::PeekInput() const
{
    return m_InputBuffer.PeekChar(); // Return next character waiting.
}

/*************************************************************************/
// CIOWindow message handlers
/*************************************************************************/

void CIOWindow::InitEvents()
{
    Bind(wxEVT_PAINT, &CIOWindow::OnPaint, this);
    Bind(wxEVT_CLOSE_WINDOW, &CIOWindow::OnClose, this);
    Bind(wxEVT_TIMER, &CIOWindow::BlinkCursor, this);
    Bind(wxEVT_KEY_DOWN, &CIOWindow::OnKeyDown, this);
    Bind(wxEVT_CHAR, &CIOWindow::OnChar, this);
    Bind(wxEVT_COMMAND_TEXT_PASTE, [this] (auto) { Paste(); });
}

/*************************************************************************/

void CIOWindow::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC dc(this);

    PrepareDC(dc);
    Draw(dc);
}

/*************************************************************************/

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

void CIOWindow::BlinkCursor(wxTimerEvent &)
{
    if (!m_showCursor)
        return; // Nothing to do

    m_cursorBlinkState = !m_cursorBlinkState;
    Invalidate(m_cursorPos); // Force redraw of the cursor
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
    Cls();

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

void CIOWindow::OnKeyDown(wxKeyEvent &e)
{
    if (e.GetKeyCode() == wxKeyCode::WXK_INSERT)
    {
        Paste();
        return;
    }

    e.Skip();
}

/*************************************************************************/

void CIOWindow::OnChar(wxKeyEvent &e)
{
    m_InputBuffer.PutChar(e.GetKeyCode());
}

/*************************************************************************/
/*************************************************************************/

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

/*************************************************************************/

char CInputBuffer::PeekChar() const
{
    char c = 0;

    if (m_pHead != m_pTail)
        c = *m_pTail;

    return c;
}

/*************************************************************************/

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

/*************************************************************************/

void CInputBuffer::Paste(const char *pcText)
{
    int nMax = std::min(strlen(pcText), BUF_SIZE);

    for (int i = 0; i < nMax; ++i)
        PutChar(pcText[i]);
}

/*************************************************************************/
