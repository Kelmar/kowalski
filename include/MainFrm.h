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

#ifndef MAIN_FRM_H__
#define MAIN_FRM_H__

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "DialBar.h"
#include "ToolBox.h"
#include "RegisterBar.h"
#include "IdentInfo.h"
#include "MemoryInfo.h"
#include "IOWindow.h"
#include "ConfigSettings.h"
#include "Broadcast.h"
#include "FlatBar.h"
#include "DynamicHelp.h"

class CMainFrame;
class CSrc6502Doc;
class CSrc6502View;

#include "ConsoleFrm.h"
#include "AsmThread.h"

#include "mdi65.h"

class CMainFrame : public MAIN_BASE
{
private:
    static constexpr char REG_ENTRY_MAINFRM[] = "MainFrame";

    ConsoleFrame *m_output;
    wxAuiPaneInfo m_outputInfo;

    wxAuiManager m_auiManager;
    wxDocManager *m_docManager;

    wxCriticalSection m_critSect;

    wxSemaphore m_semaphore;

    AsmThread* m_asmThread;

    // TODO: Move into a project document
    static std::string ProjName;
    
    //static LRESULT CALLBACK StatusBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    //static wxBitmap m_bmpCode;
    //static wxBitmap m_bmpDebug;

    std::string m_strFormat; // Formatting characters for paragraph. row/column in the status bar
    UINT m_uTimer;

    //afx_msg LRESULT OnUpdateState(WPARAM wParam, LPARAM lParam);

    int RedrawAllViews(int chgHint = 0);
    int Options(int page);
    int m_nLastPage;
    void ConfigSettings(bool load);
    void ExitDebugMode();

    // Control setup
    void InitMenu();
    void BindEvents();

public:
    /* constructor */ CMainFrame(wxDocManager *docManager);
    virtual          ~CMainFrame();

//  virtual HMENU GetWindowMenuPopup(HMENU hMenuBar);

//  void ShowRegisterBar(bool bShow = TRUE);
    void SetPositionText(int row, int col);
//  void SetRowColumn(CEdit &edit);
    CSrc6502View *GetCurrentView();
    CSrc6502Doc *GetCurrentDocument();

public:
    CRegisterBar m_wndRegisterBar;
    CIOWindow m_IOWindow;
    CMemoryInfo m_Memory;
    CMemoryInfo m_ZeroPage;
    CMemoryInfo m_Stack;
    CIdentInfo m_Idents;
    CDynamicHelp m_wndHelpBar;

    void UpdateAll();
    void DelayedUpdateAll();

    void ShowDynamicHelp(const std::string& strLine, int nWordStart, int nWordEnd);

#if 0
    virtual bool PreCreateWindow(CREATESTRUCT& cs);
    virtual bool OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
#endif

protected: // control bar embedded members
    //CDialBar m_wndZeroPageBar;
    //CToolBar m_wndToolBar;
    CFlatToolBar m_wndToolBar;

    void SymGenInterrupt(CSym6502::IntType eInt);
    void UpdateSymGenInterrupt(CCmdUI* pCmdUI);
    void StopIntGenerator();
    void StartIntGenerator();

// Generated message map functions
protected:
    afx_msg void OnClose();
    
    afx_msg void OnUpdateAssemble(CCmdUI* pCmdUI);
    afx_msg void OnUpdateSymDebug(CCmdUI* pCmdUI);
    afx_msg void OnSymDebug();
    afx_msg void OnSymStepInto();
    afx_msg void OnUpdateSymStepInto(CCmdUI* pCmdUI);
    afx_msg void OnSymSkipInstr();
    afx_msg void OnUpdateSymSkipInstr(CCmdUI* pCmdUI);
    afx_msg void OnSymBreakpoint();
    afx_msg void OnUpdateSymBreakpoint(CCmdUI* pCmdUI);
    afx_msg void OnSymBreak();
    afx_msg void OnUpdateSymBreak(CCmdUI* pCmdUI);
    afx_msg void OnSymGo();
    afx_msg void OnUpdateSymGo(CCmdUI* pCmdUI);
    afx_msg void OnOptions();
    afx_msg void OnUpdateOptions(CCmdUI* pCmdUI);
    afx_msg void OnSymGoToLine();
    afx_msg void OnUpdateSymGoToLine(CCmdUI* pCmdUI);
    afx_msg void OnSymSkipToLine();
    afx_msg void OnUpdateSymSkipToLine(CCmdUI* pCmdUI);
    afx_msg void OnSymGoToRts();
    afx_msg void OnUpdateSymGoToRts(CCmdUI* pCmdUI);
    afx_msg void OnSymStepOver();
    afx_msg void OnUpdateSymStepOver(CCmdUI* pCmdUI);
    afx_msg void OnSymEditBreakpoint();
    afx_msg void OnUpdateSymEditBreakpoint(CCmdUI* pCmdUI);
    afx_msg void OnSymRestart();
    afx_msg void OnUpdateSymRestart(CCmdUI* pCmdUI);
    afx_msg void OnSymAnimate();
    afx_msg void OnUpdateSymAnimate(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIdViewRegisterbar(CCmdUI* pCmdUI);
    afx_msg void OnFileSaveCode();
    afx_msg void OnUpdateFileSaveCode(CCmdUI* pCmdUI);
    afx_msg void OnViewDeasm();
    afx_msg void OnUpdateViewDeasm(CCmdUI* pCmdUI);
    afx_msg void OnViewIdents();
    afx_msg void OnUpdateViewIdents(CCmdUI* pCmdUI);
    afx_msg void OnViewMemory();
    afx_msg void OnUpdateViewMemory(CCmdUI* pCmdUI);
    afx_msg void OnEditorOpt();
    afx_msg void OnUpdateEditorOpt(CCmdUI* pCmdUI);
    afx_msg void OnViewIOWindow();
    afx_msg void OnUpdateViewIOWindow(CCmdUI* pCmdUI);
    afx_msg void OnDestroy();
    afx_msg void OnFileLoadCode();
    afx_msg void OnUpdateFileLoadCode(CCmdUI* pCmdUI);
    afx_msg void OnDeasmOptions();
    afx_msg void OnUpdateDeasmOptions(CCmdUI* pCmdUI);
    afx_msg void OnViewRegisterWnd();
    afx_msg void OnSysColorChange();
    afx_msg void OnUpdateViewZeropage(CCmdUI* pCmdUI);
    afx_msg void OnViewZeropage();
    afx_msg void OnUpdateMemoryOptions(CCmdUI* pCmdUI);
    afx_msg void OnMemoryOptions();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnViewStack();
    afx_msg void OnUpdateViewStack(CCmdUI* pCmdUI);
    afx_msg void OnSymGenIRQ();
    afx_msg void OnUpdateSymGenIRG(CCmdUI* pCmdUI);
    afx_msg void OnSymGenNMI();
    afx_msg void OnUpdateSymGenNMI(CCmdUI* pCmdUI);
    afx_msg void OnSymGenReset();
    afx_msg void OnUpdateSymGenReset(CCmdUI* pCmdUI);
    afx_msg void OnHtmlHelp();  //% Bug fix 1.2.14.1 - convert to HTML help
    afx_msg void OnSymGenIntDlg();
    afx_msg void OnUpdateSymGenIntDlg(CCmdUI* pCmdUI);
    afx_msg void OnHelpDynamic();
    afx_msg void OnUpdateHelpDynamic(CCmdUI* pCmdUI);

    afx_msg void OnSymDebugStop();
    afx_msg LRESULT OnStartDebugger(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitDebugger(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnChangeCode(WPARAM wParam, LPARAM lParam);

private: // Event handlers
    void OnExit(wxCommandEvent &);
    void OnShowOutput(wxCommandEvent &);
    void OnShowLog(wxCommandEvent &);
    void OnShowTest(wxCommandEvent &);

    void OnAbout(wxCommandEvent &);

    void OnAssemble(wxCommandEvent &);
    
private: // UI Updaters
    void OnUpdateShowLog(wxUpdateUIEvent &);

private: // Thread events
    void OnAsmComplete(wxThreadEvent &);
    
private:

    void EnableDockingEx(uint32_t dwDockStyle);
    static const uint32_t dwDockBarMapEx[4][2];

    void AddBreakpoint(CSrc6502View* pView, int nLine, CAsm::Breakpoint bp);
    void RemoveBreakpoint(CSrc6502View* pView, int nLine);
};

/////////////////////////////////////////////////////////////////////////////

#endif /* MAIN_FRM_H__ */