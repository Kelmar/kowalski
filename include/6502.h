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

#include "Global.h"
#include "MainFrm.h"
#include "Splash.h"
#include "encodings.h"

#include "Options.h"

#include "FontController.h"
#include "ProjectManager.h"
#include "SimulatorController.h"

class CIOWindow;

/*************************************************************************/

class C6502App : public wxApp
{
private:
    // This needs to be created first.
    OptionsController *m_optionsController;

    FontController *m_fontController;
    ProjectManager *m_projectManager;
    SimulatorController *m_simulatorController;

    CMainFrame *m_mainFrame;
    wxLogWindow *m_logFrame;
    wxConfig *m_config;

    std::unique_ptr<SplashController> m_splash;

    bool LoadResources();
    bool InitFrame();

    void SetupSingletons();
    void DisposeSingletons();

    void InitOptionPages();

    void InitBinaryTemplates();

    void InnerUpdateStatus(const std::string &str);

public:
#if 0
    // TODO: Move to config
    bool m_bMaximize; // flag - maximum window dimensions at startup;
    bool m_bFileNew;  // flag - opening a blank document at startup
#endif

    CGlobal m_global;        // application global variables

    bool m_bDoNotAddToRecentFileList;
    //CDocTemplate *m_pDocDeasmTemplate;

    /* constructor */ C6502App();
    virtual          ~C6502App();

    bool OnInit() override;

    void LoadEncodings();

    FontController &fontController() const { return *m_fontController; }
    ProjectManager &projectManager() const { return *m_projectManager; }
    SimulatorController &simulatorController() const { return *m_simulatorController; }
    OptionsController &optionsController() const { return *m_optionsController; }

    CMainFrame *mainFrame() const { return m_mainFrame; }
    wxLogWindow *logFrame() const { return m_logFrame; }

    wxConfig &Config() { return *m_config; }

    virtual void AddToRecentFileList(const std::string &pathName);

    wxFrame *CreateChildFrame(wxView *view);

    virtual bool OnExceptionInMainLoop() override;

    virtual int FilterEvent(wxEvent &event) override;

    const wxStandardPaths &GetStandardPaths() const { return m_traits->GetStandardPaths(); }

    wxPathList GetResourcePaths() const;

    template <typename... T>
    void UpdateStatus(const wxString &fmt, T&&... args)
    {
        InnerUpdateStatus(fmt::vformat(fmt.ToStdString(), fmt::make_format_args(args...)));
    }

protected:
    int OnExit() override;

protected:
    // Application Events

    void OnAppExit(wxCommandEvent &);
};

wxDECLARE_APP(C6502App);

/////////////////////////////////////////////////////////////////////////////

#endif /* APP_6502_H__ */
