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

#ifndef CONDITIONAL_ASM_H__ 
#define CONDITIONAL_ASM_H__ 

#include <StdAfx.h>

/*=======================================================================*/

// Conditional assembly (stack machine)
class CConditionalAsm
{
public:
    enum State
    {
        BEFORE_ELSE,
        AFTER_ELSE
    };

private:
    std::vector<uint8_t> stack;
    int level;

    State get_state()
    {
        ASSERT(level >= 0);
        return stack[level] & 1 ? BEFORE_ELSE : AFTER_ELSE;
    }

    bool get_assemble()
    {
        ASSERT(level >= 0);
        return stack[level] & 2 ? true : false;
    }

    bool get_prev_assemble()
    {
        ASSERT(level > 0);
        return stack[level - 1] & 2 ? true : false;
    }

    void set_state(State state, bool assemble)
    {
        if (stack.capacity() < static_cast<std::size_t>(level))
            stack.reserve(level);

        stack[level] = uint8_t((state == BEFORE_ELSE ? 1 : 0) + (assemble ? 2 : 0));
    }

public:
    CConditionalAsm()
        : stack()
        , level(-1)
    {
        stack.reserve(16);
    }

    CAsm::Stat instr_if_found(CAsm::Stat condition);
    CAsm::Stat instr_else_found();
    CAsm::Stat instr_endif_found();

    bool in_cond() const
    {
        return level >= 0;
    }

    int get_level() const
    {
        return level;
    }

    void restore_level(int level);
};

/*=======================================================================*/

#endif /* CONDITIONAL_ASM_H__ */

/*=======================================================================*/
