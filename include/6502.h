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

/*************************************************************************/

#ifndef APP_6502_H__
#define APP_6502_H__

/*************************************************************************/

// 6502.h : main header file for the 6502 application
//

#include "Global.h"
#include "MainFrm.h"
#include "encodings.h"

class CIOWindow;

/*************************************************************************/
// C6502App:
// See 6502.cpp for the implementation of this class
//

class C6502App : public wxApp
{
private:
    CMainFrame *m_mainFrame;
    wxLogWindow *m_logFrame;

    wxConfig *m_config;

    bool InitFrame();

public:
    static bool m_bMaximize; // flag - maximum window dimensions at startup;
    static bool m_bFileNew;  // flag - opening a blank document at startup

    CGlobal m_global;        // application global variables

    bool m_bDoNotAddToRecentFileList;
    //CDocTemplate *m_pDocDeasmTemplate;

    /* constructor */ C6502App();
    virtual          ~C6502App();

    bool OnInit() override;

    void LoadEncodings();

    CMainFrame *mainFrame() const { return m_mainFrame; }
    wxLogWindow *logFrame() const { return m_logFrame; }

    wxConfig &Config() { return *m_config; }

    virtual void AddToRecentFileList(const std::string &pathName);

    wxFrame *CreateChildFrame(wxView *view);

    virtual bool OnExceptionInMainLoop() override;

    virtual int FilterEvent(wxEvent &event) override;

protected:
    int OnExit() override;

protected:
    // Application Events

    void OnAppExit(wxCommandEvent &);
};

wxDECLARE_APP(C6502App);

/////////////////////////////////////////////////////////////////////////////

#endif /* APP_6502_H__ */
