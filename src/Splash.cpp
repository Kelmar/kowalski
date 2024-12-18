/*-----------------------------------------------------------------------------
        6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#include "StdAfx.h"

#include "6502.h"
#include "Splash.h"

/*************************************************************************/
/*************************************************************************/

#define SPLASH_STYLE (wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP)
#define SPLASH_FILE_NAME "splash.bmp"
#define SPLASH_DELAY 2000 /* two seconds */

#define TIMER_ID 99999

/*************************************************************************/
/*************************************************************************/

SplashController::SplashController()
    : wxFrame(nullptr, wxID_ANY, wxEmptyString, wxPoint(0, 0), wxSize(640, 480), SPLASH_STYLE)
    , m_continueWith()
    , m_window(nullptr)
    , m_timer()
{
    m_timer.SetOwner(this, TIMER_ID);

    Bind(wxEVT_TIMER, &SplashController::OnTimer, this, TIMER_ID);
    Bind(wxEVT_CLOSE_WINDOW, &SplashController::OnCloseWindow, this);

    CenterOnScreen();

    m_window = new SplashWnd(this);
    Show(true);
    m_window->SetFocus();

    Update();
    wxYield();
}

SplashController::~SplashController()
{
    delete m_window;
}

/*************************************************************************/

void SplashController::SetStatus(const wxString &text)
{
    if (m_window)
    {
        m_window->SetStatus(text);

        /*
         * We're likely in a tight loading loop, we call wxYield() here to give
         * the UI a chance to update the changed status text.
         */
        Update();
        wxYield();
    }
}

/*************************************************************************/

void SplashController::Done(std::function<void()> continueWith)
{
    m_timer.Start(SPLASH_DELAY, true);
    m_continueWith = continueWith;
}

/*************************************************************************/

void SplashController::OnTimer(wxTimerEvent &)
{
    Close(true);
}

/*************************************************************************/

void SplashController::OnCloseWindow(wxCloseEvent &)
{
    m_timer.Stop();
    Destroy();

    if (m_continueWith)
        m_continueWith();
}

/*************************************************************************/
/*************************************************************************/

SplashWnd::SplashWnd(SplashController *parent)
    : wxWindow(
        parent,
        wxID_ANY,
        wxDefaultPosition,
        wxSize(640, 480),
        wxNO_BORDER)
    , m_bitmap()
    , m_text(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_TRANSIENT);

    LoadBackground();

    m_text = std::make_unique<wxStaticText>(this, wxID_ANY, "");

    m_text->SetPosition(wxPoint(10, 20));
    m_text->SetLabel("Sample Text");
    m_text->Fit();
    m_text->Show();

    Bind(wxEVT_PAINT, &SplashWnd::OnPaint, this);
}

SplashWnd::~SplashWnd()
{
}

/*************************************************************************/

void SplashWnd::LoadBackground()
{
    auto dirs = wxGetApp().GetResourcePaths();

    wxString file = dirs.FindAbsoluteValidPath(SPLASH_FILE_NAME);

    if (!file.IsEmpty())
    {
        m_bitmap.LoadFile(file);

        if (m_bitmap.IsOk())
            SetSize(m_bitmap.GetSize());
    }
}

/*************************************************************************/

void SplashWnd::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);

    if (m_bitmap.IsOk())
        dc.DrawBitmap(m_bitmap, 0, 0);
}

/*************************************************************************/
