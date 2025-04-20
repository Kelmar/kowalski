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

#include "RegisterView.h"
#include "SimulatorController.h"

/*=======================================================================*/

RegisterView::RegisterView(wxWindow *parent)
    : wxExtra(this)
    , m_instDisplay(nullptr)
    , m_instAlert(nullptr)
    , m_aEdit(nullptr)
    , m_xEdit(nullptr)
    , m_yEdit(nullptr)
    , m_spEdit(nullptr)
    , m_pcEdit(nullptr)
    , m_cycleEdit(nullptr)
    , m_pEdit(nullptr)
{
    if (!wxXmlResource::Get()->LoadFrame(this, parent, "Reg6502Frame"))
        throw ResourceError();

    InitControlls();

    auto winColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    SetBackgroundColour(winColor);

    Fit();

    BindEvents();
}

/*=======================================================================*/

void RegisterView::InitControlls()
{
    WX_BIND_CHILD(m_instDisplay);
    WX_BIND_CHILD(m_instAlert);

    WX_BIND_CHILD(m_aEdit);
    WX_BIND_CHILD(m_xEdit);
    WX_BIND_CHILD(m_yEdit);
    WX_BIND_CHILD(m_spEdit);
    WX_BIND_CHILD(m_pcEdit);
    WX_BIND_CHILD(m_cycleEdit);
    WX_BIND_CHILD(m_pEdit);
}

/*=======================================================================*/

void RegisterView::BindEvents()
{
    Bind(wxEVT_CLOSE_WINDOW, &RegisterView::OnClose, this);
}

/*=======================================================================*/

void RegisterView::UpdateStatus()
{
    // TODO: Going to redo this, going to add a perpective manager.

    DebugState state = wxGetApp().simulatorController().CurrentState();

    switch (state)
    {
    case DebugState::Unloaded:
    case DebugState::NotStarted:
        Show(false);
        GetParent()->UpdateWindowUI();
        return;

    case DebugState::Running:
        // Disable all controls while the simulation is running.
        Enable(false);
        return; // Do not update controls

    case DebugState::Stopped:
        Enable(true);
        break;

    case DebugState::Finished:
        Enable(false);
        break;
    }

    auto simulator = wxGetApp().simulatorController().Simulator();

    if (!simulator)
        return;

    CContext &context = simulator->GetContext();

    m_aEdit->SetValue(fmt::format("${0:02X}", context.a));
    m_xEdit->SetValue(fmt::format("${0:02X}", context.x));
    m_yEdit->SetValue(fmt::format("${0:02X}", context.y));
}

/*=======================================================================*/

void RegisterView::OnClose(wxCloseEvent &e)
{
    if (e.CanVeto())
    {
        // We just want to hide ourselves, not get destroied.
        e.Veto();

        Show(false);
        GetParent()->UpdateWindowUI();
    }
    else
        Destroy();
}

/*=======================================================================*/
