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
#include "resource.h"
#include "MemoryInfo.h"
#include "ZeroPageView.h"
#include "StackView.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryInfo

CMemoryInfo::CMemoryInfo()
    : m_bHidden(false)
{
}

CMemoryInfo::~CMemoryInfo()
{
}

/*************************************************************************/
// New window

bool CMemoryInfo::Create(COutputMem *pMem, uint32_t uAddr, ViewType bView)
{
#if REWRITE_TX_WX_WIDGET
    m_pMem = pMem;
    m_uAddr = uAddr;

    CString title;
    CCreateContext ctx;
    if (bView == VIEW_MEMORY)
    {
        ctx.m_pNewViewClass = RUNTIME_CLASS(CMemoryView);
        title.LoadString(IDS_MEMORY_TITLE);
    }
    else if (bView == VIEW_ZEROPAGE)
    {
        ctx.m_pNewViewClass = RUNTIME_CLASS(CZeroPageView);
        title.LoadString(IDS_ZMEMORY_TITLE);
    }
    else if (bView == VIEW_STACK)
    {
        ctx.m_pNewViewClass = RUNTIME_CLASS(CStackView);
        title.LoadString(IDS_STACK);
    }
    else
    {
        ASSERT(false);
    }

    ctx.m_pCurrentDoc = &m_Doc;
    ctx.m_pNewDocTemplate = nullptr;
    ctx.m_pLastView = nullptr;
    ctx.m_pCurrentFrame = this;

    if (!CMiniFrameWnd::Create(m_strClass, title,
                               WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_THICKFRAME | MFS_SYNCACTIVE,
                               m_WndRect, AfxGetMainWnd(), 0))
    {
        return false;
    }

    if (!CreateView(&ctx))
    {
//    delete this;
        return false;
    }
    RecalcLayout();
    m_Doc.SetData(m_pMem, m_uAddr);
    InitialUpdateFrame(&m_Doc, false);
#endif

    return true;
}

/*************************************************************************/

#if REWRITE_TO_WX_WIDGET

BEGIN_MESSAGE_MAP(CMemoryInfo, CMiniFrameWnd)
    //{{AFX_MSG_MAP(CMemoryInfo)
    ON_WM_DESTROY()
    ON_WM_SHOWWINDOW()
    ON_WM_CLOSE()
    //}}AFX_MSG_MAP
    ON_MESSAGE(CBroadcast::WM_USER_PROG_MEM_CHANGED, OnChangeCode)
    ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebug)
    ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebug)
END_MESSAGE_MAP()

#endif

/*************************************************************************/

afx_msg LRESULT CMemoryInfo::OnChangeCode(WPARAM wParam, LPARAM lParam)
{
    if (lParam == -1)
        Close(); // No code -close the window
    else
        Refresh();

    return 0;
}

/*************************************************************************/

afx_msg LRESULT CMemoryInfo::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    if (!m_bHidden) // Was the window previously visible?
        Show();

    return 1;
}

afx_msg LRESULT CMemoryInfo::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
    if (IsShown()) // window currently displayed?
    {
        m_bHidden = false; //info -the window was displayed
        Hide();
    }
    else
        m_bHidden = true; // info -the window was hidden

    return 1;
}

/*************************************************************************/

void CMemoryInfo::OnShowWindow(bool bShow, UINT nStatus)
{
#if REWRITE_TO_WX_WIDGET
    CMiniFrameWnd::OnShowWindow(bShow, nStatus);

    m_Doc.UpdateAllViews(0, 'show', 0);
#endif
}

void CMemoryInfo::OnClose()
{
    m_bHidden = false;

#if REWRITE_TO_WX_WIDGET
    CMiniFrameWnd::OnClose();
#endif
}

void CMemoryInfo::InvalidateView(uint16_t uStackPtr/*= 0*/)
{
    m_Doc.m_stackPtr = uStackPtr;

#if REWRITE_TO_WX_WIDGET
    m_Doc.UpdateAllViews(0, 'invl', 0);
#endif
}

/*************************************************************************/