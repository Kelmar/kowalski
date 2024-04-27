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

#include "stdafx.h"
#include "MainFrm.h"

wxDEFINE_EVENT(EVT_EXIT_DEBUGGER, wxCommandEvent);
wxDEFINE_EVENT(EVT_START_DEBUGER, wxCommandEvent);
wxDEFINE_EVENT(EVT_UPDATE_REG_WND, wxCommandEvent);
wxDEFINE_EVENT(EVT_PROG_MEM_CHANGED, wxCommandEvent); // All 6502 memory has been changed (after LOAD or assembly)
wxDEFINE_EVENT(EVT_REMOVE_ERR_MARK, wxCommandEvent);

#if REWRITE_TO_WX_WIDGET

// I susspect we want these functions to be in our app, not here. -- B.Simonds (April 27, 2024)

// Sending a message to all open document windows
void CBroadcast::SendMessageToViews(UINT msg, WPARAM wParam/*= 0*/, LPARAM lParam/*= 0*/)
{
    CWinApp *pApp = AfxGetApp();
    POSITION posTempl = pApp->GetFirstDocTemplatePosition();
    while (posTempl != NULL)
    {
        CDocTemplate *pTempl = pApp->GetNextDocTemplate(posTempl);
        POSITION posDoc= pTempl->GetFirstDocPosition();
        while (posDoc != NULL)
        {
            CDocument *pDoc = pTempl->GetNextDoc(posDoc);
            POSITION posView = pDoc->GetFirstViewPosition();
            while (posView != NULL)
            {
                CView* pView = pDoc->GetNextView(posView);
                pView->SendMessage(msg,wParam,lParam);
            }
        }
    }
}

// Sending a message to windows stored in g_hWindows[]
void CBroadcast::SendMessageToPopups(UINT msg, WPARAM wParam/*= 0*/, LPARAM lParam/*= 0*/)
{
    for (int i = 0; CMainFrame::m_hWindows[i]; i++)
    {
        HWND hWnd = *CMainFrame::m_hWindows[i];
        if (hWnd && ::IsWindow(hWnd))
            ::SendMessage(hWnd, msg, wParam, lParam);
    }
}

#endif
