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

#include "Splash.h"
#include "MainFrm.h"

#include "6502Doc.h"
#include "6502View.h"

#include "Events.h"

#include "formats/AtariBin.h"
#include "formats/Code65p.h"
#include "formats/MotorolaSRecord.h"
#include "formats/RawBin.h"

/*************************************************************************/

const char VENDOR_NAME[] = "MikSoft";
const char APP_NAME[] = "6502_simulator";

/*************************************************************************/

C6502App::C6502App()
    : m_optionsController(nullptr)
    , m_fontController(nullptr)
    , m_projectManager(nullptr)
    , m_debugController(nullptr)
    , m_mainFrame(nullptr)
    , m_config(nullptr)
    , m_bDoNotAddToRecentFileList(false)
{
}

C6502App::~C6502App()
{
    DisposeSingletons();
}

wxIMPLEMENT_APP(C6502App);

/*************************************************************************/

bool C6502App::OnInit()
{
    wxLog::EnableLogging();

    wxLogStatus("Application initializing");

    if (!wxApp::OnInit())
        return false;

    SetVendorName(VENDOR_NAME);
    SetAppName(APP_NAME);
    SetAppDisplayName(_("6502 Simulator"));

    m_splash = std::make_unique<SplashController>();

    UpdateStatus(_("Loading resources..."));

    LoadResources();

    UpdateStatus(_("Loading character encodings..."));
    LoadEncodings();

    UpdateStatus(_("Initializing controllers..."));

    wxDocManager *docManager = new wxDocManager();

    new wxDocTemplate(docManager,
        "Assembly", "*.s;*.asm", "", "s;asm ",
        "Assembly Source", "Assembly View",
        CLASSINFO(CSrc6502Doc), CLASSINFO(CSrc6502View));

    new wxDocTemplate(docManager,
        "Any", "*.*", "", "",
        "Any File", "Assembly View",
        CLASSINFO(CSrc6502Doc), CLASSINFO(CSrc6502View));

    SetupSingletons();

    InitOptionPages();

    UpdateStatus(_("Initializing file formats..."));

    InitBinaryTemplates();

    UpdateStatus(_("Setting up primary window..."));

    if (!InitFrame())
        return false;

    // File history has to be loaded after the frame is created.
    docManager->FileHistoryLoad(*m_config);
    m_splash->Refresh();
    wxYield();

    UpdateStatus(_("Application loaded!"));

    m_splash->Done([this] ()
    {
        m_mainFrame->Show();
        m_splash.reset(nullptr);
    });

    return true;
}

/*************************************************************************/

void C6502App::SetupSingletons()
{
    m_config = new wxConfig();

    /*
     * The options controller needs to be created first so the other
     * controllers have a chance to register their option handlers with it.
     */
    m_optionsController = new OptionsController();

    m_fontController = new FontController();
    m_projectManager = new ProjectManager();
    m_debugController = new DebugController();
}

/*************************************************************************/

void C6502App::DisposeSingletons()
{
    delete m_debugController;
    delete m_projectManager;
    delete m_fontController;

    delete m_optionsController;
    delete m_config;
}

/*************************************************************************/

void C6502App::InitOptionPages()
{
    m_fontController->InitOptions();
    m_debugController->InitOptions();
    //m_projectManager->InitOptions();
}

/*************************************************************************/

void C6502App::InitBinaryTemplates()
{
    // Hard coded for now.

    //AddTemplate<CMotorolaSRecord>();
    m_projectManager->AddTemplate<CAtariBin>();
    m_projectManager->AddTemplate<CCode65p>();

    // Keep RawBin last so it shows up as the last option in the drop down.
    m_projectManager->AddTemplate<CRawBin>();
}

/*************************************************************************/

bool C6502App::LoadResources()
{
    wxXmlResource::Get()->InitAllHandlers();
    wxLogDebug("Loading resource files....");

    wxPathList dirs = GetResourcePaths();
    wxString file = dirs.FindAbsoluteValidPath("res6502.xrc");

    bool result = wxXmlResource::Get()->Load(file);

    if (!result)
    {
        wxLogError("Unable to load resources!");
        wxMessageBox(
            _("Unable to load resources!"),
            _("6502 Simulator - Fatal Error"),
            wxCENTER | wxOK | wxICON_ERROR);

        return false;
    }

    return true;
}

/*************************************************************************/

void C6502App::LoadEncodings()
{
    //new encodings::CodePage437();
}

/*************************************************************************/

bool C6502App::InitFrame()
{
    wxDocManager *docManager = wxDocManager::GetDocumentManager();

    m_mainFrame = new CMainFrame(docManager);

    m_logFrame = new wxLogWindow(m_mainFrame, _("Log"), false);
    wxLog::SetActiveTarget(m_logFrame);

    return true;
}

/*************************************************************************/

// This is always the first to handle an event!
int C6502App::FilterEvent(wxEvent &event)
{
    //if (event.GetId() == evTHD_ASM_COMPLETE)
    //{
    //    wxLogDebug("Got thread event!");
    //}

    return wxApp::FilterEvent(event);
}

/*************************************************************************/

wxPathList C6502App::GetResourcePaths() const
{
    auto paths = wxStandardPaths::Get();

    wxFileName exe(paths.GetExecutablePath());
    wxPathList dirs;

    //dirs.Add(paths.GetLocalizedResourcesDir());
    dirs.Add(paths.GetResourcesDir());
    dirs.Add(paths.GetDataDir());
    dirs.Add(exe.GetPath());

    return dirs;
}

/*************************************************************************/

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

/*************************************************************************/

void C6502App::AddToRecentFileList(const std::string &pathName)
{
    UNUSED(pathName);

    if (!m_bDoNotAddToRecentFileList)
        return;

    //CWinApp::AddToRecentFileList(pathName);
}

/*************************************************************************/

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

    m_global.ExitDebugger();

    wxDocManager *const docManager = wxDocManager::GetDocumentManager();

    docManager->FileHistorySave(*m_config);

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

void C6502App::InnerUpdateStatus(const std::string &msg)
{
    wxLogStatus(msg.c_str());

    if (m_splash)
    {
        m_splash->SetStatus(msg);
        m_splash->Refresh();
        wxYield();
    }
}

/*************************************************************************/
