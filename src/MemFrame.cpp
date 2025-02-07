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

#include "MemFrame.h"
#include "FormatNums.h"

/*=======================================================================*/
// Configuration
/*=======================================================================*/

namespace
{
    static const std::string MEM_FRAME_CONFIG_PATH = "MEMWINDOW";

    struct MemoryFrameConfig
    {
        int sizeH;   // Window height
        int sizeW;   // Window width
        int posX;    // Window X position
        int posY;    // Window Y position
        bool hidden; // Set if window should be hidden
    };

    MemoryFrameConfig s_memFrameConfig;
}

/*=======================================================================*/
/*=======================================================================*/
// Memory adapter

namespace
{
    class OutputMemView : public hex::MemoryView
    {
    public:
        OutputMemView()
            : hex::MemoryView()
        {
#if 0
            // For testing
            auto mem = &wxGetApp().m_global.GetMemory();

            for (int i = 0; i < 256; ++i)
                mem->set(i, (uint8_t)i);
#endif
        }

        virtual size_t GetSize() const override
        {
            auto mem = &wxGetApp().m_global.GetMemory();
            return mem->size();
        }

        virtual uint8_t GetByte(uint32_t address) const override
        {
            auto mem = &wxGetApp().m_global.GetMemory();
            sim_addr_t addr = (sim_addr_t)address;
            return mem->get(addr);
        }

        virtual void SetByte(uint32_t address, uint8_t value) override
        {
            auto mem = &wxGetApp().m_global.GetMemory();
            sim_addr_t addr = (sim_addr_t)address;
            return mem->set(addr, value);
        }
    };
}

/*=======================================================================*/
/*=======================================================================*/

MemoryFrame::MemoryFrame(wxWindow *parent)
    : wxPanel()
    , wxExtra(this)
    , m_hexEdit(nullptr)
{
    if (!wxXmlResource::Get()->LoadPanel(this, parent, "MemoryFrame"))
        throw ResourceError();

    WX_BIND_CHILD(m_jumpEdit);

    m_hexEdit = new hex::HexEdit(this, wxID_ANY);

    auto monoFont = wxGetApp().fontController().getMonoFont();
    m_hexEdit->SetFont(monoFont);

    wxXmlResource::Get()->AttachUnknownControl("m_hexView", m_hexEdit);

    m_hexEdit->SetMemory(new OutputMemView());

    m_jumpEdit->SetValue("");

    BindEvents();

    LoadConfig();
}

MemoryFrame::~MemoryFrame()
{
    SaveConfig();
}

/*=======================================================================*/

void MemoryFrame::BindEvents()
{
    m_jumpEdit->Bind(wxEVT_TEXT_ENTER, &MemoryFrame::OnJumpTo, this);
}

/*=======================================================================*/

void MemoryFrame::SaveConfig()
{
    auto config = wxConfig::Get();
    wxString oldPath = config->GetPath();

    auto restorePath = deferred_action([&] () { config->SetPath(oldPath); });

    config->SetPath(MEM_FRAME_CONFIG_PATH);

    config->Write("MemoryH", s_memFrameConfig.sizeH);
    config->Write("MemoryW", s_memFrameConfig.sizeW);
    config->Write("MemoryXPos", s_memFrameConfig.posX);
    config->Write("MemoryYPos", s_memFrameConfig.posY);
    config->Write("MemoryWndHidden", s_memFrameConfig.hidden);
}

/*=======================================================================*/

void MemoryFrame::LoadDefaults()
{

}

/*=======================================================================*/

void MemoryFrame::LoadConfig()
{
    LoadDefaults();

#if WIN32
    // TODO: Import old config if detected.
#endif

    auto config = wxConfig::Get();
    wxString oldPath = config->GetPath();

    auto restorePath = deferred_action([&] () { config->SetPath(oldPath); });

    config->SetPath(MEM_FRAME_CONFIG_PATH);

    s_memFrameConfig.sizeH = config->ReadLong("MemoryH", s_memFrameConfig.sizeH);
    s_memFrameConfig.sizeW = config->ReadLong("MemoryW", s_memFrameConfig.sizeW);
    s_memFrameConfig.posX = config->ReadLong("MemoryXPos", s_memFrameConfig.posX);
    s_memFrameConfig.posY= config->ReadLong("MemoryYPos", s_memFrameConfig.posY);
    s_memFrameConfig.hidden = config->ReadBool("MemoryWndHidden", s_memFrameConfig.hidden);
}

/*=======================================================================*/

void MemoryFrame::OnJumpTo(wxCommandEvent &)
{
    std::string val = m_jumpEdit->GetValue().ToStdString();

    val = val | str::toUpper | str::trim;
    uint32_t addr = CAsm::INVALID_ADDRESS;

    // TODO: Needs adjusting for 65816

    if ((val == "ZP") || (val == "ZEROPAGE"))
    {
        addr = 0;
    }
    else if ((val == "SP") || (val == "STACK") || (val == "STACKPOINTER"))
    {
        addr = 0x0100;
    }
    else
    {
        int relJump = 0;

        // Check to see if user has requested a relative jump

        if ((val[0] == '+') || (val[0] == '-'))
        {
            relJump = val[0] == '+' ? 1 : -1;
            val = val.substr(1);
        }

        // Try to parse an actual address

        uint32_t res;
        NumberFormat status = NumberFormats::FromString(val, _Out_ res);

        if (status != NumberFormat::Error)
        {
            switch (relJump)
            {
            case -1:
                addr = m_hexEdit->GetAddress() - res;
                break;

            default:
            case 0:
                addr = res;
                break;

            case 1:
                addr = m_hexEdit->GetAddress() + res;
                break;
            }
        }
        else
        {
            wxLogStatus(_("Unknown address format in Jump To command."));
        }
    }

    if (addr != CAsm::INVALID_ADDRESS)
    {
        m_hexEdit->JumpTo(addr);
        m_jumpEdit->SetValue("");
    }
}

/*=======================================================================*/
