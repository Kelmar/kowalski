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

// 6502.cpp : Defines the class behaviors for the application.
//

#include "StdAfx.h"

#include "MainFrm.h"
#include "ProjectManager.h"

#include "6502Doc.h"
#include "6502View.h"

#include "Events.h"

//#include "ChildFrm.h"
//#include "ChildFrmDeAsm.h"
//#include "Deasm6502Doc.h"
//#include "Deasm6502View.h"
//#include "Splash.h"
//#include "CXMultiDocTemplate.h"
//#include "About.h"

const char VENDOR_NAME[] = "MikSoft";
const char APP_NAME[] = "6502_simulator";

bool C6502App::m_bMaximize = false; // flag - maximum window dimensions at startup;
bool C6502App::m_bFileNew = true; // flag - opening a blank document at startup

/*************************************************************************/
// C6502App

C6502App::C6502App()
    : m_mainFrame(nullptr)
    , m_config(nullptr)
    , m_bDoNotAddToRecentFileList(false)
{
}

C6502App::~C6502App()
{
    delete m_config;
}

wxIMPLEMENT_APP(C6502App);

/*************************************************************************/
// C6502App initialization

bool C6502App::OnInit()
{
    wxLog::EnableLogging();

    wxLogStatus("Application initializing");

    if (!wxApp::OnInit())
        return false;

    SetVendorName(VENDOR_NAME);
    SetAppName(APP_NAME);

    // Load up resources
    //wxXmlResource::Get()->AddHandler(hexViewHandler);
    wxXmlResource::Get()->InitAllHandlers();
    

    wxLogDebug("Loading resource file....");
    if (!wxXmlResource::Get()->Load("res6502.xrc"))
    {
        wxLogError("Unable to load XRC resources!");
        wxMessageBox(
            _("Unable to load resources!"),
            _("6502 Simulator - Fatal Error"),
            wxCENTER | wxOK | wxICON_ERROR);

        return false;
    }

    SetAppDisplayName(_("6502 Simulator"));

    m_config = new wxConfig();

    wxDocManager *docManager = new wxDocManager();

    new wxDocTemplate(docManager,
        "Assembly", "*.s;*.asm", "", "s;asm",
        "Assembly Source", "Assembly View",
        CLASSINFO(CSrc6502Doc), CLASSINFO(CSrc6502View));

    new ProjectManager();

    if (!InitFrame())
        return false;

    // File history has to be loaded after the frame is created.
    docManager->FileHistoryLoad(*m_config);

    m_mainFrame->Show();

    wxLogStatus("Application loaded!");

    return true;
}

bool C6502App::InitFrame()
{
    wxDocManager *docManager = wxDocManager::GetDocumentManager();

    m_mainFrame = new CMainFrame(docManager);

    m_logFrame = new wxLogWindow(m_mainFrame, _("Log"), false);
    wxLog::SetActiveTarget(m_logFrame);

    return true;
}

// This is always the first to handle an event!
int C6502App::FilterEvent(wxEvent &event)
{
    if (event.GetId() == evTHD_ASM_COMPLETE)
    {
        wxLogDebug("Got thread event!");
    }

    return wxApp::FilterEvent(event);
}

#if 0

BOOL C6502App::InitInstance()
{
    std::string strHelpFile = m_pszHelpFilePath;
    strHelpFile = strHelpFile.Left(strHelpFile.GetLength() - 4) + ".chm";

    //First free the string allocated by MFC at CWinApp startup.
    //The string is allocated before InitInstance is called.

    free((void *)m_pszHelpFilePath);

    // set new help file path
    m_pszHelpFilePath = _tcsdup(strHelpFile);

    //-------------------------------------------------------------------------------------
    // load resources
    m_hInstRes = LoadLibrary("Res.dll");
    if (m_hInstRes == NULL)
    {
        AfxMessageBox("Can't find resource DLL 'Res.dll'.", MB_OK | MB_ICONERROR);
        return false;
    }
    AfxSetResourceHandle(m_hInstRes);

    m_hRichEdit = ::LoadLibrary(_T("riched20.dll"));
    AfxInitRichEdit();

    // CG: The following block was added by the Splash Screen component.
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);

    CSplashWnd::ShowSplashScreen();	// przeniesione z CMainFrame::OnCreate

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

    INITCOMMONCONTROLSEX init;
    init.dwSize = sizeof init;
    init.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_USEREX_CLASSES;
    ::InitCommonControlsEx(&init);

#ifdef _AFXDLL
    Enable3dControls();			// Call this when using MFC in a shared DLL
#else
    //  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

    SetRegistryKey(REGISTRY_KEY);
    //First free the string allocated by MFC at CWinApp startup.
    //The string is allocated before InitInstance is called.
    free((void *)m_pszProfileName);
    //Change the name of the .INI file.
    //The CWinApp destructor will free the memory.
    m_pszProfileName = _tcsdup(PROFILE_NAME);

    LoadStdProfileSettings(_AFX_MRU_MAX_COUNT);  // Load standard INI file options (including MRU)

    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views.

    CXMultiDocTemplate *pDocTemplate;
    pDocTemplate = new CXMultiDocTemplate(
        IDR_SRC65TYPE,
        RUNTIME_CLASS(CSrc6502Doc),
        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
        RUNTIME_CLASS(CSrc6502View));

    AddDocTemplate(pDocTemplate);

    pDocTemplate = new CXMultiDocTemplate(
        IDR_DEASM65TYPE,
        RUNTIME_CLASS(CDeasm6502Doc),
        RUNTIME_CLASS(CChildFrameDeAsm), // custom MDI child frame
        RUNTIME_CLASS(CDeasm6502View),
        false);

    AddDocTemplate(pDocTemplate);
    m_pDocDeasmTemplate = pDocTemplate;

    // create main MDI Frame window
    CMainFrame *pMainFrame = new CMainFrame;
    if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
        return false;
    m_pMainWnd = pMainFrame;

    // Enable drag/drop open
    m_pMainWnd->DragAcceptFiles();

    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(true);

    CXMultiDocTemplate::s_bRegistrationExt = false;

    // Parse command line for standard shell commands, DDE, file open

    if (!m_bFileNew && cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
    // Dispatch commands specified on the command line
    if (!ProcessShellCommand(cmdInfo))
        return false;

    if (m_bMaximize && m_nCmdShow == SW_SHOWNORMAL)
        m_nCmdShow = SW_MAXIMIZE;

    // The main window has been initialized, so show and update it.
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();
    pMainFrame->SetFocus(); // we restore the focus - instead of the DialBar, to the main window

    return true;
}

#endif /* 0 */

/*************************************************************************/

bool C6502App::OnExceptionInMainLoop()
{
    std::string error;
    bool terminate = true;

    try
    {
        throw;
    }
    catch (const FileError &ex)
    {
        error = ex.what();
        terminate = false;
    }
    catch (const std::exception &ex)
    {
        error = ex.what();
    }
    catch (...)
    {
        error = _("UNKNOWN ERROR");
    }

    wxString errMsg = _("ERROR: ") + error;

    if (terminate)
        errMsg += _("\r\n\r\nThe application will now terminate.");

    wxLogError(errMsg);

    return !terminate;
}

void C6502App::AddToRecentFileList(const std::string &pathName)
{
    UNUSED(pathName);

    if (!m_bDoNotAddToRecentFileList)
        return;

    //CWinApp::AddToRecentFileList(pathName);
}

wxFrame *C6502App::CreateChildFrame(wxView *view)
{
    auto doc = view->GetDocument();

#if wxUSE_MDI_ARCHITECTURE
    return new wxDocMDIChildFrame(
        doc,
        view,
        static_cast<wxDocMDIParentFrame *>(wxGetApp().GetTopWindow()),
        wxID_ANY,
        doc->GetFilename(),
        wxDefaultPosition,
        wxSize(300, 300)
    );
#else
    return new wxDocChildFrameAny<wxAuiMDIChildFrame, wxAuiMDIParentFrame>(
        doc,
        view,
        static_cast<wxAuiMDIParentFrame *>(wxGetApp().GetTopWindow()),
        wxID_ANY,
        doc->GetFilename(),
        wxDefaultPosition,
        wxSize(300, 300)
    );
#endif
}

/*************************************************************************/

int C6502App::OnExit()
{
    wxLogDebug("Application shutting down");

    wxDocManager *const docManager = wxDocManager::GetDocumentManager();

    docManager->FileHistorySave(*m_config);

    //wxPoint pos = m_mainFrame->GetPosition();
    //wxSize size = m_mainFrame->GetSize();

    //m_config->Write("main_pos", pos);
    //m_config->Write("main_size", size);

    m_config->Flush();

    return wxApp::OnExit();
}

/*************************************************************************/
// C6502App Events

void C6502App::OnAppExit(wxCommandEvent &)
{
    Exit();
}

/*************************************************************************/
