/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
 /*************************************************************************/

#include "StdAfx.h"
#include <wx/dcbuffer.h>

#include "encodings.h"
#include "FontController.h"
#include "HexView.h"

/*************************************************************************/

HexView::HexView()
    : wxScrolled()
    , m_memory(nullptr)
    , m_virtualSize(0, 0)
    , m_pageSize(0)
    , m_selStart(0)
    , m_selLen(0)
    , m_mouseTrack(false)
{
    Init();
}

HexView::HexView(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const wxString &name)
    : wxScrolled(parent, id, pos, size, wxVSCROLL, name)
    , m_memory(nullptr)
    , m_virtualSize(0, 0)
    , m_pageSize(0)
    , m_selStart(0)
    , m_selLen(0)
    , m_mouseTrack(false)
{
    Init();
}

HexView::~HexView()
{
}

/*************************************************************************/

void HexView::Init()
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxColor winColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    SetBackgroundColour(winColor);

    UpdateScrollInfo();

    Bind(wxEVT_PAINT, &HexView::OnPaint, this);

    Bind(wxEVT_LEFT_DOWN, &HexView::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &HexView::OnMouseUp, this);
    Bind(wxEVT_MOTION, &HexView::OnMouseMove, this);
}

/*************************************************************************/

uint32_t HexView::GetAddress() const
{
    int pos = GetScrollPos(wxVERTICAL);
    return pos * LINE_WIDTH;
}

/*************************************************************************/

void HexView::JumpTo(uint32_t address)
{
    if (!m_memory)
        return;

    if (address < m_memory->size())
    {
        int line = address / LINE_WIDTH;
        Scroll(-1, line);
    }
}

/*************************************************************************/

void HexView::CalculateScrollInfo()
{
    if (!m_memory)
        return;

    size_t sz = m_memory->size();

    if (sz == 0)
        m_virtualSize.Set(0, 0);

    // Compute the virtual height.
    int lines = sz / LINE_WIDTH;
    lines += ((sz % LINE_WIDTH) != 0 ? 1 : 0);

    if (m_pageSize > 0)
        lines += (sz / m_pageSize);

    // Compute the virtual width.

    // Compute the number of characters in a line
    wxString txt;

    int width = m_memory->size() - 1;
    width = txt.Printf("%X", width);

    width += 3; // Buffer between address and digits
    width += (3 * LINE_WIDTH); // Each hex digit
    width += 3; // Buffer between hex digits and characters
    width += LINE_WIDTH;

    wxSize cellSize = FontController::Get().getCellSize();

    m_virtualSize.Set(width * cellSize.x, lines * cellSize.y);
}

/*************************************************************************/

void HexView::UpdateScrollInfo()
{
    CalculateScrollInfo();
    CalcAddressFormat();

    wxSize cellSize = FontController::Get().getCellSize();

    SetScrollRate(0, cellSize.y);
    SetVirtualSize(m_virtualSize.GetWidth(), m_virtualSize.GetHeight());

    Refresh();
}

/*************************************************************************/

int HexView::CalcAddressChars() const
{
    wxString txt;

    if (!m_memory)
        return 0;

    // Compute the number of digits in the address
    int sz = m_memory->size() - 1;
    return txt.Printf("%X", sz);
}

/*************************************************************************/

void HexView::CalcAddressFormat()
{
    std::stringstream out;
    wxString txt;

    // Compute the number of digits in the address
    int sz = CalcAddressChars();

    // Build the format string
    out << "%0" << sz << "X ";
    m_addrFmt = out.str();
}

/*************************************************************************/

wxPoint HexView::GetHitCell(const wxPoint &p) const
{
    wxSize cellSize = FontController::Get().getCellSize();

    int row = p.y / cellSize.y;

    int leftMargin = CalcAddressChars() + 3;
    leftMargin *= cellSize.x;

    int rightMargin = leftMargin + LINE_WIDTH * 3 * cellSize.x;

    if ((p.x < leftMargin) || (p.x >= rightMargin))
        return wxPoint(-1, row); // Clicked in margin

    int col = ((p.x - leftMargin) / 3) / cellSize.x;

    return wxPoint(col, row);
}

/*************************************************************************/

void HexView::OnMouseDown(wxMouseEvent &e)
{
    auto where = e.GetPosition();
    where = this->CalcUnscrolledPosition(where);
    m_mouseDn = GetHitCell(where);

    if ((m_mouseDn.x < 0) || (m_mouseDn.y < 0))
        return;

    CaptureMouse();
    m_mouseTrack = true;
}

/*************************************************************************/

void HexView::UpdateSelection(wxMouseEvent &e)
{
    if (!m_mouseTrack)
        return;

    auto where = e.GetPosition();
    where = this->CalcUnscrolledPosition(where);
    where = GetHitCell(where);

    if ((where.x < 0) || (where.y < 0))
    {
        m_selStart = 0;
        m_selLen = 0;
    }
    else
    {
        int downPos = CalcAddressFromCell(m_mouseDn);;
        int upPos = CalcAddressFromCell(where);

        if (downPos > upPos)
        {
            m_selStart = upPos;
            m_selLen = (downPos - upPos) + 1;
        }
        else
        {
            m_selStart = downPos;
            m_selLen = (upPos - downPos) + 1;
        }
    }

    Refresh();
}

/*************************************************************************/

void HexView::OnMouseUp(wxMouseEvent &e)
{
    if (HasCapture())
        ReleaseMouse(); // Ensure mouse is always released.

    if ((m_mouseDn.x < 0) || (m_mouseDn.y < 0))
    {
        // Mouse down originated out of bounds.
        m_selStart = 0;
        m_selLen = 0;
        Refresh();
        return; 
    }

    UpdateSelection(e);
    m_mouseTrack = false;
}

/*************************************************************************/

void HexView::OnMouseMove(wxMouseEvent &e)
{
    UpdateSelection(e);
}

/*************************************************************************/

void HexView::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC dc(this);

    PrepareDC(dc);
    Draw(dc);
}

void HexView::Draw(wxDC &dc)
{
    wxDCTextColourChanger txtColor(dc);

    wxRect area = GetClientRect();

    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    FontController &fontController = FontController::Get();
    wxSize cellSize = fontController.getCellSize();
    dc.SetFont(fontController.getMonoFont());

    int start;
    GetViewStart(nullptr, &start); // Returns in logical units, not pixels.

    // Get the starting offset from within memory we should draw at.
    uint32_t addr = start * LINE_WIDTH;
    wxString charDisplay;
    wxString txt;

    // Figure out the virtual y offset to start drawing at.
    int y_off = start * fontController.getCellSize().y;

    wxCoord x = 0;
    wxCoord y = 0;

    wxColor fontClr = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOWTEXT);
    wxColor selClr = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_HIGHLIGHTTEXT);
    wxColor winBg = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOW);
    wxColor highBg = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_HIGHLIGHT);

    wxBrush winBrush(winBg);
    wxBrush highBrush(highBg);

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(winBrush);

    dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);

    const int CELL_WIDTH = cellSize.x * 3;
    const int HALF_CHAR = (cellSize.x / 2);

    if (!m_memory)
        return;

    // TODO: Allow map to other character sets.
    auto encoding = encodings::CodePage437::Get();

    for (int l = 0; y < area.GetHeight() && addr < m_memory->size(); ++l)
    {
        int w = txt.Printf(m_addrFmt, addr);

        dc.DrawText(txt, x, y + y_off);

        // +2 for line gap
        x += (w + 2) * cellSize.x;

        charDisplay = "";

        for (int i = 0; i < LINE_WIDTH && addr < m_memory->size(); ++i, ++addr)
        {
            uint8_t val = m_memory->get(addr);
            charDisplay += encoding.toUnicode(val);

            w = txt.Printf("%02X ", val);

            if (m_selLen > 0)
            {
                if ((addr >= m_selStart) && (addr < (m_selStart + m_selLen)))
                {
                    txtColor.Set(selClr);
                    dc.SetBrush(highBrush);
                }
                else
                {
                    txtColor.Set(fontClr);
                    dc.SetBrush(winBrush);
                }

                dc.DrawRectangle(x - HALF_CHAR, y + y_off, CELL_WIDTH, cellSize.y);
            }

            dc.DrawText(txt, x, y + y_off);
            x += w * cellSize.x;
        }

        // Add gap for line
        x += 2 * cellSize.x;

        txtColor.Set(fontClr);
        dc.SetBrush(winBrush);

        dc.DrawText(charDisplay, x, y + y_off);

        y += cellSize.y;
        x = 0;
    }

    // Draw divider lines

    wxColor frameClr = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWFRAME);
    dc.SetPen(wxPen(frameClr));

    // Find location of first line
    int w = txt.Printf(m_addrFmt, 0);
    x = (int)((w + 0.5) * cellSize.x);

    dc.DrawLine(x, y_off, x, area.GetHeight() + y_off);

    // Find location of second line
    x += (int)(cellSize.x * ((16 * 3) + 2));

    dc.DrawLine(x, y_off, x, area.GetHeight() + y_off);
}

/*************************************************************************/
