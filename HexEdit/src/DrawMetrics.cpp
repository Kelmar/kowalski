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

#include <wx/settings.h>

/*=======================================================================*/

// TODO: Replace this with a variable
// Values per line
#define LINE_WIDTH 16

using namespace hex;

/*=======================================================================*/

Colors::Colors()
{
    // TODO: Theme color selection.

    FontColor = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOWTEXT);
    SelectFontColor = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_HIGHLIGHTTEXT);

    WindowBackground = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOW);
    ControlBackground = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_3DFACE);

    wxColour bg = wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_HIGHLIGHT);

    HighlightBorder = bg.ChangeLightness(50);
    HighlightBackground = bg;
}

/*=======================================================================*/
/*=======================================================================*/

DrawMetrics::DrawMetrics(const HexEdit *owner)
    : m_owner(owner)
    , colors()
{
    GetBaseItems();
    CalcCellMetrics();
    CalcSizes();
}

/*=======================================================================*/

void DrawMetrics::GetBaseItems()
{
    wxMemoryDC dc;
    dc.SetFont(m_owner->GetFont());

    CharSizePx = dc.GetTextExtent("M");

    // These are hard coded for now
    LineByteCount = LINE_WIDTH;
}

/*=======================================================================*/

void DrawMetrics::CalcCellMetrics()
{
    cellMetrics.CharacterCount = 2; // Hard coded for now
    cellMetrics.SizePx = wxSize(cellMetrics.CharacterCount * CharSizePx.x, CharSizePx.y);
    cellMetrics.GapPx = wxSize(CharSizePx.x, 0);
}

/*=======================================================================*/

void DrawMetrics::CalcSizes()
{
    auto memory = m_owner->GetMemory();

    size_t max = (memory ? memory->GetSize() : DEFAULT_MAX) - 1;

    BaseAddressCharCount = (int)std::ceil(std::log10(max) / std::log10(16));

    BaseAddressWidthPx = std::min(BaseAddressCharCount * CharSizePx.x, MIN_BASE_ADDR_SIZE);

    GapSizePx.x = CharSizePx.x / 2;
    GapSizePx.y = 0;

    CharAreaWidthPx = CharSizePx.x * LineByteCount;

    TotalLineCount = (int)std::ceil((float)max / LineByteCount);

    LineCellsWidthChars = LineByteCount * cellMetrics.CharacterCount;

    LineCellsWidthPx = 
        (LineByteCount * cellMetrics.GapPx.x) +
        (LineByteCount * cellMetrics.SizePx.x);

    CharAreaStartX = (GapSizePx.x * 2) + LineCellsWidthPx;

    LineSizePx.x = LineCellsWidthChars * CharSizePx.x;
    LineSizePx.y = CharSizePx.y;

    OffsetHeightPx = LineSizePx.y;

    TotalScrollPx.x = CharAreaWidthPx + LineCellsWidthPx + GapSizePx.x;
    TotalScrollPx.y = CharSizePx.y * TotalLineCount;
}

/*=======================================================================*/
