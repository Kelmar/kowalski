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

#ifndef ATARI_BIN_H__
#define ATARI_BIN_H__

#include "Archive.h"
#include "LoadCodeOptions.h"
#include "ProjectManager.h"

/**
 * @brief Loads Atari binary files (xex) into simulator memory.
 */
class CAtariBin : public BinaryCodeTemplate
{
protected:
    virtual void read(BinaryArchive &archive, LoadCodeState *state);
    virtual void write(BinaryArchive &archive, LoadCodeState *state)
    { 
	UNUSED(archive);
	UNUSED(state);
	ASSERT(false); 
    }

public:
    /* constructor */ CAtariBin();
    virtual	     ~CAtariBin();

    virtual bool canRead() const { return true; }

    virtual bool canWrite() const { return false; }

    virtual std::string getDescription() const { return std::string(_("Atari Binary Files")); }

    virtual std::vector<std::string> getExtensions() const { return { "xex" }; }
};

#endif /* ATARI_BIN_H__ */

