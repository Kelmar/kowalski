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

// LogWindow.cpp : implementation file
//

#include "StdAfx.h"
#include "LogWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CLogWindow

CLogWindow::CLogWindow()
{
    m_bHidden = false;
}

CLogWindow::~CLogWindow()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLogWindow message handlers

bool CLogWindow::Create()
{
#if 0
    m_brBackground.DeleteObject();
    m_brBackground.CreateSolidBrush(::GetSysColor(COLOR_WINDOW));

    std::string strClass= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW/*CS_DBLCLKS*/, ::LoadCursor(NULL, IDC_ARROW), 0,
                                          AfxGetApp()->LoadIcon(IDI_MEMORY_INFO));

    m_WndRect.SetRect(100, 100, 400, 600);

    if (!CMiniFrameWnd::Create(strClass, "Command Log",
                               WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_THICKFRAME | MFS_SYNCACTIVE,
                               m_WndRect, AfxGetMainWnd(), 0))
        return false;

    CCreateContext ctx;
    ctx.m_pNewViewClass = RUNTIME_CLASS(CEditView);
    ctx.m_pCurrentDoc = 0;			// document
    ctx.m_pNewDocTemplate = NULL;	// template
    ctx.m_pLastView = NULL;			// lastView
    ctx.m_pCurrentFrame = this;		// current frame

    CEditView* pView= static_cast<CEditView*>(CreateView(&ctx));

    if (pView == 0)
        return false;

    pView->ModifyStyle(ES_AUTOHSCROLL | WS_HSCROLL,
                       ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE, SWP_FRAMECHANGED);

    pView->GetEditCtrl().SetReadOnly();

    if (m_fntMono.m_hObject == 0)
    {
        HFONT hFont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
        LOGFONT lf;
        ::GetObject(hFont, sizeof(lf), &lf);
        lf.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lf.lfFaceName, "FixedSys");
        m_fntMono.CreateFontIndirect(&lf);
    }
    pView->SetFont(&m_fntMono);

    RecalcLayout();

    InitialUpdateFrame(0, false);
#endif

    return true;
}

void CLogWindow::PostNcDestroy()
{
    // skip default: deletes this
}

int CLogWindow::OnCtlColor(wxDC* dc, wxWindow* pWnd, UINT nCtlColor)
{
    UNUSED(dc);
    UNUSED(pWnd);
    UNUSED(nCtlColor);

    //dc->SetBkColor(::GetSysColor(COLOR_WINDOW));
    //dc->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
    //return m_brBackground;
    return 0;
}

void CLogWindow::SetText(const CommandLog& log)
{
    UNUSED(log);

#if 0
    int nCount = log.GetCount();

    if (nCount == 0)
    {
        if (wxWindow* pView = GetActiveView())
            pView->SetWindowText("");
        return;
    }

    std::string strBuf;
    strBuf.GetBuffer(nCount * 16);	// estimate
    strBuf.ReleaseBuffer(0);

    for (int i= 0; i < nCount; ++i)
        strBuf += log[i].Asm() + "\r\n";

    if (wxWindow* pView = GetActiveView())
    {
        wxEdit* pEdit = static_cast<wxEdit*>(pView);
        pEdit->SetWindowText(strBuf);
        int nLen = strBuf.GetLength();
        pEdit->SetSel(nLen, nLen);
    }
#endif
}

void CLogWindow::Invalidate()
{
#if 0
    if (wxWindow* pView = GetActiveView())
        pView->Refresh();
#endif
}

//=============================================================================

afx_msg LRESULT CLogWindow::OnChangeCode(WPARAM wParam, LPARAM lParam)
{
    UNUSED(wParam);

    if (lParam == static_cast<uint32_t>(-1))
        Close();
    else
        Refresh();
    return 0;
}

afx_msg LRESULT CLogWindow::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    if (!m_bHidden) // Was the window visible?
        Show();

    return 1;
}

afx_msg LRESULT CLogWindow::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    if (IsShown()) // window currently displayed?
    {
        m_bHidden = FALSE; // info -the window was displayed
        Hide();
    }
    else
        m_bHidden = TRUE; // info -the window was hidden

    return 1;
}

void CLogWindow::OnClose()
{
    Hide();
}
