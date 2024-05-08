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

// DialAsmStat.cpp : implementation file
//

#include "StdAfx.h"

#include "M6502.h"

#include "MainFrm.h"
#include "DialAsmStat.h"
#include "Broadcast.h"
#include "6502View.h"
#include "6502Doc.h"

/////////////////////////////////////////////////////////////////////////////
// CDialAsmStat dialog

CDialAsmStat::CDialAsmStat(CSrc6502View *pView)
    : wxDialog() //pView)
    , m_pView(pView)
{
    ASSERT(pView != NULL);
    ASSERT(pView->GetDocument()); // Document must be attached

    m_strCtrlRow = "";
    m_strCtrlPassNo = "";

    m_nLines = m_pView->GetLineCount();
    m_nCurrLine = 0;
    m_nPassNo = 0;
    m_pAsm6502 = NULL;
}

CDialAsmStat::~CDialAsmStat()
{
    //DestroyWindow();
    delete m_pAsm6502;
}

wxControl *CDialAsmStat::FindControl(long id) const
{
    wxWindow *res = FindWindow(id);
    ASSERT(res); // Wrong object number in the dialog box
    return dynamic_cast<wxControl*>(res);
}

#if 0

void CDialAsmStat::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDialAsmStat)
    DDX_Text(pDX, IDC_CURR_ROW, m_strCtrlRow);
    DDX_Text(pDX, IDC_PASS_NO, m_strCtrlPassNo);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDialAsmStat, CDialog)
    //{{AFX_MSG_MAP(CDialAsmStat)
    ON_WM_SETCURSOR()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_USER_ABORT_ASM, OnAbortAsm)
    ON_MESSAGE(WM_USER_GET_NEXT_LINE, OnGetNextLine)
    ON_MESSAGE(WM_USER_GET_LINE_NO, OnGetLineNo)
    ON_MESSAGE(WM_USER_GET_TITLE, OnGetDocTitle)
    ON_MESSAGE(WM_USER_NEXT_PASS, OnNextPass)
    ON_MESSAGE(WM_USER_FIN, OnFinished)
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CDialAsmStat message handlers

void CDialAsmStat::SetValues(int row, int pass)
{
    m_strCtrlRow.Printf("%d", row);
    m_strCtrlPassNo.Printf("%d", pass);

    //UpdateData();

    //FindControl(IDC_CURR_ROW)->Refresh();
    //FindControl(IDC_PASS_NO)->Refresh();
}

bool CDialAsmStat::Create()
{
    //return CDialog::Create(IDD);
    return false;
}

bool CDialAsmStat::OnInitDialog()
{
#if 0
    CDialog::OnInitDialog();

    try
    {
        m_pView->GetText(m_strText);
        m_pText = m_strText;

        SetProgressRange(m_nLines);
        m_dwTimer= ::GetTickCount();

        m_stAsmRetCode = CAsm::OK;
        m_bFinished = FALSE;

        m_pAsm6502 = new CAsm6502(this, theApp.m_global.GetMemForAsm(),
                                  theApp.m_global.GetDebug(), theApp.m_global.GetMarkArea(),
                                  theApp.m_global.GetProcType(),
                                  theApp.m_global.m_bGenerateListing && !theApp.m_global.m_strListingFile.IsEmpty() ?
                                  (const TCHAR *)(theApp.m_global.m_strListingFile) : NULL);
        if (AfxBeginThread(CDialAsmStat::start_asm_thread,this) == NULL)
        {
            AfxMessageBox(IDS_ERR_ASM_THREAD);
            EndDialog(-1);
            return TRUE;
        }
    }
    catch (CException *ex)
    {
        ex->ReportError();
    }
#endif

    return true;
}

std::string CDialAsmStat::GetLine(int nLine)
{
#if 0

#ifdef USE_CRYSTAL_EDIT
    //TODO
    return "";
#else
    std::string buff;

    int chr_index= m_pView->GetEditCtrl().LineIndex(nLine);

    if (chr_index == -1)
        return buff;

    int len = m_pView->GetEditCtrl().LineLength(chr_index) + 1;

    char *ptr = buff.GetBuffer(len + 1); // Allocating space for 'len' characters in 'buff'

    m_pView->GetEditCtrl().GetLine(nLine, ptr, len);

    ptr[len - 1] = '\n'; // Include a line break character
    buff.ReleaseBuffer(len);

    return buff;
#endif

#endif

    return "";
}

void CDialAsmStat::GetLine(int nLine, char *buf, size_t max_len)
{
    if (m_pText == NULL)
    {
        buf[0] = 0;
        return;
    }

    const char *ptr = strchr(m_pText, 0xD);

    if (ptr >= m_pText)
    {
        size_t len = std::min((size_t)(ptr + 1 - m_pText), max_len - 1);
        memcpy(buf, m_pText, len * sizeof(char));
        buf[len] = 0;
        m_pText = ptr + 1 + 1;		// omini�cie 0D 0A
    }
    else // Last line
    {
        size_t len = std::min(strlen(m_pText), max_len - 1);
        memcpy(buf, m_pText, len * sizeof(char));
        buf[len++] = '\n';
        buf[len] = 0;
        m_pText = NULL;
    }

    /*
      int chr_index = m_edit.LineIndex(nLine);
      if (chr_index == -1)
      {
        *buf = '\0';
        return;
      }
      int len = m_edit.GetLine(nLine, buf, max_len);
      len = min(len, max_len);
      buf[len++] = '\n';
      buf[len] = '\0';
    */
}

afx_msg LRESULT CDialAsmStat::OnAbortAsm(WPARAM wParam, LPARAM /* lParam */)
{
    // TODO: I expect there to be more to this. -- B.Simonds (April 27, 2024)

    EndModal(wxID_CANCEL);
    return LRESULT(0);
}

afx_msg LRESULT CDialAsmStat::OnGetNextLine(WPARAM wParam, LPARAM lParam)
{
#if 0
//  std::string test;
//  m_edit.GetWindowText(test);

    ASSERT((char *)lParam != NULL); // Required return buffer address
    ASSERT(wParam > 0); // Minimum length required for buffer

    if (m_nCurrLine >= m_nLines)
    {
        *(char *)lParam = '\0';
        return LRESULT(-1);
    }

//  const std::string &line = GetLine(m_nCurrLine++);
//  int len= min((int)wParam - 1, line.GetLength());
//  _tcsncpy((char *)lParam, (const char *)line, len);
//  *((char *)lParam + len) = '\0';

    GetLine(m_nCurrLine++, (char *)lParam, (int)wParam-1);

    DWORD timer = ::GetTickCount();
    if (timer - m_dwTimer >= 100)
    {
        m_dwTimer = timer;
        SendDlgItemMessage(IDC_ASM_PROGRESS, PBM_SETPOS, (m_nPassNo - 1) * m_nLines + m_nCurrLine);
        SetLineNo(m_nCurrLine); // Display the current line number
    }

    return LRESULT(1);
#endif
    return 0;
}

void CDialAsmStat::SetPassNo(int val) // Display the assembly pass number
{
    //SetCtrlText(IDC_PASS_NO, val);
}

void CDialAsmStat::SetLineNo(int val) // Display the line number
{
    //SetCtrlText(IDC_CURR_ROW, val);
}

void CDialAsmStat::SetCtrlText(int id, int val)
{
    //SetDlgItemInt(id, val, FALSE);
}

void CDialAsmStat::ProgressStep()
{
    //CProgressCtrl *pProgress = static_cast<CProgressCtrl *>(GetDlgItem(IDC_ASM_PROGRESS));
    //pProgress->StepIt();
}

void CDialAsmStat::SetProgressRange(int max_line)
{
    //CProgressCtrl *pProgress = static_cast<CProgressCtrl *>(FindControl(IDC_ASM_PROGRESS));
    //pProgress->SetRange(1, 2 * max_line);
}

afx_msg LRESULT CDialAsmStat::OnGetLineNo(WPARAM wParam, LPARAM lParam)
{
#if 0
    ASSERT((int *)lParam != nullptr);
    *(int *)lParam = m_nCurrLine - 1; // Current line (numbers from 0)
    return LRESULT(1);
#endif
    return 0;
}

afx_msg LRESULT CDialAsmStat::OnGetDocTitle(WPARAM wParam, LPARAM lParam)
{
#if 0
    ASSERT((char *)lParam != nullptr); // Required return buffer address
    ASSERT(wParam > 0); // Minimum length required for buffer

    CSrc6502Doc* pDoc = dynamic_cast<CSrc6502Doc*>(m_pView->GetDocument());

    //if (pDoc && pDoc->m_strPath.IsEmpty())
    //    pDoc->m_strPath = pDoc->GetPathName();

    const std::string &title = pDoc ? pDoc->GetPathName()/*pDoc->m_strPath*/ : "";

    int len = min((int)wParam - 1, title.GetLength()); // Document name (with path)

    _tcsncpy((char *)lParam, title.c_str(), len);

    *((char *)lParam + len) = '\0';
    return LRESULT(1);
#endif
    return 0;
}

afx_msg LRESULT CDialAsmStat::OnNextPass(WPARAM wParam, LPARAM /* lParam */)
{
    //ASSERT(wParam > 0); // Assembly pass number required
    SetPassNo(++m_nPassNo); // Display the assembly pass number
    m_nCurrLine = 0; // Reading returns to the beginning
    m_pText = m_strText.c_str();
    return LRESULT(1);
}

afx_msg LRESULT CDialAsmStat::OnFinished(WPARAM wParam, LPARAM /* lParam */)
{
    m_bFinished = true; // Assembly complete

#if 0
    std:::string ok;

    if (ok.LoadString(IDS_DIAL_ASM_OK)) // change the text on the button from 'Abort' to 'OK'
        SetDlgItemText(IDCANCEL, ok);
#endif

    wxControl *prg = FindControl(IDC_ASM_PROGRESS);
    prg->Hide();

    wxControl *icn = FindControl(IDC_DIAL_ASM_ICN1);
    icn->Hide();

    icn = FindControl(IDC_DIAL_ASM_ICN2);
    icn->Hide();

    icn = FindControl(IDC_DIAL_ASM_ICN3);
    icn->Hide();

    CGlobal &globals = wxGetApp().m_global;

    bool isStatusOkay = (CAsm::Stat)wParam == CAsm::Stat::OK;

    globals.SetCodePresence(isStatusOkay);
    globals.SetStart(m_pAsm6502->GetProgramStart());
    
    Broadcast::ToViews(EVT_PROG_MEM_CHANGED, (WPARAM)-1, isStatusOkay ? 0 : -1);
    Broadcast::ToPopups(EVT_PROG_MEM_CHANGED, (WPARAM)-1, isStatusOkay ? 0 : -1);

    if (wParam) // Assembly error?
    {
        SetLineNo(m_nCurrLine); // Display the current line number

        const std::string err = m_pAsm6502->GetErrMsg((CAsm::Stat)wParam);

        wxControl *ctrl = FindControl(IDC_DIAL_ASM_ERR);

        ctrl->SetLabel(err);
        ctrl->Show();
    }
    else
        EndModal(wxID_OK);

    return LRESULT(1);
}

void CDialAsmStat::OnCancel()
{
    if (m_bFinished) // Assembly finished?
    {
        if (m_stAsmRetCode) // Did we get an error?
        {
            m_pView->SetErrMark(m_nCurrLine - 1); // Select the line containing the error

            // This probably should be handled with the logging system. -- B.Simonds (April 27, 2024)
            wxGetApp().SetStatusText(0, m_pAsm6502->GetErrMsg(m_stAsmRetCode));
        }

        EndModal(wxID_OK);
    }
    else // Assembly interrupted
    {
        m_pAsm6502->Abort();
        EndModal(wxID_CANCEL);
    }
}

UINT CDialAsmStat::start_asm_thread(void *pDial)
{
    return ((CDialAsmStat *)pDial)->StartAsm();
}

UINT CDialAsmStat::StartAsm()
{
    // Get the file name

    CSrc6502Doc* pDoc = dynamic_cast<CSrc6502Doc*>(m_pView->GetDocument());
    wxFileName path(pDoc ? pDoc->GetTitle() : "");
    
#if 0
    if (path.HasName())
        CMainFrame::ProjName = path.GetName();
#endif
    
    m_stAsmRetCode = (CAsm::Stat)(m_pAsm6502->Assemble());
    //PostMessage(WM_USER_FIN, WPARAM(m_stAsmRetCode));

    return 0;
}

bool CDialAsmStat::OnSetCursor(wxWindow* pWnd, UINT nHitTest, UINT message)
{
#if 0
    if (!m_bFinished && nHitTest == HTCLIENT)
    {
        wxWindow *btn = GetDlgItem(IDCANCEL);
        ASSERT(btn != NULL); // Wrong object number in the dialog box
        
        // Convert cursor position to client co-ordinates
        wxPoint point;
        GetCursorPos(&point);

//      pWnd->ScreenToClient(&point);

        wxRect rect;

        btn->GetWindowRect(rect); // Button dimensions

        if (rect.PtInRect(point)) // Mouse over button?
            return CDialog::OnSetCursor(pWnd, nHitTest, message);

        ::SetCursor(wxGetApp().LoadStandardCursor(IDC_WAIT));
        return true;
    }

    return wxDialog::OnSetCursor(pWnd, nHitTest, message);
#endif

    return false;
}
