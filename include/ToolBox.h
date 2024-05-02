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

#ifndef TOOLBOX_H__
#define TOOLBOX_H__

// ToolBox.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CToolBox dialog

class CToolBox : public wxDialog
{
private:
    int rsc_id;

public:
    /* constructor */ CToolBox();
    virtual          ~CToolBox();
    
    bool Create(int rsc_id, wxWindow* parent);
};

#endif /* TOOLBOX_H__ */
