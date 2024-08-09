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
#include "HexView.h"

/*************************************************************************/

HexView::HexView()
    : wxControl()
    , m_span()
    , m_groupBy(0)
    , m_pageSize(0)
    , m_fontDirty(false)
    , m_digitFont(nullptr)
{
    LoadFonts();
}

HexView::HexView(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
    : wxControl(parent, id, pos, size, style, wxDefaultValidator, name)
    , m_span()
    , m_groupBy(0)
    , m_pageSize(0)
    , m_fontDirty(false)
    , m_digitFont(nullptr)
{
    LoadFonts();
}

HexView::~HexView()
{
    delete m_digitFont;
}

wxIMPLEMENT_DYNAMIC_CLASS(HexView, wxControl);

/*************************************************************************/

wxBEGIN_EVENT_TABLE(HexView, wxControl)
    EVT_PAINT(HexView::OnPaint)
wxEND_EVENT_TABLE()

/*************************************************************************/

int HexView::GetLineCount() const
{
    if (m_span.empty())
        return 0; // Memory not set, no lines

    size_t sz = m_span.size();
    int groups = m_groupBy; // Assume fixed width of bytes
    int lines;

    if (groups == 0)
    {
        // Attempt to fill as much of the control's client area as possible
        wxRect rect = GetClientRect();

        // TODO: We need three sections, the address, the data and then the character display.

        //wxSize fontSz = m_digitFont->GetPixelSize();

        // Faking a font width for now

        groups = rect.width / 8; // fontSz.GetWidth();
    }

    lines = sz / groups;
    lines += ((sz % groups) != 0 ? 1 : 0);

    if (m_pageSize > 0)
        lines += (sz / m_pageSize); // Add lines for page boundary markers.

    return lines;
}

/*************************************************************************/

void HexView::LoadFonts()
{
    if (m_digitFont && !m_fontDirty)
        return;

    wxFontInfo info;

    info.Family(wxFONTFAMILY_TELETYPE);
    info.AntiAliased(true);

    delete m_digitFont;
    m_digitFont = new wxFont(info);
}

/*************************************************************************/

std::string HexView::GetAddressFormat() const
{
    std::stringstream out;
    wxString txt;

    // Compute the number of digits in the address
    int sz = m_span.size() - 1;
    sz = txt.Printf("%X", sz);

    // Build the format string
    out << "%0" << sz << "X ";
    return out.str();
}

/*************************************************************************/

void HexView::OnPaint(wxPaintEvent &)
{
    wxClientDC dc(this);
    LoadFonts();
    Draw(dc);
}

void HexView::Draw(wxDC &dc)
{
    wxRect rect = GetClientRect();

    wxColor c("white");

    dc.FloodFill(wxPoint(0, 0), c);
    dc.SetFont(*m_digitFont);

    //int lines = GetLineCount();

    wxSize fontSz = m_digitFont->GetPixelSize();

    wxCoord x;
    wxCoord y;

    dc.GetTextExtent(" ", &x, &y);

    fontSz.SetWidth(x);

    uint32_t addr = 0;
    wxString addrFmt = GetAddressFormat();
    wxString charDisp;
    wxString txt;

    x = 0;
    y = 0;

    for (int l = 0; y < rect.GetHeight() && addr < m_span.size(); ++l)
    {
        int w = txt.Printf(addrFmt, addr);
        dc.DrawText(txt, x, y);
        x += w * fontSz.GetWidth();

        // Add gap for line
        x += 2 * fontSz.GetWidth();

        charDisp = "";

        for (int i = 0; i < LINE_WIDTH && addr < m_span.size(); ++i, ++addr)
        {
            uint8_t val = m_span[addr];

            // TODO: Allow map to other character sets.

            if (val < 32)
                charDisp += ".";
            else if (val < 128)
                charDisp += (char)val;
            else
            {
                // TODO: Handle other character maps
                //charDisp += (char)val;
                charDisp += ".";
            }

            w = txt.Printf("%02X ", val);

            dc.DrawText(txt, x, y);
            x += w * fontSz.GetWidth();
        }

        // Add gap for line
        x += 2 * fontSz.GetWidth();

        dc.DrawText(charDisp, x, y);

        y += fontSz.GetHeight();
        x = 0;
    }

    // Find location of first line

    int w = txt.Printf(addrFmt, 0);
    x = (int)((w + 0.5) * fontSz.GetWidth());

    dc.DrawLine(x, 0, x, rect.GetHeight());

    // Find location of second line
    x += (int)(fontSz.GetWidth() * ((16 * 3) + 2));

    dc.DrawLine(x, 0, x, rect.GetHeight());
}

/*************************************************************************/
