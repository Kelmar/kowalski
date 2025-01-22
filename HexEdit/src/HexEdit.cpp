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

//#include "encodings.h"

#include "HexEdit.h"
#include "he_priv.h"

#include "OffsetColumns.h"
#include "BaseAddressRows.h"

#include "HexArea.h"

/*=======================================================================*/

using namespace hex;

/*=======================================================================*/

#define HE_STYLE (wxVSCROLL | wxHSCROLL | wxWANTS_CHARS)

/*=======================================================================*/

HexEdit::HexEdit()
    : wxScrolled()
    , m_memory(nullptr)
    , m_offsets(nullptr)
    , m_baseAddresses(nullptr)
    , m_selStart(0)
    , m_selLen(0)
    , m_mouseTrack(false)
{
    Init();
}

HexEdit::HexEdit(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const wxString &name)
    : wxScrolled(parent, id, pos, size, HE_STYLE, name)
    , m_memory(nullptr)
    , m_offsets(nullptr)
    , m_baseAddresses(nullptr)
    , m_selStart(0)
    , m_selLen(0)
    , m_mouseTrack(false)
{
    Init();
}

HexEdit::~HexEdit()
{
}

/*=======================================================================*/

wxBEGIN_EVENT_TABLE(HexEdit, wxScrolled<wxWindow>)
    EVT_SIZE(HexEdit::OnSize)
wxEND_EVENT_TABLE()

/*=======================================================================*/

void HexEdit::Init()
{
    DisableKeyboardScrolling();

    m_metrics = std::make_shared<DrawMetrics>(this);

    m_offsets = new OffsetColumns(this);
    m_baseAddresses = new BaseAddressRows(this);
    m_hexArea = new HexArea(this, m_baseAddresses, m_offsets);

    wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 2, m_metrics->GapSizePx);

    sizer->Add(m_metrics->BaseAddressWidthPx, m_metrics->OffsetHeightPx);

    sizer->Add(m_offsets, wxSizerFlags().Expand());
    sizer->Add(m_baseAddresses, wxSizerFlags().Expand());

    m_areaSizer = sizer->Add(m_hexArea, wxSizerFlags().Expand());

    sizer->AddGrowableRow(1);
    sizer->AddGrowableCol(1);

    SetSizer(sizer);

    SetTargetWindow(m_hexArea);

    AdjustItemSizes();
}

/*=======================================================================*/

uint32_t HexEdit::GetCurrentLine() const
{
    //int pos = GetScrollPos(wxVERTICAL);
    return 0;
}

/*=======================================================================*/

uint32_t HexEdit::GetCurrentColumn() const
{
    return 0;
}

/*=======================================================================*/

uint32_t HexEdit::GetAddress() const
{
    int pos = GetScrollPos(wxVERTICAL);
    return pos * m_metrics->LineByteCount;
}

/*=======================================================================*/

void HexEdit::JumpTo(uint32_t address)
{
    if (!m_memory)
        return;

    if (address < m_memory->GetSize())
    {
        int line = address / m_metrics->LineByteCount;
        Scroll(-1, line);
    }
}

/*=======================================================================*/

void HexEdit::AdjustItemSizes()
{
    // Preserve the current scrolled to position.
    wxPoint p = CalcUnscrolledPosition(wxPoint(0, 0));
    int lineStart = p.y / m_metrics->LineSizePx.y;

    m_metrics = std::make_shared<DrawMetrics>(this);

    m_areaSizer->SetMinSize(m_metrics->LineSizePx.x, 0);

    Layout();

    int charCount = (int)std::ceil((float)m_metrics->TotalScrollPx.x / m_metrics->CharSizePx.x);

    int displayLineCount = GetRect().GetHeight() / m_metrics->LineSizePx.y;

    // Add some virtual space for asthetics.
    int totalLines = m_metrics->TotalLineCount +(displayLineCount / 4);

    SetScrollbars(
        m_metrics->CharSizePx.x, m_metrics->LineSizePx.y,
        charCount, totalLines,
        0, lineStart);

    AdjustScrollbars();
}

/*=======================================================================*/
// Events
/*=======================================================================*/

void HexEdit::OnSize(wxSizeEvent &)
{
    AdjustItemSizes();
}

/*=======================================================================*/
