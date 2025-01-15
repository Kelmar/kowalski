/*************************************************************************/
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
/*************************************************************************/

#include "StdAfx.h"
#include "6502.h"
#include "config.h"

#include "MemFrame.h"
#include "FormatNums.h"

/*************************************************************************/
// Configuration
/*************************************************************************/

namespace
{
    struct MemoryFrameConfig
    {
        int sizeH;   // Window height
        int sizeW;   // Window width
        int posX;    // Window X position
        int posY;    // Window Y position
        bool hidden; // Set if window should be hidden
    };

    struct MemoryFrameConfigMap : config::Mapper<MemoryFrameConfig>
    {
        bool to(MemoryFrameConfig &cfg, config::Context &ctx) const
        {
            return
                ctx.map("MemoryH", cfg.sizeH) ||
                ctx.map("MemoryW", cfg.sizeW) ||
                ctx.map("MemoryXPos", cfg.posX) ||
                ctx.map("MemoryYPos", cfg.posY) ||
                ctx.map("MemoryWndHidden", cfg.hidden)
            ;
        }
    };

    MemoryFrameConfig s_frameConfig;

    // Setup default config for the MemoryFrame
    void InitDefaultConfig()
    {
        s_frameConfig =
        {
            .sizeH = wxDefaultSize.GetHeight(),
            .sizeW = wxDefaultSize.GetWidth(),
            .posX = wxDefaultPosition.x,
            .posY = wxDefaultPosition.y,
            .hidden = false
        };
    }

    // Load in configuration for the MemoryFrame
    void LoadConfig()
    {
#if WIN32
        config::source::WinRegistry reg("/");
        
        if (!reg.read("MemoryFrame", s_frameConfig))
            InitDefaultConfig();
#else
        InitDefaultConfig();
#endif
    }

    // Save the configuration for the MemoryFrame
    void SaveConfig()
    {

    }
}

/*************************************************************************/
/*************************************************************************/

MemoryFrame::MemoryFrame(wxWindow *parent)
    : wxPanel()
    , wxExtra(this)
    , m_hexView(nullptr)
{
    if (!wxXmlResource::Get()->LoadPanel(this, parent, "MemoryFrame"))
        throw ResourceError();

    WX_BIND_CHILD(m_jumpEdit);

    m_hexView = new HexView(this, wxID_ANY);

    wxXmlResource::Get()->AttachUnknownControl("m_hexView", m_hexView);

    m_hexView->SetMemory(&wxGetApp().m_global.GetMemory());

    m_jumpEdit->SetValue("");

    BindEvents();

    LoadConfig();
}

MemoryFrame::~MemoryFrame()
{
    SaveConfig();
}

/*************************************************************************/

void MemoryFrame::BindEvents()
{
    m_jumpEdit->Bind(wxEVT_TEXT_ENTER, &MemoryFrame::OnJumpTo, this);
}

/*************************************************************************/

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
                addr = m_hexView->GetAddress() - res;
                break;

            default:
            case 0:
                addr = res;
                break;

            case 1:
                addr = m_hexView->GetAddress() + res;
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
        m_hexView->JumpTo(addr);
        m_jumpEdit->SetValue("");
    }
}

/*************************************************************************/
