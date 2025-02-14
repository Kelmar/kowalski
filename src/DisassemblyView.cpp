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
#define GAP_SIZE 2

/*=======================================================================*/

namespace
{
    std::string EmptyFormat(const std::span<uint8_t> &) { return std::string(""); }

    std::string ByteFormat(const std::span<uint8_t> &bytes)
    {
        return fmt::format("{:02X}", bytes[0]);
    }

    std::string TwoByteFormat(const std::span<uint8_t> &bytes)
    {
        return fmt::format("{:02X}, {:02X}", bytes[0], bytes[1]);
    }

    std::string ByteFormatX(const std::span<uint8_t> &bytes)
    {
        return fmt::format("{:02X}, X", bytes[0]);
    }

    std::string ByteFormatY(const std::span<uint8_t> &bytes)
    {
        return fmt::format("{:02X}, Y", bytes[0]);
    }

    std::string ByteFormatS(const std::span<uint8_t> &bytes)
    {
        return fmt::format("{:02X}, S", bytes[0]);
    }

    std::string ByteFormatSY(const std::span<uint8_t> &bytes)
    {
        return fmt::format("({:02X}, S), Y", bytes[0]);
    }

    std::string WordFormat(const std::span<uint8_t> &bytes)
    {
        uint16_t word = bytes[0] << 8 | bytes[1];
        return fmt::format("{:04X}", word);
    }

    std::string WordFormatX(const std::span<uint8_t> &bytes)
    {
        uint16_t word = bytes[0] << 8 | bytes[1];
        return fmt::format("{:04X}, X", word);
    }

    std::string WordFormatY(const std::span<uint8_t> &bytes)
    {
        uint16_t word = bytes[0] << 8 | bytes[1];
        return fmt::format("{:04X}, Y", word);
    }

    std::string LWordFormat(const std::span<uint8_t> &bytes)
    {
        uint32_t lword = bytes[0] << 16 | bytes[1] << 8 | bytes[0];
        return fmt::format("{:06X}", lword);
    }

    std::string LWordFormatX(const std::span<uint8_t> &bytes)
    {
        uint32_t lword = bytes[0] << 16 | bytes[1] << 8 | bytes[0];
        return fmt::format("{:06X}, X", lword);
    }

    std::string ZeroPageIndirect(const std::span<uint8_t> &bytes)
    {
        return fmt::format("[{:02X}]", bytes[0]);
    }

    std::string ZeroPageIndirectY(const std::span<uint8_t> &bytes)
    {
        return fmt::format("[{:02X}], Y", bytes[0]);
    }

    std::string UnknownFormatToDo(const std::span<uint8_t> &)
    {
        return "Unknown format TODO";
    }

    /*===============================================================*/

    typedef std::function<std::string(const std::span<uint8_t> &)> ArgumentFormatter;

    const std::map<CAsm::CodeAdr, ArgumentFormatter> s_argumentFormatters
    {
        { CAsm::CodeAdr::A_IMP, EmptyFormat },
        { CAsm::CodeAdr::A_ACC, EmptyFormat },
        { CAsm::CodeAdr::A_IMP, EmptyFormat },
        { CAsm::CodeAdr::A_IMM, ByteFormat },
        { CAsm::CodeAdr::A_ZPG, ByteFormat },
        { CAsm::CodeAdr::A_ABS, WordFormat },
        { CAsm::CodeAdr::A_ABS_X, WordFormatX },
        { CAsm::CodeAdr::A_ABS_Y, WordFormatY },
        { CAsm::CodeAdr::A_ZPG_X, ByteFormatX },
        { CAsm::CodeAdr::A_ZPG_Y, ByteFormatY },
        { CAsm::CodeAdr::A_REL, WordFormat },
        { CAsm::CodeAdr::A_ZPGI, ByteFormat },
        { CAsm::CodeAdr::A_ZPGI_X, ByteFormatX },
        { CAsm::CodeAdr::A_ZPGI_Y, ByteFormatY },
        { CAsm::CodeAdr::A_ABSI, WordFormat },
        { CAsm::CodeAdr::A_ABSI_X, WordFormatX },

        { CAsm::CodeAdr::A_ZPG2, UnknownFormatToDo },
        { CAsm::CodeAdr::A_ZREL, UnknownFormatToDo },

        { CAsm::CodeAdr::A_ABSL, LWordFormat },
        { CAsm::CodeAdr::A_ABSL_X, LWordFormatX },

        { CAsm::CodeAdr::A_ZPIL, ZeroPageIndirect },
        { CAsm::CodeAdr::A_ZPIL_Y, ZeroPageIndirectY },

        { CAsm::CodeAdr::A_SR, ByteFormatS },
        { CAsm::CodeAdr::A_SRI_Y, ByteFormatSY },

        { CAsm::CodeAdr::A_RELL, WordFormat },

        { CAsm::CodeAdr::A_XYC, TwoByteFormat },

        { CAsm::CodeAdr::A_IMM2, WordFormat },

        { CAsm::CodeAdr::A_ILL, EmptyFormat }
    };
}

/*=======================================================================*/
/*=======================================================================*/

DisassemblyView::DisassemblyView()
    : wxScrolled()
    , m_offset(sim::INVALID_ADDRESS)
{
    Init();
}

DisassemblyView::DisassemblyView(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const wxString &name)
    : wxScrolled(parent, id, pos, size, DV_STYLE, name)
    , m_offset(sim::INVALID_ADDRESS)
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

    // Set initial scrolling value to the current program address.

    PSym6502 simulator = wxGetApp().simulatorController().Simulator();

    m_offset = simulator->GetContext().GetProgramAddress();
}

/*=======================================================================*/

void DisassemblyView::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

    PSym6502 simulator = wxGetApp().simulatorController().Simulator();
    CDeasm disassembler(simulator);

    m_programAddress = simulator->GetContext().GetProgramAddress();

    DisassembleInfo info;

    wxFont font = wxGetApp().fontController().getMonoFont();
    dc.SetFont(font);

    m_charSize = dc.GetTextExtent("M");

    wxRect client = GetClientRect();
    int lines = std::ceil((float)client.GetHeight() / m_charSize.y);

    info.Address = m_offset;

    m_drawLine = 0;

    for (int i = 0; i < lines; ++i)
    {
        //// Simulate found label.
        //std::string testLabel = fmt::format("   label_{0}:", i);
        //dc.DrawText(testLabel, 0, y);
        //m_drawLine += sz.y;

        // TODO: Check if current address matches a label in the symbol table and add it.

        disassembler.Parse(&info);

        DrawLineMarks(dc, info);
        DrawLine(dc, info);

        m_drawLine += m_charSize.y;
        info.Address += info.Bytes.size();
    }
}

/*=======================================================================*/

void DisassemblyView::DrawLineMarks(wxDC &dc, DisassembleInfo &info)
{
    UNUSED(dc);
    UNUSED(info);

    /*
    Breakpoint bp = wxGetApp().m_global.GetBreakpoint(uint16_t(ptr));

    if (bp & BPT_MASK) // Is there a break point?
        draw_breakpoint(*dc, mark.left, mark.top, m_nFontHeight, bp & BPT_DISABLED ? false : true);

    if (info.Address == m_programAddress)
    {
        draw_pointer(*dc, mark.left, mark.top, m_nFontHeight);
    }
    */
}

/*=======================================================================*/

void DisassemblyView::DrawLine(wxDC &dc, DisassembleInfo &info)
{
    int x = 0;

    std::string txt = fmt::format("${0:06X}", info.Address);

    dc.DrawText(txt, x, m_drawLine);

    x += m_charSize.x * (7 + GAP_SIZE);

    dc.DrawText(info.Mnemonic, x, m_drawLine);

    // TODO: Pull max mnemonic size from platform.
    x += m_charSize.x * (3 + GAP_SIZE);

    auto paramFmt = s_argumentFormatters.find(info.AddressingMode);

    if (paramFmt != s_argumentFormatters.end())
    {
        std::span<uint8_t> bytes(info.Bytes);
        txt = paramFmt->second(bytes.subspan(1));
    }
    else
    {
        txt = "";
    }

    dc.DrawText(txt, x, m_drawLine);

    // TODO: Figure out max parameter size?
    x += m_charSize.x * (10 + GAP_SIZE);

    auto mode = CAsm::ADDRESS_MODES.find(info.AddressingMode);
    txt = mode == CAsm::ADDRESS_MODES.end() ? _("Unknown Addressing Mode!") : wxString(mode->second);

    x += m_charSize.x * (10 + GAP_SIZE);

    dc.DrawText(txt, x, m_drawLine);
}

/*=======================================================================*/
