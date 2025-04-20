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

#include "StdAfx.h"
#include "6502.h"

#include "InteractiveMargin.h"

/*=======================================================================*/

InteractiveMargin::InteractiveMargin()
    : wxWindow()
{
    Bind(wxEVT_PAINT, &InteractiveMargin::OnPaint, this);
}

/*=======================================================================*/

void InteractiveMargin::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

#if 0
    dc.SetFont(m_owner->GetFont());
    dc.SetBackground(wxBrush(metrics->colors.InteractiveMarginBackground));
    dc.Clear();

    int scrollUnits, origin;

    m_owner->GetViewStart(&origin, nullptr);
    m_owner->GetScrollPixelsPerUnit(&scrollUnits, nullptr);

    dc.SetDeviceOrigin(-origin * scrollUnits, 0);
#endif
}

/*=======================================================================*/

void InteractiveMargin::DrawBreakpoint(wxDC &dc, CAsm::Breakpoint breakpoint)
{
    UNUSED(dc);
    UNUSED(breakpoint);

#if 0
    if (breakpoint == CAsm::Breakpoint::BPT_NONE)
        return;

    bool isDisabled = (breakpoint & CAsm::Breakpoint::BPT_DISABLED) != 0;

    wxPen oldPen = dc.GetPen();
    wxBrush oldBrush = dc.GetBrush();

    dc.SetPen(wxPen(m_breakColor));
    dc.SetBrush(isDisabled ? *wxTRANSPARENT_BRUSH : wxBrush(m_breakColor));

    dc.DrawEllipse(0, m_drawLine, m_charSize.x, m_drawLine + m_charSize.y);

    dc.SetBrush(oldBrush);
    dc.SetPen(oldPen);
#endif
}

/*=======================================================================*/