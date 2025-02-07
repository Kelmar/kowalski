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

#include "Deasm.h"

#include "DisassemblyView.h"

/*=======================================================================*/

#define DV_STYLE (wxVSCROLL | wxHSCROLL)

/*=======================================================================*/

DisassemblyView::DisassemblyView()
    : wxScrolled()
{
    Init();
}

DisassemblyView::DisassemblyView(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const wxString &name)
    : wxScrolled(parent, id, pos, size, DV_STYLE, name)
{
    Init();
}

DisassemblyView::~DisassemblyView()
{
}

/*=======================================================================*/

void DisassemblyView::Init()
{
    Bind(wxEVT_PAINT, &DisassemblyView::OnPaint, this);
}

/*=======================================================================*/

void DisassemblyView::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

    PSym6502 simulator = wxGetApp().simulatorController().Simulator();
    CDeasm disassembler(simulator);

    DisassembleInfo info;

    wxFont font = wxGetApp().fontController().getMonoFont();
    dc.SetFont(font);

    wxSize sz = dc.GetTextExtent("M");

    wxRect client = GetClientRect();
    int lines = std::ceil((float)client.GetHeight() / sz.y);

    info.Address = 0x8000; // Hard coded for the moment

    for (int i = 0, y = 0; i < lines; ++i)
    {
        disassembler.Parse(&info);

        dc.DrawText(info.Mnemonic, 0, y);

        y += sz.y;

        info.Address += info.Bytes.size();
    }
}

/*=======================================================================*/
