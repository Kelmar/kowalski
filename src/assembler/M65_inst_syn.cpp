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

/*=======================================================================*/
// Interpretation of processor instruction arguments
CAsm6502::Stat CAsm6502::proc_instr_syntax(CToken &leks, CodeAdr &mode, Expr &expr,
    Expr &expr_bit, Expr &expr_zpg)
{
    static char x_idx_reg[] = "X";
    static char y_idx_reg[] = "Y";
    static char s_idx_reg[] = "S";
    bool reg_x = false;
    bool longImm = false;
    std::string val;
    uint16_t move;
    Stat ret;

    switch (leks.type)
    {
    case CTokenType::L_LBRACKET_L: // 65816 '[' long indirect
        if (m_procType != ProcessorType::WDC65816)
            return ERR_MODE_NOT_ALLOWED; // [ only allowed for 65816

        leks = next_leks(); // another non-empty lexeme
        ret = expression(leks, expr);

        if (ret) // invalid expression?
            return ret;

        if (expr.inf == Expr::EX_LONG)
            return ERR_NUM_LONG; // Too large number, max $FFFF

        switch (leks.type)
        {
        case CTokenType::L_SPACE:
            ASSERT(false);
            break;

        case CTokenType::L_COMMA:
            return ERR_BRACKET_R_EXPECTED;

        case CTokenType::L_LBRACKET_R: // 65816 ']'
            leks = next_leks(); // Another non-empty lexeme

            if (leks.type != CTokenType::L_COMMA)
            {
                switch (expr.inf)
                {
                case Expr::EX_UNDEF: // Undefined expression value?
                case Expr::EX_BYTE:  // Expression < 256?
                    mode = A_ZPIL;
                    break;

                case Expr::EX_WORD:
                    mode = A_INDL;
                    break;

                default:
                    ASSERT(false);
                    //!!! add errors for string or long?
                    break;
                }

                return OK;
            }

            leks = next_leks(); // Another non-empty lexeme

            if (leks.type != CTokenType::L_IDENT)
                return ERR_IDX_REG_Y_EXPECTED;

            val = leks.GetString();

            if (strcasecmp(val.c_str(), y_idx_reg)) // Not register Y?
                return ERR_IDX_REG_Y_EXPECTED;

            if (expr.inf == Expr::EX_WORD)
                return ERR_INDIRECT_BYTE_EXPECTED;

            mode = A_ZPIL_Y;
            leks = next_leks(); // Another non-empty lexeme
            return OK;

        default:
            return ERR_COMMA_OR_BRACKET_EXPECTED;
        }
        return OK;

    case CTokenType::L_BRACKET_L: // Left paren '('
        leks = next_leks(); // Another non-empty lexeme
        ret = expression(leks, expr);

        if (ret) // Incorrect expression?
            return ret;

        //if (expr.inf == Expr::EX_LONG)
        //    return ERR_NUM_LONG; // max $FFFF

        switch (leks.type)
        {
        case CTokenType::L_SPACE:
            ASSERT(false);
            break;

        case CTokenType::L_COMMA:
            reg_x = false;
            if ((m_procType == ProcessorType::M6502) && expr.inf != Expr::EX_BYTE && expr.inf != Expr::EX_UNDEF)
                return ERR_NUM_NOT_BYTE; // Too large number, max $FF

            leks = next_leks(); // Another non-empty lexeme

            if (leks.type != CTokenType::L_IDENT)
                return ERR_IDX_REG_EXPECTED;

            val = leks.GetString();

            if (strcasecmp(val.c_str(), x_idx_reg) == 0) // Index register 'X'?
                reg_x = true;
            else if (strcasecmp(val.c_str(), s_idx_reg)) // Not register 'S'?
                return ERR_IDX_REG_EXPECTED;

            leks = next_leks(); // Another non-empty lexeme

            if (leks.type != CTokenType::L_BRACKET_R)
                return ERR_BRACKET_R_EXPECTED;	// brak nawiasu ')'

            if (expr.inf == Expr::EX_LONG)
            {
                expr.inf = Expr::EX_WORD;
                //return ERR_NUM_LONG;
            }

            if (expr.inf == Expr::EX_WORD && reg_x)
                mode = A_ABSI_X;
            else if (expr.inf == Expr::EX_BYTE && reg_x)
                mode = A_ZPGI_X;
            else if (reg_x)
                mode = A_ABSIX_OR_ZPGIX;
            else
            {
                leks = next_leks(); // Another non-empty lexeme

                if (leks.type != CTokenType::L_COMMA)
                    return ERR_COMMA_OR_BRACKET_EXPECTED;

                leks = next_leks(); // Another non-empty lexeme

               val = leks.GetString();

                if (leks.type != CTokenType::L_IDENT)
                    return ERR_IDX_REG_Y_EXPECTED;
                else if (strcasecmp(val.c_str(), y_idx_reg)) // Not 'S' register?
                    return ERR_IDX_REG_Y_EXPECTED;

                if ((expr.inf == Expr::EX_BYTE) | (expr.inf == Expr::EX_UNDEF))
                    mode = A_SRI_Y;
                else
                    return ERR_NUM_NOT_BYTE;
            }

            leks = next_leks(); // Another non-empty lexeme
            return OK;

        case CTokenType::L_BRACKET_R:
            leks = next_leks(); // Another non-empty lexeme

            if (leks.type != CTokenType::L_COMMA)
            {
                switch (expr.inf)
                {
                case Expr::EX_UNDEF: // nieokre�lona warto�� wyra�enia?
                    mode = A_ABSI_OR_ZPGI; // Undetermined addressing mode
                    break;

                case Expr::EX_BYTE: // expression < 256?
                    mode = A_ZPGI;
                    break;

                case Expr::EX_WORD:
                    mode = A_ABSI;
                    break;

                case Expr::EX_LONG:
                    mode = A_ABSI; // added use WORD unless \3 is inserted
                    break;

                default:
                    ASSERT(false);
                }
                return OK;
            }

            leks = next_leks(); // Another non-empty lexeme

            if (leks.type != CTokenType::L_IDENT)
                return ERR_IDX_REG_Y_EXPECTED;

            val = leks.GetString();

            if (strcasecmp(val.c_str(), y_idx_reg)) // Not register Y?
                return ERR_IDX_REG_Y_EXPECTED;

            if (expr.inf == Expr::EX_WORD)
                return ERR_INDIRECT_BYTE_EXPECTED;

            mode = A_ZPGI_Y;
            leks = next_leks(); // Another non-empty lexeme
            return OK;

        default:
            return ERR_COMMA_OR_BRACKET_EXPECTED;
        }
        return OK;


    case CTokenType::L_LHASH: // Immediate argument '#'
        if (m_procType == ProcessorType::WDC65816)
            longImm = true;
        [[fallthrough]];

    case CTokenType::L_HASH: // Immediate argument '#'
        leks = next_leks(); // Another non-empty lexeme

        ret = expression(leks, expr);
        if (ret) // Invalid expression?
            return ret;

        //if ((bProc6502 == 2) && ((expr.inf == Expr::EX_WORD) || (expr.inf == Expr::EX_UNDEF) || longImm))
        if ((m_procType == ProcessorType::WDC65816) && longImm)
            mode = A_IMM2;
        else if (expr_bit.inf == Expr::EX_WORD)
        {
            mode = A_IMM2;
            expr_bit.inf = Expr::EX_UNDEF;
        }
        else if (expr.inf != Expr::EX_BYTE && expr.inf != Expr::EX_UNDEF)
            return ERR_NUM_NOT_BYTE; // Too large number, max $FF
        else
            mode = A_IMM;

        if (!(m_procType == ProcessorType::M6502) && leks.type == CTokenType::L_COMMA) // only for M6502
        {
            leks = next_leks(); // Another non-empty lexeme

            if (leks.type == CTokenType::L_HASH)
            {
                move = (uint8_t)expr.value;
                leks = next_leks(); // Another non-empty lexeme
                ret = expression(leks, expr);

                if (ret) // Invalid expression?
                    return ret;

                if (expr.inf != Expr::EX_BYTE && expr.inf != Expr::EX_UNDEF)
                    return ERR_NUM_NOT_BYTE;

                expr.value += (move << 8); // make 2 bytes = 1 word
                mode = A_XYC;
                return OK;
            }

            if (expr.inf == Expr::EX_BYTE && abs(expr.value) > 7)
                return ERR_NOT_BIT_NUM; // Wrong bit number

            ret = expression(leks, expr_zpg);

            if (ret) // Invalid expression?
                return ret;

            expr_bit = expr; // expression -bit number

            if (expr_zpg.inf != Expr::EX_BYTE && expr_zpg.inf != Expr::EX_UNDEF)
                return ERR_NUM_NOT_BYTE; // Too large number, max $FF

            if (leks.type == CTokenType::L_COMMA) // Comma after expression?
            {
                leks = next_leks(); // Another non-empty lexeme
                ret = expression(leks, expr);

                if (ret) // Invalid expression?
                    return ret;

                mode = A_ZREL; // Address mode for BBS and BBR
            }
            else
                mode = A_ZPG2; // Address mode for RMB and SMB
        }
        return OK;

    default: // expression or nothing
        if (!is_expression(leks)) // Beginning of an expression?
        {
            mode = A_IMP_OR_ACC;
            return OK;
        }

        ret = expression(leks, expr);

        if (ret) // Invalid expression?
            return ret;

        if (leks.type != CTokenType::L_COMMA) // Does not comma after expression?
        {
            switch (expr.inf)
            {
            case Expr::EX_UNDEF: // Undefined value?
                mode = A_ABS_OR_ZPG; // Undetermined addressing mode
                break;

            case Expr::EX_BYTE: // expression < 256?
                mode = A_ZPG;
                break;

            case Expr::EX_WORD:
                mode = A_ABS;
                break;

            case Expr::EX_LONG: // $65816
                //				mode = A_ABSL;
                if (forcelong == 3)
                    mode = A_ABSL;
                else
                {
                    uint8_t exprBank = (expr.value >> 16) & 0xFF;
                    uint8_t orgBank = (origin >> 16) & 0xFF;

                    if (exprBank != orgBank) // fix using long when abs should be used
                        mode = A_ABSL;
                    else
                        mode = A_ABS; // Add test for A_ABS vs A_ABSL?
                }
                break;

            default:
                ASSERT(false);
                break;
            }

            return OK;
        }

        leks = next_leks(); // comma found

        if (leks.type != CTokenType::L_IDENT)
            return ERR_IDX_REG_EXPECTED;

        val = leks.GetString();

        if (strcasecmp(val.c_str(), s_idx_reg) == 0) // 'S'?
        {
            if ((expr.inf == Expr::EX_BYTE) || (expr.inf == Expr::EX_UNDEF))
                mode = A_SR;
            else
                return ERR_NUM_NOT_BYTE;

            leks = next_leks(); // Another non-empty lexeme
            return OK;
        }

        reg_x = false;

        if (strcasecmp(val.c_str(), x_idx_reg) == 0) // 'X'?
            reg_x = true;

        if (reg_x || (strcasecmp(val.c_str(), y_idx_reg) == 0)) // 'Y'?
        {
            switch (expr.inf)
            {
            case Expr::EX_UNDEF: // undefined expression value?
                mode = reg_x ? A_ABSX_OR_ZPGX : A_ABSY_OR_ZPGY; // Undetermined addressing mode
                break;

            case Expr::EX_BYTE: // expression < 256?
                mode = reg_x ? A_ZPG_X : A_ZPG_Y;
                break;

            case Expr::EX_WORD:
                mode = reg_x ? A_ABS_X : A_ABS_Y;
                break;

            case Expr::EX_LONG:  //$65816
                if (reg_x)
                {
                    if (forcelong == 3)
                        mode = A_ABSL_X;
                    else
                    {
                        uint8_t exprBank = (expr.value >> 16) & 0xFF;
                        uint8_t orgBank = (origin >> 16) & 0xFF;

                        if (exprBank != orgBank) // fix using long when abs should be used
                            mode = A_ABSL_X;
                        else
                            mode = A_ABS_X; // Add test for A_ABS vs A_ABSL?
                    }
                }

                //			 	    mode = A_ABSL_X;
                //				    else
                //						mode = A_ABS_X; // Add test for A_ABS vs A_ABSL?
                else
                {
                    if (forcelong == 3)
                        return ERR_IDX_REG_X_EXPECTED; // fix for locations above bank 0
                    else if (((expr.value >> 16) & 0xFF) != ((origin >> 16) & 0xFF)) // fix using long when abs should be used
                        return ERR_IDX_REG_X_EXPECTED; // fix for locations above bank 0         
                    else
                        mode = A_ABS_Y;       // fix for locations above bank 0
                    //return ERR_IDX_REG_X_EXPECTED;
                }
                break;

            default:
                ASSERT(false);
                break;
            }

            leks = next_leks(); // Another non-empty lexeme
            return OK;
        }
        else
            return ERR_IDX_REG_EXPECTED;
    }
}

/*=======================================================================*/
