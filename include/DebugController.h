/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Several useful debugging functions/annotations for code.
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

#ifndef DEBUG_CONTROLLER_6502_H__
#define DEBUG_CONTROLLER_6502_H__

/*************************************************************************/
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

/*************************************************************************/

class DebugController : public wxEvtHandler
{
private:
    CMainFrame *m_view;
    wxMenuItem *m_menu;

    wxCriticalSection m_critSect;
    wxSemaphore m_semaphore;
    class AsmThread *m_asmThread;

    PSym6502 m_simulator;

    sigslot::connection m_simConn;

    void StartDebug();

    void ExitDebugMode();

    void DebugStopped();

    bool ConfirmStop(const wxString &msg);

public:
    /* constructor */ DebugController(CMainFrame *view);
    virtual          ~DebugController();

public: // Properties

    PSym6502 Simulator() const;

    DebugState CurrentState() const;

    inline
    bool IsDebugging() const
    {
        DebugState state = CurrentState();

        return
            (state == DebugState::Running) ||
            (state == DebugState::Stopped);
    }

public: // Commands
    void BuildMenu(wxMenuBar *);

    void Run();
    void Restart();
    void Break();
    void StepOver();

private:
    void BindEvents();

    // Menu handlers
    void OnAssemble(wxCommandEvent &);
    void OnRun(wxCommandEvent &);
    void OnStop(wxCommandEvent &);
    void OnBreak(wxCommandEvent &);
    void OnStepOver(wxCommandEvent &);

    // Update handlers
    void OnUpdateRun(wxUpdateUIEvent &);
    void OnUpdateStop(wxUpdateUIEvent &);
    void OnUpdateBreak(wxUpdateUIEvent &);
    void OnUpdateStepOver(wxUpdateUIEvent &);

    // Thread handlers
    void OnAsmComplete(wxThreadEvent &);

    void OnSimUpdate();
};

/*************************************************************************/

#endif /* DEBUG_CONTROLLER_6502_H__ */

/*************************************************************************/
