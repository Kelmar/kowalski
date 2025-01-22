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

#include "OffsetColumns.h"

/*=======================================================================*/

using namespace hex;

/*=======================================================================*/

OffsetColumns::OffsetColumns(HexEdit *parent)
    : wxWindow(parent, wxID_ANY)
    , SubControl(parent)
{
    Bind(wxEVT_PAINT, &OffsetColumns::OnPaint, this);
}

/*=======================================================================*/

void OffsetColumns::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

    auto metrics = GetMetrics();

    dc.SetFont(m_owner->GetFont());
    dc.SetBackground(wxBrush(metrics->colors.ControlBackground));
    dc.Clear();

    int scrollUnits, origin;

    m_owner->GetViewStart(&origin, nullptr);
    m_owner->GetScrollPixelsPerUnit(&scrollUnits, nullptr);

    dc.SetDeviceOrigin(-origin * scrollUnits, 0);

    wxString txt;
    int x = metrics->cellMetrics.GapPx.x / 2;

    for (int i = 0; i < metrics->LineByteCount; ++i)
    {
        txt.Printf("%02X ", i);
        dc.DrawText(txt, x, 0);

        x += metrics->cellMetrics.SizePx.x + metrics->cellMetrics.GapPx.x;
    }
}

/*=======================================================================*/
