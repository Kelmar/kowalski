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

#include "6502View.h"
#include "6502Doc.h"

#include "DisassemblyFrame.h"
#include "RegisterView.h"

/*=======================================================================*/

#include "Options.h"

#include "sim.h"
#include "sim_priv.h"

#include "options/OptionsSymPage.h"

#include "SimulatorController.h"
#include "AsmThread.h"

/*=======================================================================*/
// Configuration
/*=======================================================================*/

// Directly exposed to OptionsSymPage
SimulatorConfig s_simConfig;

/*=======================================================================*/

namespace
{
    static const std::string SIM_CONFIG_PATH = "Simulator";
}

/*=======================================================================*/
/*=======================================================================*/

SimulatorController::SimulatorController()
    : m_assembling(false)
    , m_critSect()
    , m_semaphore()
    , m_asmThread(nullptr)
    , m_debugInfo()
    , m_simulator(nullptr)
    , m_menu(nullptr)
    , m_regView(nullptr)
{
    BindEvents();

    LoadConfig();
}

SimulatorController::~SimulatorController()
{
}

/*=======================================================================*/

void SimulatorController::Shutdown()
{
    ExitDebugMode();
    SaveConfig();
}

/*=======================================================================*/

const SimulatorConfig &SimulatorController::GetConfig() const { return s_simConfig; }

/*=======================================================================*/

void SimulatorController::SaveConfig()
{
    auto config = wxConfig::Get();
    wxString oldPath = config->GetPath();

    auto restorePath = deferred_action([&] () { config->SetPath(oldPath); });

    config->SetPath(SIM_CONFIG_PATH);

    config->Write("ProcType", (long)s_simConfig.Processor);
    config->Write("SimFinish", (long)s_simConfig.SimFinish);
    config->Write("IOEnabled", s_simConfig.IOEnable);
    config->Write("IOAddress", s_simConfig.IOAddress);
    config->Write("ProtectMem", s_simConfig.ProtectMemory);
    config->Write("ProtectMemFrom", s_simConfig.ProtectStart);
    config->Write("ProtectMemTo", s_simConfig.ProtectEnd);
}

/*=======================================================================*/

void SimulatorController::LoadDefaults()
{
    s_simConfig =
    {
        .Processor = ProcessorType::M6502,
        .SimFinish = CAsm::Finish::FIN_BY_BRK,
        .IOEnable = true,
        .IOAddress = 0xE000,
        .ProtectMemory = false,
        .ProtectStart = 0,
        .ProtectEnd = 0
    };
}

/*=======================================================================*/

void SimulatorController::LoadConfig()
{
    LoadDefaults();

#if WIN32
    // TODO: Import old config if detected.
#endif

    auto config = wxConfig::Get();
    wxString oldPath = config->GetPath();

    auto restorePath = deferred_action([&] () { config->SetPath(oldPath); });

    config->SetPath(SIM_CONFIG_PATH);

    s_simConfig.Processor = (ProcessorType)config->ReadLong("ProcType", (long)s_simConfig.Processor);
    s_simConfig.SimFinish = (CAsm::Finish)config->ReadLong("SimFinish", (long)s_simConfig.SimFinish);
    s_simConfig.IOEnable = config->ReadBool("IOEnabled", s_simConfig.IOEnable);
    s_simConfig.IOAddress = config->ReadLong("IOAddress", s_simConfig.IOAddress);
    s_simConfig.ProtectMemory = config->ReadBool("ProtecteMem", s_simConfig.ProtectMemory);
    s_simConfig.ProtectStart = config->ReadLong("ProtectMemFrom", s_simConfig.ProtectStart);
    s_simConfig.ProtectEnd = config->ReadLong("ProtectMemTo", s_simConfig.ProtectEnd);
}

/*=======================================================================*/

void SimulatorController::InitOptions()
{
    OptionsPageFactory factory = [] (wxBookCtrlBase *parent)
    {
        return new OptionsSymPage(parent);
    };

    wxGetApp().optionsController().RegisterPage(factory, _("Simulator"), 0);
}

/*=======================================================================*/

void SimulatorController::CreateSimulator()
{
    CGlobal *global = &wxGetApp().m_global;

    CContext ctx(GetConfig());

    // Build up device list
    // For now hard coded to a RAM and IOWindow module.

    COutputMem *memory = &global->GetMemory();

    if (s_simConfig.IOEnable)
    {
        sim_addr_t ioAddr = s_simConfig.IOAddress;

        sim::PDevice lowRam(new sim::dev::RAM(memory, 0, ioAddr));

        const CMainFrame *mainFrame = wxGetApp().mainFrame();
        sim::PDevice simpleIO(new sim::dev::SimpleIO(mainFrame->ioWindow()));

        sim_addr_t hiRamStart = ioAddr + simpleIO->AddressSize();
        size_t hiRamSize = (ctx.bus.maxAddress() - hiRamStart) + 1;

        sim::PDevice hiRam(new sim::dev::RAM(memory, hiRamStart, hiRamSize));

        ctx.bus.AddDevice(lowRam, 0);
        ctx.bus.AddDevice(simpleIO, ioAddr);
        ctx.bus.AddDevice(hiRam, hiRamStart);
    }
    else
    {
        sim::PDevice lowRam(new sim::dev::RAM(memory, 0, ctx.bus.maxAddress()));
    }

    //io::output &out = wxGetApp().mainFrame()->console()->GetOutput("simulator");

    // Get a fresh new simulator
    m_simulator = std::make_shared<CSym6502>(std::move(ctx), global->m_startAddress, &m_debugInfo);
}

/*=======================================================================*/
// TODO: Remove after removal of CGlobal
void SimulatorController::SetIOAddress(sim_addr_t addr)
{
    s_simConfig.IOAddress = addr;
}

/*=======================================================================*/

DebugState SimulatorController::CurrentState() const
{
    if (!m_simulator)
        return DebugState::Unloaded;

    if (m_simulator->IsRunning())
        return DebugState::Running;

    if (m_simulator->IsFinished())
        return DebugState::Finished;

    if (m_simulator->IsBroken())
        return DebugState::Stopped;

    return DebugState::NotStarted;
}

/*=======================================================================*/

void SimulatorController::BindEvents()
{
    // View menu handlers
    Bind(wxEVT_MENU, &SimulatorController::OnViewDisassembler, this, evID_SHOW_DISASM);
    Bind(wxEVT_MENU, &SimulatorController::OnViewRegisters, this, evID_SHOW_REGS);

    // View update handlers
    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenDebugging, this, evID_SHOW_DISASM);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::OnUpdateShowRegs, this, evID_SHOW_REGS);

    // Simulator menu handlers
    Bind(wxEVT_MENU, &SimulatorController::OnAssemble, this, evID_ASSEMBLE);
    Bind(wxEVT_MENU, &SimulatorController::OnRun, this, evID_RUN);
    Bind(wxEVT_MENU, &SimulatorController::OnRestart, this, evID_RESTART);
    Bind(wxEVT_MENU, &SimulatorController::OnStop, this, evID_STOP);

    Bind(wxEVT_MENU, &SimulatorController::OnStepInto, this, evID_STEP_INTO);
    Bind(wxEVT_MENU, &SimulatorController::OnStepOver, this, evID_STEP_OVER);
    Bind(wxEVT_MENU, &SimulatorController::OnStepOut, this, evID_STEP_OUT);

    Bind(wxEVT_MENU, &SimulatorController::OnRunToCursor, this, evID_RUN_TO);

    Bind(wxEVT_MENU, &SimulatorController::OnSkipInstruction, this, evID_SKIP_INSTR);
    Bind(wxEVT_MENU, &SimulatorController::OnSkipToCursor, this, evID_SKIP_TO_LINE);

    // Simulator update handlers
    Bind(wxEVT_UPDATE_UI, &SimulatorController::OnUpdateAssemble, this, evID_ASSEMBLE);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::OnUpdateRun, this, evID_RUN);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::OnUpdateRestart, this, evID_RESTART);

    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenRunning, this, evID_STOP);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenRunning, this, evID_BREAK);

    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenStopped, this, evID_STEP_INTO);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenStopped, this, evID_STEP_OVER);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenStopped, this, evID_STEP_OUT);

    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenStopped, this, evID_RUN_TO);

    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenStopped, this, evID_SKIP_INSTR);
    Bind(wxEVT_UPDATE_UI, &SimulatorController::EnableWhenStopped, this, evID_SKIP_TO_LINE);

    // Thread event bindings
    Bind(wxEVT_THREAD, &SimulatorController::OnAsmComplete, this, evTHD_ASM_COMPLETE);
}

/*=======================================================================*/

void SimulatorController::BuildMenu(wxMenuBar *menuBar)
{
    wxMenu *menu = new wxMenu();

    menu->Append(evID_ASSEMBLE, _("Assemble\tF7"));

    menu->AppendSeparator();
    menu->Append(evID_RUN, _("Run\tF5"));
    menu->Append(evID_STOP, _("Stop\tShift+F5"));
    menu->Append(evID_RESTART, _("Reset\tCtrl+Shift+F5"));
    menu->Append(evID_BREAK, _("Break\tCtrl+Break"));

    menu->AppendSeparator();
    menu->Append(evID_STEP_INTO, _("Step Into"));
    menu->Append(evID_STEP_OVER, _("Step Over"));
    menu->Append(evID_STEP_OUT, _("Run Till Return"));
    menu->Append(evID_RUN_TO, _("Run to Cursor"));

    /*
    auto intMenu = new wxMenu();

    intMenu->Append(0, _("Maskable (IRQ)\tCtrl+Shift+I"));
    intMenu->Append(0, _("Nonmaskable (NMI)\tCtrl+Shift+N"));
    intMenu->Append(0, _("Reset (RST)\tCtrl+Shift+R"));
    intMenu->AppendSeparator();
    intMenu->Append(0, _("Generator...\tCtrl+Shift+G"));

    menu->AppendSubMenu(intMenu, _("Interrupts"));
    */

    menu->AppendSeparator();
    menu->Append(evID_SKIP_INSTR, _("Skip Instruction\tShift+F11"));
    menu->Append(evID_SKIP_TO_LINE, _("Skip to Cursor\tCtrl+Shift+F11"));

    /*
    menu->AppendSeparator();
    menu->Append(0, _("Breakpoint\tF9"));
    menu->Append(0, _("Breakpoint Params...\tAlt+F9"));
    */

    menuBar->Append(menu, _("Simulator"));
}

/*=======================================================================*/

void SimulatorController::Assemble()
{
    wxCriticalSectionLocker enter(m_critSect);

    if (m_assembling)
        return; // Currently assembling code, wait for completion.

    CSrc6502View *pView = wxGetApp().mainFrame()->GetCurrentView();

    if (pView == nullptr)
        return;

    CSrc6502Doc *doc = pView->GetDocument();

    if (doc == nullptr)
        return;

    // Ensure the document is written to disk so the assembler can read the latest and greatest.
    doc->Save();

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

    m_assembling = true;
    auto asmOutput = wxGetApp().GetConsole(ASSEMBLER_CONSOLE);

    asmOutput->write(_("Staring assemlber...").ToStdString() + "\r\n");

    m_asmThread = new AsmThread(this, path, asmOutput);

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
        return;
    }
}

/*=======================================================================*/

void SimulatorController::Run()
{
    DebugState state = CurrentState();

    switch (state)
    {
    case DebugState::Running:
        return; // Shouldn't happen, don't do anything just incase!

    case DebugState::Unloaded:
    case DebugState::Finished:
        StartDebug();
        break;

    case DebugState::NotStarted:
    case DebugState::Stopped:
        break; // Continue where we left off.
    }

    m_simulator->Run();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::RunToAddress(sim_addr_t address)
{
    if (address == sim::INVALID_ADDRESS)
        return;

    m_debugInfo.SetTemporaryExecBreakpoint(address);
    m_simulator->Run(); // Run after setting a temporary breakpoint

    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::Restart()
{
    StartDebug();
    m_simulator->Run();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::Break()
{
    if (CurrentState() == DebugState::Running)
        return;

    m_simulator->Break();
    UpdateUI();

#if 0
    // Likely this will come from the simulator sending an event.

    m_view->DelayedUpdateAll();

    // If the I/O window has focus, set it to the to main window.
    if (m_view->ioWindow()->HasFocus())
        m_view->SetFocus();
#endif
}

/*=======================================================================*/

void SimulatorController::StepOver()
{
    if (CurrentState() != DebugState::Stopped)
        return;

    m_simulator->StepOver();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::StepInto()
{
    m_simulator->StepInto();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::StepOut()
{
    m_simulator->RunTillRet();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::SkipInstruction()
{
    m_simulator->SkipInstr();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::SkipToAddress(sim_addr_t address)
{
    if (address == sim::INVALID_ADDRESS)
        return;

    m_simulator->SkipToAddr(address);
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::StartDebug()
{
    CreateSimulator();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::ExitDebugMode(bool uiEvent)
{
    if (!IsDebugging())
    {
        if (uiEvent)
            wxBell();

        return;
    }

    if (CurrentState() == DebugState::Running)
        m_simulator->AbortProg(); // Interrupt the running program

    DebugStopped();
    UpdateUI();
}

/*=======================================================================*/

void SimulatorController::DebugStopped()
{
    m_simulator.reset();

    UpdateUI();

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

/*=======================================================================*/

bool SimulatorController::ConfirmStop(const wxString &msg)
{
    auto res = wxMessageBox(msg, _("Stop debugging?"), wxYES_NO | wxICON_QUESTION);
    return res == wxOK;
}

/*=======================================================================*/

sim_addr_t SimulatorController::GetCursorAddress(bool skipping)
{
    auto view = wxGetApp().mainFrame()->GetCurrentView();

    if (!view)
        return sim::INVALID_ADDRESS;

    std::string pathName = view->GetDocument()->GetFilename().ToStdString();

    int line = view->GetCurrLineNo();

    CAsm::FileUID fuid = m_debugInfo.GetFileUID(pathName);
    CDebugLine dl = m_debugInfo.GetLineInfo(fuid, line);

    CAsm::DbgFlag flag = (CAsm::DbgFlag)dl.flags;

    int msgResult = wxID_OK;

    if ((flag == CAsm::DBG_EMPTY) || ((flag & CAsm::DBG_MACRO) != 0))
    {
        wxString skipMessage = _("Current row doesn't contain machine code.\nSkip to cursor not possible.");
        wxString runMessage = _("Current row doesn't contain machine code.\nRun to cursor not possible.");

        // Line without result code.
        wxMessageBox(
            skipping ? skipMessage : runMessage,
            wxGetApp().GetAppDisplayName(),
            wxICON_ERROR
        );

        return sim::INVALID_ADDRESS;
    }
    else if ((flag & CAsm::DBG_DATA) != 0)
    {
        // Line with data instead of commands
        msgResult = wxMessageBox(
            _("Current row contains data, not code.\nContinue anyway?"),
            wxGetApp().GetAppDisplayName(),
            wxICON_ASTERISK | wxYES_NO
        );
    }

    if (msgResult != wxID_OK)
        return sim::INVALID_ADDRESS;

    return dl.addr;
}

/*=======================================================================*/

void SimulatorController::UpdateUI()
{
    // Need some sort of "perspective" manager.

    if (m_regView)
        m_regView->UpdateStatus();

    wxGetApp().mainFrame()->UpdateFlea();

    wxGetApp().mainFrame()->UpdateAll();
}

/*=======================================================================*/
// Menu handlers
/*=======================================================================*/

void SimulatorController::OnViewDisassembler(wxCommandEvent &)
{
    auto wnd = new DisassemblyFrame(wxGetApp().mainFrame());
    wnd->Show();
}

/*=======================================================================*/

void SimulatorController::OnViewRegisters(wxCommandEvent &)
{
    if (!m_regView)
        m_regView = new RegisterView(wxGetApp().mainFrame());

    m_regView->Show();
}

/*=======================================================================*/

void SimulatorController::OnAssemble(wxCommandEvent &)
{
    Assemble();
}

/*=======================================================================*/

void SimulatorController::OnRun(wxCommandEvent &)
{
    Run();
}

/*=======================================================================*/

void SimulatorController::OnRunToCursor(wxCommandEvent &)
{
    sim_addr_t address = GetCursorAddress(false);
    RunToAddress(address);
}

/*=======================================================================*/

void SimulatorController::OnRestart(wxCommandEvent &)
{
    Restart();
}

/*=======================================================================*/

void SimulatorController::OnStop(wxCommandEvent &)
{
    ExitDebugMode(true);
}

/*=======================================================================*/

void SimulatorController::OnBreak(wxCommandEvent &)
{
    Break();
}

/*=======================================================================*/

void SimulatorController::OnStepOver(wxCommandEvent &)
{
    StepOver();
}

/*=======================================================================*/

void SimulatorController::OnStepInto(wxCommandEvent &)
{
    StepInto();
}

/*=======================================================================*/

void SimulatorController::OnStepOut(wxCommandEvent &)
{
    StepOut();
}

/*=======================================================================*/

void SimulatorController::OnSkipInstruction(wxCommandEvent &)
{
    SkipInstruction();
}

/*=======================================================================*/

void SimulatorController::OnSkipToCursor(wxCommandEvent &)
{
    sim_addr_t address = GetCursorAddress(true);
    SkipToAddress(address);
}

/*=======================================================================*/
// Update handlers
/*=======================================================================*/

void SimulatorController::OnUpdateAssemble(wxUpdateUIEvent &e)
{
    bool enabled = m_asmThread == nullptr; // Enabled only if we're not currently assembling code.

    CSrc6502View *pView = wxGetApp().mainFrame()->GetCurrentView();

    enabled &= pView != nullptr;

    if (pView != nullptr)
    {
        CSrc6502Doc *doc = pView->GetDocument();
        enabled &= doc != nullptr;
    }

    e.Enable(enabled);
}

/*=======================================================================*/

void SimulatorController::OnUpdateRun(wxUpdateUIEvent &e)
{
    wxString title = IsDebugging() ? _("Continue\tF5") : _("Run\tF5");

    e.SetText(title);
    e.Enable(CurrentState() != DebugState::Running && !m_assembling);
}

/*=======================================================================*/

void SimulatorController::OnUpdateRestart(wxUpdateUIEvent &e)
{
    e.Enable(CurrentState() != DebugState::Unloaded && !m_assembling);
}

/*=======================================================================*/

void SimulatorController::OnUpdateShowRegs(wxUpdateUIEvent &e)
{
    if (IsDebugging())
    {
        e.Enable(true);
        e.Check(m_regView && m_regView->IsVisible());
    }
    else
    {
        e.Enable(false);
        e.Check(false);
    }
}

/*=======================================================================*/

void SimulatorController::EnableWhenDebugging(wxUpdateUIEvent &e)
{
    e.Enable(IsDebugging());
}

/*=======================================================================*/

void SimulatorController::EnableWhenRunning(wxUpdateUIEvent &e)
{
    e.Enable(CurrentState() == DebugState::Running && !m_assembling);
}

/*=======================================================================*/

void SimulatorController::EnableWhenStopped(wxUpdateUIEvent &e)
{
    e.Enable(CurrentState() == DebugState::Stopped && !m_assembling);
}

/*=======================================================================*/
// Thread events
/*=======================================================================*/

void SimulatorController::OnAsmComplete(wxThreadEvent &)
{
    wxCriticalSectionLocker enter(m_critSect);

    m_assembling = false;

    if (!m_asmThread)
        return;

    m_asmThread->Wait();

    delete m_asmThread;
    m_asmThread = nullptr;

    UpdateUI();
}

/*=======================================================================*/
