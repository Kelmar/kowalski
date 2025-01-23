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

#ifndef SIMULATOR_CONTROLLER_6502_H__
#define SIMULATOR_CONTROLLER_6502_H__

/*=======================================================================*/

#include "sim.h"

#include "DebugInfo.h"

/*=======================================================================*/
/**
 * @brief Indicates the current state of the debugger.
 */
enum class DebugState
{
    /// Simulator is currently not loaded
    Unloaded = 0,

    /// Simulator has been loaded but has not yet been started.
    NotStarted = 1,

    /// The simulator is currently running.
    Running = 2,

    /// The simulator is suspended.
    Stopped = 3,

    /// The simulator has reached an "end of program" marker.
    Finished = 4
};

/*=======================================================================*/

class SimulatorController : public wxEvtHandler
{
private:
    friend class C6502App;

    // Temporary until we remove CGlobal
    friend class CGlobal;

private:
    

private: // Simulator items
    /// Set when we're currently assembling code.
    std::atomic_bool m_assembling;

    wxCriticalSection m_critSect;
    wxSemaphore m_semaphore;
    class AsmThread *m_asmThread;

    // startup information for the simulator
    CDebugInfo m_debugInfo;

    PSym6502 m_simulator;

    /**
     * @brief Factory method for creating a new simulator instance
     * @remark If an instance already exists, it is replaced with a new one.
     */
    void CreateSimulator();

    // Temporary until removal of CGlobal
    void SetIOAddress(sim_addr_t addr);

private: // UI items
    wxMenuItem *m_menu;

private:
    /* constructor */ SimulatorController();

    void StartDebug();

    void DebugStopped();

    bool ConfirmStop(const wxString &msg);

    sim_addr_t GetCursorAddress(bool skipping);

public:
    virtual ~SimulatorController();

public: // Properties

    PSym6502 Simulator() const { return m_simulator; }

    DebugState CurrentState() const;

    CDebugInfo *DebugInfo() { return &m_debugInfo; }

    inline bool IsLoaded() const { return CurrentState() != DebugState::Unloaded; }

    inline
    bool IsDebugging() const
    {
        DebugState state = CurrentState();

        return
            (state == DebugState::Running) ||
            (state == DebugState::Stopped);
    }

    const SimulatorConfig &GetConfig() const;

    void SaveConfig();

public: // Commands
    void InitOptions();
    void BuildMenu(wxMenuBar *);

    void Run();
    void RunToAddress(sim_addr_t address);
    void Restart();
    void Break();
    void ExitDebugMode(bool uiEvent = false);

    void StepOver();
    void StepInto();
    void StepOut();

    void SkipInstruction();
    void SkipToAddress(sim_addr_t address);

private:
    void BindEvents();

    // View Menu handlers
    void OnViewDisassembler(wxCommandEvent &);

    // Simulator Menu handlers
    void OnAssemble(wxCommandEvent &);
    void OnRun(wxCommandEvent &);
    void OnRunToCursor(wxCommandEvent &);
    void OnRestart(wxCommandEvent &);
    void OnStop(wxCommandEvent &);
    void OnBreak(wxCommandEvent &);
    void OnStepOver(wxCommandEvent &);
    void OnStepInto(wxCommandEvent &);
    void OnStepOut(wxCommandEvent &);
    void OnSkipInstruction(wxCommandEvent &);
    void OnSkipToCursor(wxCommandEvent &);

    // Update handlers
    void OnUpdateAssemble(wxUpdateUIEvent &);
    void OnUpdateRun(wxUpdateUIEvent &);
    void OnUpdateRestart(wxUpdateUIEvent &);

    void EnableWhenRunning(wxUpdateUIEvent &);
    void EnableWhenStopped(wxUpdateUIEvent &);

    // Thread handlers
    void OnAsmComplete(wxThreadEvent &);
};

/*=======================================================================*/

#endif /* SIMULATOR_CONTROLLER_6502_H__ */

/*=======================================================================*/
