
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

// Assembler for M65XX and M65C02 microprocessors

//#include <ctime>
#include "StdAfx.h"

#include "M6502.h"

/*************************************************************************/

CAsm6502::Stat CConditionalAsm::instr_if_found(CAsm::Stat condition)
{
    ASSERT(condition == CAsm::STAT_IF_TRUE || condition == CAsm::STAT_IF_FALSE || condition == CAsm::STAT_IF_UNDETERMINED);

    bool assemble = level < 0 || get_assemble();

    if (assemble && condition == CAsm::STAT_IF_UNDETERMINED)
        return CAsm::ERR_UNDEF_EXPR; // Expected defined expression

    level++; // Change state, save it on the stack

    if (assemble && condition == CAsm::STAT_IF_TRUE)
    {
        set_state(BEFORE_ELSE, true);
        return CAsm::STAT_ASM; // Subsequent lines should be assembled
    }
    else
    {
        set_state(BEFORE_ELSE, false);
        return CAsm::STAT_SKIP; // Next lines should be skipped until .ELSE or .ENDIF
    }
}

CAsm6502::Stat CConditionalAsm::instr_else_found()
{
    if (level < 0 || get_state() != BEFORE_ELSE)
        return CAsm::ERR_SPURIOUS_ELSE;
        
    // Change the state of the machine
    if (get_assemble()) // before .ELSE lines assembled?
        set_state(AFTER_ELSE, false); // No more after .ELSE
    else
    {
        // lines before .ELSE were not assembled
        if (level > 0 && get_prev_assemble() || level == 0)	// Parent if/endif assembled
            set_state(AFTER_ELSE, true);                    // or no parent if/endif
        else
            set_state(AFTER_ELSE, false);
    }
    
    return get_assemble() ? CAsm::STAT_ASM : CAsm::STAT_SKIP;
}

CAsm6502::Stat CConditionalAsm::instr_endif_found()
{
    if (level < 0)
        return CAsm::ERR_SPURIOUS_ENDIF;

    level--; // Change state by removing the top of the stack
    
    if (level >= 0)
        return get_assemble() ? CAsm::STAT_ASM : CAsm::STAT_SKIP;

    return CAsm::STAT_ASM;
}

void CConditionalAsm::restore_level(int new_level)
{
    if (new_level >= -1 && new_level < stack.size())
        level = new_level;
}

/*************************************************************************/
