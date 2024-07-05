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

// Code65p.h: interface for the CCode65p class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CODE_65P_H__
#define CODE_65P_H__

#include "Archive.h"
#include "ProjectManager.h"

class CCode65p : public BinaryCodeTemplate
{
protected:
    virtual bool read(BinaryArchive &archive, LoadCodeState *state);
    virtual bool write(BinaryArchive &archive, LoadCodeState *state);
    
public:
    /* constructor */ CCode65p();
    virtual	     ~CCode65p();

    virtual bool canRead() const { return true; }

    virtual bool canWrite() const { return false; }

    virtual std::string getDescription() const { return std::string(_("Binary Program")); }

    virtual std::vector<std::string> getExtensions() const { return { "65p" }; }
};

#endif /* CODE_65P_H__ */
