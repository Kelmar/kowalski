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

#include "MainFrm.h"
#include "ChildFrm.h"
#include "6502View.h"
#include "6502Doc.h"
#include "Options.h"

#include "DebugController.h"
#include "AsmThread.h"

/*************************************************************************/

DebugController::DebugController(CMainFrame *mainFrame)
    : m_mainFrame(mainFrame)
    , m_critSect()
    , m_semaphore()
    , m_asmThread(nullptr)
{
    BindEvents();
}

DebugController::~DebugController()
{
}

/*************************************************************************/

PSym6502 DebugController::Simulator() const
{
    return wxGetApp().m_global.GetSimulator();
}

/*************************************************************************/

DebugState DebugController::CurrentState() const
{
    PSym6502 simulator = Simulator();

    if (!simulator)
        return DebugState::Unloaded;

    if (simulator->IsRunning())
        return DebugState::Running;

    if (simulator->IsFinished())
        return DebugState::Finished;

    if (simulator->IsBroken())
        return DebugState::Stopped;

    return DebugState::NotStarted;
}

/*************************************************************************/

void DebugController::BindEvents()
{
    // Menu handlers
    Bind(wxEVT_MENU, &DebugController::OnAssemble, this, evID_ASSEMBLE);
    Bind(wxEVT_MENU, &DebugController::OnRun, this, evID_RUN);
    Bind(wxEVT_MENU, &DebugController::OnStop, this, evID_STOP);
    Bind(wxEVT_MENU, &DebugController::OnStepOver, this, evID_STEP_OVER);

    Bind(wxEVT_MENU, &DebugController::OnOptions, this, evID_OPTIONS);

    // Update handlers
    Bind(wxEVT_UPDATE_UI, &DebugController::OnUpdateAssemble, this, evID_ASSEMBLE);
    Bind(wxEVT_UPDATE_UI, &DebugController::OnUpdateRun, this, evID_RUN);
    Bind(wxEVT_UPDATE_UI, &DebugController::OnUpdateStop, this, evID_STOP);
    Bind(wxEVT_UPDATE_UI, &DebugController::OnUpdateBreak, this, evID_BREAK);
    Bind(wxEVT_UPDATE_UI, &DebugController::OnUpdateStepOver, this, evID_STEP_OVER);

    // Thread event bindings
    Bind(wxEVT_THREAD, &DebugController::OnAsmComplete, this, evTHD_ASM_COMPLETE);
}

/*************************************************************************/

void DebugController::BuildMenu(wxMenuBar *menuBar)
{
    wxMenu *menu = new wxMenu();

    menu->Append(evID_ASSEMBLE, _("Assemble\tF7"));
    menu->AppendSeparator();
    menu->Append(evID_RUN, _("Run\tF5"));
    menu->Append(evID_STOP, _("Stop\tShift+F5"));
    menu->Append(evID_RESET, _("Reset\tCtrl+Shift+F5"));
    menu->Append(evID_BREAK, _("Break\tCtrl+Break"));
    menu->AppendSeparator();
    menu->Append(evID_STEP_INTO, _("Step Into"));
    menu->Append(evID_STEP_OVER, _("Step Over"));
    menu->Append(evID_STEP_OUT, _("Run Till Return"));
    menu->Append(evID_RUN_TO, _("Run to Cursor"));

    menu->AppendSeparator();
    menu->Append(evID_OPTIONS, _("Options\tCtrl+E"));

    /*
    auto intMenu = new wxMenu();

    intMenu->Append(0, _("Maskable (IRQ)\tCtrl+Shift+I"));
    intMenu->Append(0, _("Nonmaskable (NMI)\tCtrl+Shift+N"));
    intMenu->Append(0, _("Reset (RST)\tCtrl+Shift+R"));
    intMenu->AppendSeparator();
    intMenu->Append(0, _("Generator...\tCtrl+Shift+G"));

    menu->AppendSubMenu(intMenu, _("Interrupts"));
    */

    /*
    menu->AppendSeparator();
    menu->Append(0, _("Skip Instruction\tShift+F11"));
    menu->Append(0, _("Skip to Cursor\tCtrl+Shift+F11"));
    menu->AppendSeparator();
    menu->Append(0, _("Breakpoint\tF9"));
    menu->Append(0, _("Breakpoint Params...\tAlt+F9"));
    menu->AppendSeparator();
    menu->Append(0, _("Options...\tAlt+E"));
    */

    menuBar->Append(menu, _("Simulator"));
}

/*************************************************************************/

void DebugController::Run()
{
    if (CurrentState() == DebugState::Unloaded)
        StartDebug();

    Simulator()->Run();
    m_mainFrame->UpdateFlea();
}

/*************************************************************************/

void DebugController::Restart()
{
    StartDebug();
    Simulator()->Run();
    m_mainFrame->UpdateFlea();
}

/*************************************************************************/

void DebugController::Break()
{
    if (CurrentState() == DebugState::Running)
        return;

    Simulator()->Break();
    m_mainFrame->UpdateFlea();

#if 0
    // Likely this will come from the simulator sending an event.

    m_view->DelayedUpdateAll();

    // If the I/O window has focus, set it to the to main window.
    if (m_view->ioWindow()->HasFocus())
        m_view->SetFocus();
#endif
}

/*************************************************************************/

void DebugController::StepOver()
{
    if (CurrentState() != DebugState::Stopped)
        return;

    Simulator()->StepOver();
    m_mainFrame->UpdateFlea();
}

/*************************************************************************/

void DebugController::StartDebug()
{
    wxGetApp().m_global.StartDebug();
}

/*************************************************************************/

void DebugController::ExitDebugMode()
{
    if (CurrentState() == DebugState::Running)
        Simulator()->AbortProg(); // Interrupt the running program

    DebugStopped();
    m_mainFrame->UpdateFlea();
}

/*************************************************************************/

void DebugController::DebugStopped()
{
    if (!IsDebugging())
    {
        wxBell();
        return;
    }

    wxGetApp().m_global.ExitDebugger();

    //m_view->StopIntGenerator();

    /*
      if (m_IOWindow.m_hWnd != 0)
        m_IOWindow.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
      if (m_wndRegisterBar.m_hWnd != 0)
        m_wndRegisterBar.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
      if (m_Idents.m_hWnd != 0)
        m_Idents.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
    */
}

/*************************************************************************/

bool DebugController::ConfirmStop(const wxString &msg)
{
    auto res = wxMessageBox(msg, _("Stop debugging?"), wxYES_NO | wxICON_QUESTION);
    return res == wxOK;
}

/*************************************************************************/
// Menu handlers
/*************************************************************************/

void DebugController::OnAssemble(wxCommandEvent &)
{
    if (m_asmThread)
        return; // Currently assembling code, wait for completion.

    CSrc6502View *pView = m_mainFrame->GetCurrentView();

    if (pView == nullptr)
        return;

    CSrc6502Doc *doc = pView->GetDocument();

    if (doc == nullptr)
        return;

    /*if (m_IOWindow.IsWaiting())
    {
        m_IOWindow.SetFocus();
        return;
    }*/

    //SendMessageToViews(WM_USER_REMOVE_ERR_MARK);

    switch (CurrentState())
    {
    case DebugState::Unloaded:
    case DebugState::NotStarted:
    case DebugState::Finished:
        break; // Don't prompt

    default:
        if (!ConfirmStop(_("Assemble stops running simulator.\nExit debugger to assemble the program?")))
            return;

        ExitDebugMode();

        break;
    }

    // TODO: Remove this, the assembler should be able to find the includes itself. -- B.Simonds (July 28, 2024)

    // before assembly start set current dir to the document directory,
    // so include directive will find included files
    //

    const std::string &path = doc->GetFilename().ToStdString();

    const std::string &dirName = file::getDirectory(path);

    if (!dirName.empty())
    {
        if (chdir(dirName.c_str()))
            throw FileError(FileError::CannotChangeDir);
    }

    wxCriticalSectionLocker enter(m_critSect);

    m_asmThread = new AsmThread(this, path);

    if (m_asmThread->Create() != wxTHREAD_NO_ERROR)
    {
        wxLogError("Can't create background thread!");
        delete m_asmThread;
        m_asmThread = nullptr;
        return;
    }

    if (m_asmThread->Run() != wxTHREAD_NO_ERROR)
    {
        wxLogError("Can't start background thread!");
        delete m_asmThread;
        m_asmThread = nullptr;
    }
    else
        wxLogStatus("Assembler started");
}

/*************************************************************************/

void DebugController::OnRun(wxCommandEvent &)
{
    Run();
}

void DebugController::OnStop(wxCommandEvent &)
{
    ExitDebugMode();
}

/*************************************************************************/

void DebugController::OnBreak(wxCommandEvent &)
{
    Break();
}

/*************************************************************************/

void DebugController::OnStepOver(wxCommandEvent &)
{
    StepOver();
}

/*************************************************************************/

void DebugController::OnOptions(wxCommandEvent &)
{
    auto options = std::unique_ptr<COptions>(new COptions(m_mainFrame));

    if (options->ShowModal() != wxID_OK)
        return;

    wxLogDebug("Save global options here.");
}

/*************************************************************************/
// Update handlers
/*************************************************************************/

void DebugController::OnUpdateAssemble(wxUpdateUIEvent &e)
{
    bool enabled = m_asmThread == nullptr; // Enabled only if we're not currently assembling code.

    CSrc6502View *pView = m_mainFrame->GetCurrentView();

    enabled &= pView != nullptr;

    if (pView != nullptr)
    {
        CSrc6502Doc *doc = pView->GetDocument();
        enabled &= doc != nullptr;
    }

    e.Enable(enabled);
}

/*************************************************************************/

void DebugController::OnUpdateRun(wxUpdateUIEvent &e)
{
    wxString title = IsDebugging() ? _("Continue\tF5") : _("Run\tF5");

    e.SetText(title);
    e.Enable(CurrentState() != DebugState::Running);
}

/*************************************************************************/

void DebugController::OnUpdateStop(wxUpdateUIEvent &e)
{
    auto state = CurrentState();
    wxLogDebug(fmt::format("CurrentState(): {0}", (int)state).c_str());

    e.Enable(state == DebugState::Running);
}

/*************************************************************************/

void DebugController::OnUpdateBreak(wxUpdateUIEvent &e)
{
    e.Enable(CurrentState() == DebugState::Running);
}

/*************************************************************************/

void DebugController::OnUpdateStepOver(wxUpdateUIEvent &e)
{
    e.Enable(CurrentState() == DebugState::Stopped);
}

/*************************************************************************/
// Thread events
/*************************************************************************/

void DebugController::OnAsmComplete(wxThreadEvent &)
{
    wxLogDebug("OnAsmComplete() enter");

    wxCriticalSectionLocker enter(m_critSect);

    if (!m_asmThread)
        return;

    wxThread::ExitCode exit = m_asmThread->Wait();

    delete m_asmThread;
    m_asmThread = nullptr;

    if (exit != nullptr)
    {
        wxLogStatus("Assembly failed");
        m_mainFrame->console()->AppendText("Error from assembler!\r\n");
    }
    else
    {
        wxLogStatus("Assembly completed");
        m_mainFrame->console()->AppendText("Assemble OK\r\n");
    }
}

/*************************************************************************/

