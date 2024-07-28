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

#ifndef LOAD_CODE_OPTIONS_H__
#define LOAD_CODE_OPTIONS_H__

#include "HexValidator.h"
#include "Sym6502.h"

/*************************************************************************/
/**
 * @brief Model that holds the state of loading and saving raw binary code.
 */
struct LoadCodeState
{
    LoadCodeState()
	: StartAddress(CAsm::INVALID_ADDRESS)
	, LoadAddress(0)
	, ClearMemory(false)
	, FillByte(0)
	, Memory(new COutputMem())
	, Marks()
    {
    }

    // Address where simulator should jump to, to begin execution.
    uint32_t StartAddress;

    // Address where we should load the block of memory to.
    uint32_t LoadAddress;

    // Set if the memory should be cleared with FillByte
    bool ClearMemory;

    // The byte value to clear the memory with.
    uint32_t FillByte;

    // The memory that is to be loaded into the simulator
    CMemoryPtr Memory;

    // Sections of memory to be marked.
    CMarkArea Marks;
};

/*************************************************************************/
/**
 * @brief Dialog box that shows options for loading binary code.
 */
class LoadCodeOptionsDlg : public wxDialog, public wxExtra
{
private:
    LoadCodeState *m_state;

public:
    /* constructor */ LoadCodeOptionsDlg(LoadCodeState *state);
};

/*************************************************************************/

#endif /* LOAD_CODE_OPTIONS_H__ */
