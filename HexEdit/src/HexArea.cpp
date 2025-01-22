/*=======================================================================*/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
 /*=======================================================================*/

#include "HexEdit.h"
#include "he_priv.h"

#include <ranges>

#include "BaseAddressRows.h"
#include "OffsetColumns.h"
#include "HexArea.h"

/*=======================================================================*/

using namespace hex;

#define HA_STYLE wxWANTS_CHARS

/*=======================================================================*/
/*=======================================================================*/

AreaDrawState::AreaDrawState(wxDC &drawContext, HexArea *self)
    : dc(drawContext)
    , metrics(self->GetMetrics())
    , mem(self->GetMemory())
{
    start = self->m_owner->CalcUnscrolledPosition(wxPoint(0, 0));

    int sizeY = self->GetRect().GetHeight();

    int lineCount = (int)std::ceil((float)sizeY / metrics->LineSizePx.y);

    int lineStart = start.y / metrics->LineSizePx.y;
    int addrCnt = (lineCount + 1) * metrics->LineByteCount;

    startAddress = lineStart * metrics->LineByteCount;
    endAddress = startAddress + addrCnt;

    startAddress = std::min(startAddress, (uint32_t)mem->GetSize());
    endAddress = std::min(endAddress, (uint32_t)mem->GetSize());

    startLine = self->m_selection.start / metrics->LineByteCount;
    endLine = endAddress / metrics->LineByteCount;
}

/*=======================================================================*/
/*=======================================================================*/

CellPainter::CellPainter(
    const AreaDrawState &drawState,
    const Selection &selection,
    uint32_t cell)
    : CellWorker(cell, drawState.mem, selection)
    , m_dc(drawState.dc)
    , m_drawState(drawState)
{
    m_line = cell / m_drawState.metrics->LineByteCount;
    m_column = cell % m_drawState.metrics->LineByteCount;

    m_sizePx = m_drawState.metrics->CellMetrics.SizePx;
    m_gapPx = m_drawState.metrics->CellMetrics.GapPx;
    m_fullPx = m_sizePx + m_gapPx;

    int top = m_line * m_fullPx.y;
    int left = m_column * m_fullPx.x;

    m_bounds = wxRect(left, top, m_fullPx.x, m_fullPx.y);
}

/*=======================================================================*/

void CellPainter::Draw()
{
    if (selected())
    {
        DrawBackground();
        DrawSelectionBounds();
    }

    DrawText();
}

/*=======================================================================*/

void CellPainter::DrawBackground()
{
    m_dc.SetPen(*wxTRANSPARENT_PEN);
    m_dc.SetBrush(wxBrush(m_drawState.metrics->Colors.HighlightBackground));

    m_dc.DrawRectangle(m_bounds);
}

/*=======================================================================*/

void CellPainter::DrawSelectionBounds()
{
    // Mostly works, but has some edge cases that need to be handled.

    bool aboveSelected = (m_line > 0) && selected(-m_drawState.metrics->LineByteCount);
    bool leftSelected = (m_column > 0) && selected(-1);

    bool belowSelected = (m_line < (m_drawState.metrics->TotalLineCount - 1)) && selected(m_drawState.metrics->LineByteCount);
    bool rightSelected = (m_column < (m_drawState.metrics->LineByteCount - 1)) && selected(1);

    if (!aboveSelected || !leftSelected || !belowSelected || !rightSelected)
    {
        m_dc.SetPen(wxPen(m_drawState.metrics->Colors.HighlightBorder, 1));

        int right = m_bounds.GetRight();
        int bottom = m_bounds.GetBottom();

        if (!aboveSelected)
            m_dc.DrawLine(m_bounds.x, m_bounds.y, right + 1, m_bounds.y);

        if (!leftSelected)
            m_dc.DrawLine(m_bounds.x, m_bounds.y, m_bounds.x, bottom + 1);

        if (!belowSelected)
            m_dc.DrawLine(m_bounds.x, bottom, right + 1, bottom);

        if (!rightSelected)
            m_dc.DrawLine(right, m_bounds.y, right, bottom + 1);
    }
}

/*=======================================================================*/

void CellPainter::DrawText()
{
    wxPen prevPen = m_dc.GetPen();

    wxDCTextColourChanger txtColor(m_dc);

    if (selected())
        txtColor.Set(m_drawState.metrics->Colors.SelectFontColor);
    else
        txtColor.Set(m_drawState.metrics->Colors.FontColor);

    wxString txt;
    txt.Printf("%02X", value());

    m_dc.DrawText(txt, m_bounds.GetTopLeft() + (m_gapPx / 2));
}

/*=======================================================================*/
/*=======================================================================*/

HexArea::HexArea(HexEdit *owner, BaseAddressRows *baseRows, OffsetColumns *offsetCols)
    : wxPanel(owner, wxID_ANY, wxDefaultPosition, wxDefaultSize, HA_STYLE)
    , SubControl(owner)
    , m_baseRows(baseRows)
    , m_offsetCols(offsetCols)
    , m_selection()
    , m_mouseOnChar(false)
    , m_mouseTrack(false)
    , m_mouseDn(0, 0)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxColor winColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    SetBackgroundColour(winColor);
}

/*=======================================================================*/

wxBEGIN_EVENT_TABLE(HexArea, wxWindow)
    EVT_PAINT(HexArea::OnPaint)
    EVT_LEFT_DOWN(HexArea::OnMouseDown)
    EVT_LEFT_UP(HexArea::OnMouseUp)
    EVT_MOTION(HexArea::OnMouseMove)
    EVT_MOUSE_CAPTURE_LOST(HexArea::OnCaptureLost)
    EVT_KEY_DOWN(HexArea::OnKeyDown)
wxEND_EVENT_TABLE()

/*=======================================================================*/

uint32_t HexArea::CalcAddressFromCell(wxPoint &p) const
{
    auto metrics = GetMetrics();
    return p.x + p.y * metrics->LineByteCount;
}

/*=======================================================================*/

bool HexArea::ToCell(wxPoint *where)
{
    *where = m_owner->CalcUnscrolledPosition(*where);
    auto metrics = GetMetrics();

    where->y /= metrics->LineSizePx.y;

    bool rval = where->x >= metrics->CharAreaStartX;

    if (rval)
    {
        where->x -= metrics->CharAreaStartX;
        where->x /= metrics->CharSizePx.x;
    }
    else
    {
        auto full = metrics->CellMetrics.SizePx + metrics->CellMetrics.GapPx;
        where->x /= full.x;
    }

    wxString tmp;
    tmp.Printf("{%d,%d}", where->x, where->y);
    wxLogDebug(tmp);

    return rval;
}

/*=======================================================================*/

void HexArea::UpdateSelection(wxMouseEvent &e)
{
    if (!m_mouseTrack)
        return;

    wxPoint where = e.GetPosition();
    ToCell(&where);

    if ((where.x < 0) || (where.y < 0))
        m_selection.clear();
    else
    {
        uint32_t downPos = CalcAddressFromCell(m_mouseDn);;
        uint32_t upPos = CalcAddressFromCell(where);

        if (downPos > upPos)
        {
            m_selection.start = upPos;
            m_selection.length = (downPos - upPos) + 1;
        }
        else
        {
            m_selection.start = downPos;
            m_selection.length = (upPos - downPos) + 1;
        }
    }

    Refresh();
}

/*=======================================================================*/

void HexArea::OnMouseDown(wxMouseEvent &e)
{
    SetFocus();

    m_mouseDn = e.GetPosition();
    m_mouseOnChar = ToCell(&m_mouseDn);

    CaptureMouse();
    m_mouseTrack = true;
}

/*=======================================================================*/

void HexArea::OnMouseUp(wxMouseEvent &e)
{
    if (HasCapture())
        ReleaseMouse(); // Ensure mouse is always released.

    if ((m_mouseDn.x < 0) || (m_mouseDn.y < 0))
    {
        // Mouse down originated out of bounds.
        m_selection.clear();
        Refresh();
        return;
    }

    UpdateSelection(e);
    m_mouseTrack = false;
}

/*=======================================================================*/

void HexArea::OnMouseMove(wxMouseEvent &e)
{
    UpdateSelection(e);
}

/*=======================================================================*/

void HexArea::OnCaptureLost(wxMouseCaptureLostEvent &)
{
    m_mouseTrack = false;
}

/*=======================================================================*/

void HexArea::OnKeyDown(wxKeyEvent &e)
{
    switch (e.GetKeyCode())
    {
    case WXK_ESCAPE:
        // If we don't have a selection, let the parent handle the key.
        if (m_selection.length > 0)
        {
            // We have a selection and focus, clear the selection.
            e.Skip();
            m_selection.clear();
            Refresh();
        }
        break;
    }
}

/*=======================================================================*/

void HexArea::ScrollWindow(int dx, int dy, const wxRect *rect)
{
    wxPanel::ScrollWindow(dx, dy, rect);

    m_baseRows->ScrollWindow(0, dy, rect);
    m_offsetCols->ScrollWindow(dx, 0, rect);
}

/*=======================================================================*/

#if 0

wxString HexArea::LineToHex(const std::span<const uint8_t> &bytes)
{
    wxString rval;
    wxString txt;

    rval.reserve(bytes.size() * 3);

    for (size_t i = 0; i < bytes.size(); ++i)
    {
        uint8_t val = bytes[i];
        txt.Printf("%02X ", val);
        rval += txt;
    }

    return rval.Trim();
}

/*=======================================================================*/

wxString HexArea::LineToChars(const std::span<const uint8_t> &bytes)
{
    wxString rval;

    rval.reserve(bytes.size());

    for (size_t i = 0; i < bytes.size(); ++i)
    {
        uint8_t val = bytes[i];
        rval += (val < 32) || (val > 126) ? '.' : (char)val;
    }

    return rval;
}
#endif

/*=======================================================================*/

void HexArea::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC dc(this);
    m_owner->PrepareDC(dc);

    AreaDrawState state(dc, this);

    dc.SetBackground(wxBrush(state.metrics->Colors.ControlBackground));
    dc.Clear();

    if (!state.mem)
        return;

    dc.SetFont(m_owner->GetFont());
    dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);

    wxPen prevPen = dc.GetPen();
    wxBrush prevBrush = dc.GetBrush();

    for (uint32_t i = state.startAddress; i < state.endAddress; ++i)
    {
        CellPainter painter(state, m_selection, i);
        painter.Draw();
    }

    dc.SetBrush(prevBrush);
    dc.SetPen(prevPen);

}

/*=======================================================================*/
