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

#ifndef MEMORY_INFO_H__
#define MEMORY_INFO_H__

// MemoryInfo.h : header file
//

#include "MemoryView.h"
#include "MemoryDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryInfo frame
class COutputMem;

class CMemoryInfo : public wxWindow //CMiniFrameWnd
{
private:
    COutputMem *m_pMem;
    uint32_t m_uAddr;
    CMemoryDoc m_Doc;

public:
    /* constructor */ CMemoryInfo();
    virtual          ~CMemoryInfo();

    wxRect m_WndRect;
    bool m_bHidden;

public:
    enum ViewType { VIEW_MEMORY, VIEW_ZEROPAGE, VIEW_STACK };

    bool Create(COutputMem *mem, uint32_t addr, ViewType view);

    void InvalidateView(uint16_t stackPtr = 0);

protected:
    virtual void PostNcDestroy();

    afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);
    // Generated message map functions
    afx_msg void OnDestroy();
    afx_msg void OnShowWindow(bool show, UINT nStatus);
    afx_msg void OnClose();
    afx_msg LRESULT OnChangeCode(WPARAM wParam, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////

#endif /* MEMORY_INFO_H__ */
