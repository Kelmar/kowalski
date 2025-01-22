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

#include <sstream>

#include "BaseAddressRows.h"

/*=======================================================================*/

using namespace hex;

/*=======================================================================*/

BaseAddressRows::BaseAddressRows(HexEdit *parent)
    : wxWindow(parent, wxID_ANY)
    , SubControl(parent)
{
    Bind(wxEVT_PAINT, &BaseAddressRows::OnPaint, this);
}

/*=======================================================================*/

std::string BaseAddressRows::CalcAddressFormat()
{
    std::stringstream out;
    wxString txt;

    // Build the format string
    auto metrics = GetMetrics();
    out << "%0" << metrics->BaseAddressCharCount << "X ";
    return out.str();
}

/*=======================================================================*/

void BaseAddressRows::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

    auto metrics = GetMetrics();

    dc.SetBackground(wxBrush(metrics->colors.ControlBackground));
    dc.Clear();

    dc.SetFont(m_owner->GetFont());

    wxString addrFmt = CalcAddressFormat();

    auto mem = GetMemory();

    if (!mem)
        return;

    int scrollY = 0;
    int sizeY = GetRect().GetHeight();

    m_owner->CalcUnscrolledPosition(0, scrollY, nullptr, &scrollY);

    int y = metrics->cellMetrics.GapPx.y / 2;
    int y_off = metrics->LineSizePx.y;

    int lineStart = scrollY / y_off;

    uint32_t addr = lineStart * metrics->LineByteCount;
    size_t sz = mem->GetSize();

    wxString txt;

    while ((addr < sz) && (y < sizeY))
    {
        txt.Printf(addrFmt, addr);

        dc.DrawText(txt, 0, y);

        addr += metrics->LineByteCount;
        y += y_off;
    }
}

/*=======================================================================*/
