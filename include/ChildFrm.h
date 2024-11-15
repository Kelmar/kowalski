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

// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CHILD_FRAME_H__
#define CHILD_FRAME_H__

#include "mdi65.h"
#include "HexView.h"

class CChildFrame : public CHILD_BASE
{
private:
    //HexView *m_hexView;
    //uint8_t m_data[1024 * 64]; // Dummy data

public:
    //CSplitterWnd m_wndSplitter;

    /* constructor */ CChildFrame(MAIN_BASE *parent, wxWindowID id, const wxString &title);
    virtual          ~CChildFrame() { }

protected:
    //virtual bool OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};

/////////////////////////////////////////////////////////////////////////////

#endif /* CHILD_FRAME_H__ */
