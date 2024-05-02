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

#ifndef APP_6502_H__
#define APP_6502_H__

// 6502.h : main header file for the 6502 application
//

#include "resource.h"
#include "Global.h"

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

//#include "resource.h"       // main symbols

class CMainFrame;
class CIOWindow;

/////////////////////////////////////////////////////////////////////////////
// C6502App:
// See 6502.cpp for the implementation of this class
//

class C6502App : public wxApp
{
private:
    //static const char REGISTRY_KEY[];
    static const char PROFILE_NAME[];

    //HINSTANCE m_hInstRes;
    //HMODULE m_hRichEdit;

    CMainFrame *m_mainFrame;
    wxConfig *m_config;

public:
    static bool m_bMaximize;    // flag - maximum window dimensions at startup;
    static bool m_bFileNew;     // flag - opening a blank document at startup

    CGlobal m_global;           // application global variables

    bool m_bDoNotAddToRecentFileList;
    //CDocTemplate *m_pDocDeasmTemplate;

    /* constructor */ C6502App();
    virtual          ~C6502App();

    bool OnInit() override;

    wxConfig &Config() { return *m_config; }

    CIOWindow *ioWindow() { return nullptr; }// m_ioWindow; }

    virtual void SetStatusText(int col, const std::string &message);

    virtual void AddToRecentFileList(const std::string &pathName);
    //virtual int ExitInstance();

    void OnAppAbout();
    //DECLARE_MESSAGE_MAP()
};

wxDECLARE_APP(C6502App);

/////////////////////////////////////////////////////////////////////////////

#endif /* APP_6502_H__ */
