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

#ifndef _broadcast_
#define _broadcast_

wxDECLARE_EVENT(EVT_EXIT_DEBUGGER, wxCommandEvent);
wxDECLARE_EVENT(EVT_START_DEBUGGER, wxCommandEvent);
wxDECLARE_EVENT(EVT_UPDATE_REG_WND, wxCommandEvent);
wxDECLARE_EVENT(EVT_PROG_MEM_CHANGED, wxCommandEvent); // All 6502 memory has been changed (after LOAD or assembly)
wxDECLARE_EVENT(EVT_REMOVE_ERR_MARK, wxCommandEvent);

namespace Broadcast
{
    void ToViews(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
    void ToPopups(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
}

#endif
