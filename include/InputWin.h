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

#ifndef INPUT_WIN_H__
#define INPUT_WIN_H__

/*************************************************************************/

#include "Asm.h"
#include "Ident.h"
#include "DebugInfo.h"
#include "OutputMem.h"

#include "InputBase.h"

/*************************************************************************/

// Reading data from the document window
class CInputWin : public CInputBase
{
private:
    wxWindow *m_window;

public:
    CInputWin(wxWindow *window) : m_window(window)
    {}

    virtual bool read_line(std::string &buf);
    virtual const std::string &get_file_name();
    virtual void seek_to_begin();
};

/*************************************************************************/

#endif /* INPUT_WIN_H__ */

/*************************************************************************/
