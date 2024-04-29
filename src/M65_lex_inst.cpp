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

bool CAsm6502::proc_instr(const std::string &str_, CAsm::OpCode &code)
{
    ASSERT(str_.size() == 3);

    char tmp[4] = { 0, 0, 0, 0 };
    
    for (int i = 0; i < 3; ++i) 
        tmp[i] = toupper(str_[i]);

    switch (tmp[0]) // Check whether 'str' is the instruction code
    {
    case 'A':
        switch (tmp[1])
        {
        case 'D':
            if (tmp[2] == 'C')
                return code = CAsm::C_ADC, true;
            break;
            
        case 'N':
            if (tmp[2] == 'D')
                return code = CAsm::C_AND, true;
            break;

        case 'S':
            if (tmp[2] == 'L')
                return code = CAsm::C_ASL, true;
            break;
        }
        break;

    case 'B':
        switch (tmp[1])
        {
        case 'B':
            if (bProc6502 != 1)
                break;
            if (tmp[2] == 'S')
                return code = CAsm::C_BBS, true;
            else if (tmp[2] == 'R')
                return code = CAsm::C_BBR, true;
            break;

        case 'C':
            if (tmp[2] == 'C')
                return code = CAsm::C_BCC, true;
            else if (tmp[2] == 'S')
                return code = CAsm::C_BCS, true;
            break;

        case 'E':
            if (tmp[2] == 'Q')
                return code = CAsm::C_BEQ, true;
            break;

        case 'I':
            if (tmp[2] == 'T')
                return code = CAsm::C_BIT, true;
            break;

        case 'M':
            if (tmp[2] == 'I')
                return code = CAsm::C_BMI, true;
            break;

        case 'N':
            if (tmp[2] == 'E')
                return code = CAsm::C_BNE, true;
            break;

        case 'P':
            if (tmp[2] == 'L')
                return code = CAsm::C_BPL, true;
            break;

        case 'R':
            if (!(bProc6502 == 0) && tmp[2] == 'A')
                return code = CAsm::C_BRA, true;
            else if (tmp[2] == 'K')
                return code = CAsm::C_BRK, true;
            else if ((bProc6502 == 2) && tmp[2] == 'L')  //% 65816
                return code = CAsm::C_BRL, true;
            break;

        case 'V':
            if (tmp[2] == 'C')
                return code = CAsm::C_BVC, true;
            else if (tmp[2] == 'S')
                return code = CAsm::C_BVS, true;
            break;
        }
        break;

    case 'C':
        switch (tmp[1])
        {
        case 'L':
            if (tmp[2] == 'C')
                return code = CAsm::C_CLC, true;
            else if (tmp[2] == 'D')
                return code = CAsm::C_CLD, true;
            else if (tmp[2] == 'I')
                return code = CAsm::C_CLI, true;
            else if (tmp[2] == 'V')
                return code = CAsm::C_CLV, true;
            break;

        case 'M':
            if (tmp[2] == 'P')
                return code = CAsm::C_CMP, true;
            break;

        case 'O':
            if ((bProc6502 == 2) && tmp[2] == 'P')  //% 65816
                return code = CAsm::C_COP, true;
            break;

        case 'P':
            if (tmp[2] == 'X')
                return code = CAsm::C_CPX, true;
            else if (tmp[2] == 'Y')
                return code = CAsm::C_CPY, true;
            break;
        }
        break;

    case 'D':
        if (tmp[1] == 'E')
        {
            switch (tmp[2])
            {
            case 'A':
                if (bProc6502 != 0)
                    return code = CAsm::C_DEA, true;
                break;

            case 'C':
                return code = CAsm::C_DEC, true;

            case 'X':
                return code = CAsm::C_DEX, true;

            case 'Y':
                return code = CAsm::C_DEY, true;
            }
        }
        break;

    case 'E':
        if (tmp[1] == 'O' && tmp[2] == 'R')
            return code = CAsm::C_EOR, true;
        break;

    case 'I':
        if (tmp[1] == 'N')
            switch (tmp[2])
            {
            case 'A':
                if (bProc6502 != 0)
                    return code = CAsm::C_INA, true;
                break;

            case 'C':
                return code = CAsm::C_INC, true;

            case 'X':
                return code = CAsm::C_INX, true;

            case 'Y':
                return code = CAsm::C_INY, true;
            }
        break;

    case 'J':
        if (tmp[1] =='M' && tmp[2] == 'P')
            return code = CAsm::C_JMP, true;
        else if (tmp[1] == 'S' && tmp[2] == 'R')
            return code = CAsm::C_JSR, true;
        else if ((bProc6502 == 2) && tmp[1] == 'M' && tmp[2] =='L')  //% 65816
            return code = CAsm::C_JML, true;
        else if ((bProc6502 == 2) && tmp[1] == 'S' && tmp[2] =='L')  //% 65816
            return code = CAsm::C_JSL, true;
        break;

    case 'L':
        if (tmp[1] == 'D')
        {
            if (tmp[2] == 'A')
                return code = CAsm::C_LDA, true;
            else if (tmp[2] == 'X')
                return code = CAsm::C_LDX, true;
            else if (tmp[2] == 'Y')
                return code = CAsm::C_LDY, true;
        }
        else if (tmp[1] =='S' && tmp[2] == 'R')
            return code = CAsm::C_LSR, true;
        break;

    case 'M':
        if (tmp[1] == 'V')
        {
            if ((bProc6502 == 2) && tmp[2] =='N')  //% 65816
                return code = CAsm::C_MVN, true;
            else if ((bProc6502 == 2) && tmp[2] == 'P')  //% 65816
                return code = CAsm::C_MVP, true;
        }
        break;

    case 'N':
        if (tmp[1] =='O' && tmp[2] == 'P')
            return code = CAsm::C_NOP, true;
        break;

    case 'O':
        if (tmp[1] =='R' && tmp[2] == 'A')
            return code = CAsm::C_ORA, true;
        break;

    case 'P':
        if (tmp[1] == 'E')
        {
            if ((bProc6502 == 2) && tmp[2] == 'A')  //% 65816
                return code = CAsm::C_PEA, true;
            else if ((bProc6502 == 2) && tmp[2] == 'I')  //% 65816
                return code = CAsm::C_PEI, true;
            else if ((bProc6502 == 2) && tmp[2] == 'R')  //% 65816
                return code = CAsm::C_PER, true;
        }
        else if (tmp[1] =='H')
            switch (tmp[2])
            {
            case 'A':
                return code = CAsm::C_PHA, true;

            case 'B':
                if (bProc6502 == 2)
                    return code = CAsm::C_PHB, true;  //% 65816
                break;

            case 'D':
                if (bProc6502 == 2)
                    return code = CAsm::C_PHD, true;  //% 65816
                break;

            case 'K':
                if (bProc6502 == 2)
                    return code = CAsm::C_PHK, true;  //% 65816
                break;

            case 'P':
                return code = CAsm::C_PHP, true;

            case 'X':
                if (bProc6502 != 0)
                    return code = CAsm::C_PHX, true;
                break;

            case 'Y':
                if (bProc6502 != 0)
                    return code = CAsm::C_PHY, true;
                break;
            }
        else if (tmp[1] == 'L')
            switch (tmp[2])
            {
            case 'A':
                return code = CAsm::C_PLA, true;

            case 'B':
                if (bProc6502 == 2) 
                    return code = CAsm::C_PLB, true;  //% 65816
                break;

            case 'D':
                if (bProc6502 == 2)
                    return code = CAsm::C_PLD, true;  //% 65816
                break;

            case 'P':
                return code = CAsm::C_PLP, true;

            case 'X':
                if (bProc6502 != 0)
                    return code = CAsm::C_PLX, true;
                break;
                
            case 'Y':
                if (bProc6502 != 0)
                    return code = CAsm::C_PLY, true;
                break;
            }
        break;

    case 'R':
        switch (tmp[1])
        {
        case 'E':
            if ((bProc6502 == 2) && tmp[2] == 'P')  //% 65816
                return code = CAsm::C_REP, true;
            break;

        case 'O':
            if (tmp[2] == 'L')
                return code = CAsm::C_ROL, true;
            else if (tmp[2] == 'R')
                return code = CAsm::C_ROR, true;
            break;

        case 'M':
            if ((bProc6502 == 1) && tmp[2] == 'B')
                return code = CAsm::C_RMB, true;
            break;

        case 'T':
            if (tmp[2] == 'I')
                return code = CAsm::C_RTI, true;
            else if ((bProc6502 == 2) && tmp[2] == 'L')  //% 65816
                return code = CAsm::C_RTL, true;
            else if (tmp[2] == 'S')
                return code = CAsm::C_RTS, true;
            break;
        }
        break;

    case 'S':
        switch (tmp[1])
        {
        case 'B':
            if (tmp[2] == 'C')
                return code = CAsm::C_SBC, true;
            break;

        case 'E':
            if (tmp[2] == 'C')
                return code = CAsm::C_SEC, true;
            else if (tmp[2] == 'D')
                return code = CAsm::C_SED, true;
            else if (tmp[2] == 'I')
                return code = CAsm::C_SEI, true;
            else if ((bProc6502 == 2) && tmp[2] =='P')  //% 65816
                return code = CAsm::C_SEP, true;
            break;

        case 'M':
            if ((bProc6502 == 1) && tmp[2] == 'B')
                return code = CAsm::C_SMB, true;
            break;

        case 'T':
            if (tmp[2] == 'A')
                return code = CAsm::C_STA, true;
            else if ((bProc6502 == 2) && tmp[2] == 'P')  //% 65816
                return code = CAsm::C_STP, true;
            else if (tmp[2] == 'X')
                return code = CAsm::C_STX, true;
            else if (tmp[2] == 'Y')
                return code = CAsm::C_STY, true;
            else if (!(bProc6502 == 0) && tmp[2] == 'Z')
                return code = CAsm::C_STZ, true;
            break;
        }
        break;

    case 'T':
        switch (tmp[1])
        {
        case 'A':
            if (tmp[2] == 'X')
                return code = CAsm::C_TAX, true;
            if (tmp[2] == 'Y')
                return code = CAsm::C_TAY, true;
            break;

        case 'C':
            if ((bProc6502 == 2) && tmp[2] == 'D')  //% 65816
                return code = CAsm::C_TCD, true;
            if ((bProc6502 == 2) && tmp[2] == 'S')  //% 65816
                return code = CAsm::C_TCS, true;
            break;

        case 'D':
            if ((bProc6502 == 2) && tmp[2] == 'C')  //% 65816
                return code = CAsm::C_TDC, true;
            break;

        case 'R':
            if (!(bProc6502 == 0) && tmp[2] == 'B')
                return code = CAsm::C_TRB, true;
            break;

        case 'S':
            if (!(bProc6502 == 0) && tmp[2]=='B')
                return code = CAsm::C_TSB, true;
            if ((bProc6502 == 2) && tmp[2] == 'C')  //% 65816
                return code = CAsm::C_TSC, true;
            if (tmp[2] == 'X')
                return code = CAsm::C_TSX, true;
            break;

        case 'X':
            if (tmp[2] == 'A')
                return code = CAsm::C_TXA, true;
            if (tmp[2] == 'S')
                return code = CAsm::C_TXS, true;
            if ((bProc6502 == 2) && tmp[2] == 'Y')  //% 65816
                return code = CAsm::C_TXY, true;
            break;

        case 'Y':
            if (tmp[2] == 'A')
                return code = CAsm::C_TYA, true;
            if ((bProc6502 == 2) && tmp[2] == 'X')  //% 65816
                return code = CAsm::C_TYX, true;
            break;
        }
        break;

    case 'W':
        if ((bProc6502 == 2) && tmp[1] == 'A' && tmp[2] == 'I')  //% 65816
            return code = CAsm::C_WAI, true;
        if ((bProc6502 == 2) && tmp[1] == 'D' && tmp[2] == 'M')  //% 65816
            return code = CAsm::C_WDM, true;
        break;

    case 'X':
        if ((bProc6502 == 2) && tmp[1] == 'B' && tmp[2] == 'A')  //% 65816
            return code = CAsm::C_XBA, true;
        if ((bProc6502 == 2) && tmp[1] == 'C' && tmp[2] == 'E')  //% 65816
            return code = CAsm::C_XCE, true;
        break;
    }

    return false;
}

/*************************************************************************/