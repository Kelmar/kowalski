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
#include "sim.h"

#include "M6502.h"

#include "IOWindow.h"	// this is sloppy, but right now there's no mechanism to let framework know about requested new terminal wnd size

/*=======================================================================*/

// Interpretation of the directive
CAsm6502::Stat CAsm6502::asm_instr_syntax_and_generate(CToken &leks, InstrType it, const std::string *pLabel)
{
    Stat ret;
    int def = -2;

    switch (it)
    {
    case I_ORG: // origin
    {
        Expr expr;
        ret = expression(leks, expr); // expected word

        if (ret)
            return ret;

        if (expr.inf == Expr::EX_UNDEF) // Undefined value
            return ERR_UNDEF_EXPR;

        if (expr.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (!(m_procType == ProcessorType::WDC65816) && expr.inf == Expr::EX_LONG) // Value too large
            return ERR_NUM_LONG;

        if (m_procType == ProcessorType::WDC65816)
            originWrapped = false;

        if (m_progStart == ~0u) // Beginning of program not defined yet?
        {
            m_progStart = origin = expr.value & mem_mask;

            if (markArea && pass == 2)
                markArea->SetStart(origin);
        }
        else
        {
            if (markArea && (pass == 2) && (origin != static_cast<uint32_t>(-1)))
                markArea->SetEnd(uint32_t(origin - 1));

            origin = expr.value & mem_mask;

            if (markArea && pass == 2)
                markArea->SetStart(origin);
        }
        if (pass == 2 && listing.IsOpen())
            listing.AddCodeBytes((uint32_t)origin);
        break;
    }

    case I_START: // Start of the simulator program
    {
        if (pLabel) // Is there a label before .START?
            return ERR_LABEL_NOT_ALLOWED;

        Expr expr;
        ret = expression(leks, expr); // Expected word

        if (ret)
            return ret;

        if (expr.inf == Expr::EX_UNDEF)	// Undefined value
            return pass == 1 ? OK : ERR_UNDEF_EXPR;

        if (expr.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (expr.inf == Expr::EX_LONG && (m_procType != ProcessorType::WDC65816)) // Value too large
            return ERR_NUM_LONG;

        m_progStart = expr.value & mem_mask;

        if (listing.IsOpen())
            listing.AddValue(uint32_t(m_progStart));

        break;
    }

    case I_DDW:  // 4 def DWORD 32 bit number
        def++;
        [[fallthrough]];

    case I_DX:   // 3 def long 24 bit number
        def++;
        [[fallthrough]];

    case I_DD:   // 2 def double byte
        def++;
        [[fallthrough]];

    case I_DW:   // 1 def word
        def++;
        [[fallthrough]];

    case I_DB:   // 0 def byte
        def++;
        [[fallthrough]];

    case I_DS:    // -1 def string
    case I_LS:    // -1 def long string
        def++;
        [[fallthrough]];

    case I_ASCIS: // -2 def ascii + $80
    {
        uint32_t cnt_org = origin; // Space for data length byte (.STR only)
        int cnt = 0; // Data length (info for .STR)

        if (def == -1) // If .STR then reserve a byte
        {
            if (it == I_LS)
                ret = inc_prog_counter(2);
            else
                ret = inc_prog_counter(1);

            if (ret)
                return ret; // The data does not fit in the memory of the 6502 system
        }

        for (;;)
        {
            Expr expr;
            ret = expression(leks, expr, def <= 0); // Expected expression

            if (ret)
                return ret;

            if (expr.inf == Expr::EX_STRING) // Text?
            {
                ASSERT(def <= 0); // Text only in .DB and .STR
                const std::string &str = expr.string;
                uint32_t org = origin;
                //if (origin > 0xFFFF)
                //    return ERR_UNDEF_ORIGIN;
                int len = str.size();
                cnt += len;
                ret = inc_prog_counter(len);

                if (ret)
                    return ret; // String will not fit in system memory 6502

                if (pass == 2 && out)
                {
                    for (int i = 0; org < origin; org++, i++)
                        out->set(org, str[i]);
                }
                //leks = next_leks();
            }
            else if (pass == 1)
            {
                if (def == 0 && ((expr.inf == Expr::EX_WORD) || (expr.inf == Expr::EX_LONG)))
                    return ERR_NUM_NOT_BYTE; // Too large number, max $FF

                ret = inc_prog_counter(def > 1 ? def : def == 1 ? 2 : 1);

                if (ret)
                    return ret; // The data does not fit in the memory of the 6502 system

                cnt++;
            }
            else
            {
                if (expr.inf == Expr::EX_UNDEF)
                    return ERR_UNDEF_EXPR;

                if (def == 0 && ((expr.inf == Expr::EX_WORD) || (expr.inf == Expr::EX_LONG)))
                    return ERR_NUM_NOT_BYTE; // Too large number, max $FF

                uint32_t org = origin;
                //if (origin > 0xFFFF)
                //    return ERR_UNDEF_ORIGIN;

                ret = inc_prog_counter(def > 1 ? def : def == 1 ? 2 : 1);

                if (ret)
                    return ret; // The data does not fit in the memory of the 6502 system

                if (out)
                {
                    switch (def)
                    {
                    case -1: // .str
                        cnt++;
                        [[fallthrough]];

                    case -2: // .ascis
                    case 0: // .db
                        out->set(org, (UINT)(expr.value & 0xFF));
                        break;

                    case 1: // .dw
                        out->setWord(org, expr.value);
                        break;

                    case 2: // .dd
                        out->set(org, (expr.value >> 8) & 0xFF);
                        out->set(org + 1, expr.value & 0xFF);
                        break;

                    case 3: // .dl
                        out->setLWord(org, expr.value);
                        break;

                    case 4: // .dbl
                        out->setDWord(org, expr.value);
                        break;
                    }
                }
            }

            if (leks.type != CTokenType::L_COMMA) // Next data after the decimal point (if present).
            {
                if (def == -1) // .STR ?
                {
                    if (cnt >= 256 && it == I_DS)
                        return ERR_STRING_TOO_LONG; //***

                    if ((pass == 2) && out)
                    {
                        if (it == I_LS)
                            out->setWord(cnt_org, cnt);
                        else
                            out->set(cnt_org, cnt);
                    }
                }
                else if (def == -2) // .ASCIS ?
                {
                    if (pass == 2 && out)
                    {
                        uint8_t v = (*out)[origin - 1] ^ 0x80;
                        out->set(origin - 1, v);
                    }
                }
                return OK; // There is no comma -end of data
            }

            leks = next_leks();
        }

        // Assignment in if, BUG? -- B.Simonds (April 28, 2024)
        // Unreachable -- B.Simonds (May 12, 2024)
        /*if (pass = 2 && listing.IsOpen())
            listing.AddBytes(uint32_t(cnt_org), mem_mask,out->Mem(), origin-cnt_org);*/
    }
    break;

    case I_DCB: // declare block
    {
        Expr expr;
        ret = expression(leks, expr); // Expected word

        if (ret)
            return ret;

        if (expr.inf == Expr::EX_UNDEF) // Undefined token
            return ERR_UNDEF_EXPR;

        if (expr.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (expr.inf == Expr::EX_LONG) // too high a value
            return ERR_NUM_LONG;

        uint32_t org = origin;
        ret = inc_prog_counter(expr.value);

        if (ret)
            return ret;

        if (leks.type != CTokenType::L_COMMA) // Next data after the decimal point
            return OK;

        leks = next_leks();
        Expr init;
        ret = expression(leks, init); // expected byte

        if (ret)
            return ret;

        if (init.inf == Expr::EX_UNDEF) // Undefined value?
            return pass == 1 ? OK : ERR_UNDEF_EXPR;

        if (init.inf != Expr::EX_BYTE) // Value too large?
            return ERR_NUM_NOT_BYTE;

        if (pass == 2 && out)
        {
            int len = origin - org;

            for (int i = 0; org < origin; org++, i++)
                out->set(org, init.value);

            if (len && listing.IsOpen())
                listing.AddBytes(uint32_t(org - len), mem_mask, out->Mem(), len);
        }
        break;
    }

    case I_RS: // reserve space
    {
        Expr expr;
        ret = expression(leks, expr); // Expected word

        if (ret)
            return ret;

        if (expr.inf == Expr::EX_UNDEF) // Undefined value
            return ERR_UNDEF_EXPR;

        if (expr.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (!(m_procType == ProcessorType::WDC65816) && expr.inf == Expr::EX_LONG) // Value too large
            return ERR_NUM_LONG;

        //if (origin > 0xFFFF)
        if (origin > mem_mask)
            return ERR_UNDEF_ORIGIN;

        origin += expr.value & mem_mask; // Reserved space

        if (origin > mem_mask)
            return ERR_PC_WRAPED; // "rewind" order counter

        if (pass == 2 && listing.IsOpen())
            listing.AddCodeBytes(uint32_t(origin));
        break;
    }

    case I_END: // end - why expression here?
    {
        return STAT_FIN;

        /*
        if (!is_expression(leks)) // no expression?
            return STAT_FIN;

        Expr expr;
        ret = expression(leks, expr); // Expected word

        if (ret)
            return ret;

        if (expr.inf == Expr::EX_UNDEF) // Undefined value
            return pass == 1 ? OK : ERR_UNDEF_EXPR;

        if (expr.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (expr.inf == Expr::EX_LONG) // Value too large
            return ERR_NUM_LONG;

        m_progStart = expr.value & mem_mask;
        return STAT_FIN;
        */
    }

    case I_ERROR: // Bug report
        if (pLabel) // Is there a label before .ERROR?
            return ERR_LABEL_NOT_ALLOWED;

        if (leks.type == CTokenType::L_STR)
        {
            Expr expr;
            ret = expression(leks, expr, true); // Expected text

            if (ret)
                return ret;

            if (expr.inf != Expr::EX_STRING)
                return ERR_STR_EXPECTED;

            user_error_text = expr.string;
        }
        else
            user_error_text.clear();

        return STAT_USER_DEF_ERR; // User error

    case I_INCLUDE: // Include a file
        if (pLabel) // Is there a label before .INCLUDE?
            return ERR_LABEL_NOT_ALLOWED;

        if (leks.type == CTokenType::L_STR)
        {
            Expr expr;

            ret = expression(leks, expr, true); // Expected text

            if (ret)
                return ret;

            if (expr.inf != Expr::EX_STRING)
                return ERR_STR_EXPECTED;

            wxFileName strPath(expr.string);
            m_includeFileName = strPath.GetAbsolutePath();
        }
        else
            return ERR_STR_EXPECTED; // Expected string
        return STAT_INCLUDE;

    case I_IF:
    {
        if (pLabel) // Is there a label before .IF ?
            return ERR_LABEL_NOT_ALLOWED;

        Expr expr;
        ret = expression(leks, expr); // Expected expression

        if (ret)
            return ret;

        //leks = next_leks();
        if (expr.inf == Expr::EX_UNDEF) // undefined value
            return check_line ? OK : STAT_IF_UNDETERMINED;

        return expr.value ? STAT_IF_TRUE : STAT_IF_FALSE;
    }

    case I_ELSE:
        if (pLabel) // Is there a label before .ELSE?
            return ERR_LABEL_NOT_ALLOWED;
        //leks = next_leks();
        return STAT_ELSE;

    case I_ENDIF:
        if (pLabel) // Is there a label before .ENDIF?
            return ERR_LABEL_NOT_ALLOWED;
        //leks = next_leks();
        return STAT_ENDIF;

    case I_MACRO: // Macro definition
    {
        if (!pLabel) // There is no label before .MACRO ?
            return ERR_MACRONAME_REQUIRED;

        if ((*pLabel)[0] == LOCAL_LABEL_CHAR) // Local labels not allowed
            return ERR_BAD_MACRONAME;

        CMacroDef *pMacro = nullptr;

        if (pass == 1)
        {
            pMacro = get_new_macro_entry(); // Space for a new macro definition
            CIdent tmpIdent(CIdent::I_MACRONAME, get_last_macro_entry_index());
            ret = def_macro_name(*pLabel, _Inout_ tmpIdent);

            if (ret)
                return ret;

            if (!check_line)
                pMacro->SetFileUID(text->GetFileUID());
        }
        else if (pass == 2)
        {
            ret = chk_macro_name(*pLabel);

            if (ret)
                return ret;

            return STAT_MACRO; // The macro has already been recorded
        }

        ASSERT(pMacro);
        pMacro->m_strName = *pLabel; // Remembering the macro name in the macro definition description

        for (bool bRequired = false;;)
        {
            if (leks.type == CTokenType::L_IDENT) // Parameter name?
            {
                if (pMacro->AddParam(leks.GetString()) < 0)
                    return ERR_PARAM_ID_REDEF; // Repeated parameter name
            }
            else if (leks.type == CTokenType::L_MULTI) // Ellipsis?
            {
                pMacro->AddParam(MULTIPARAM);
                leks = next_leks();
                break;
            }
            else
            {
                if (bRequired) // After the decimal point, the required macro parameter
                    return ERR_PARAM_DEF_REQUIRED;
                break;
            }
            leks = next_leks();

            if (leks.type == CTokenType::L_COMMA)
            {
                leks = next_leks();
                bRequired = true;
            }
            else
                break;
        }

        in_macro = pMacro; // Macro currently being recorded
        return STAT_MACRO;
    }

    case I_ENDM: // end of macro definition
        if (pLabel)
            return ERR_LABEL_NOT_ALLOWED;
        return ERR_SPURIOUS_ENDM; // .ENDM without changing .MACRO

    case I_EXITM: // Leaving the macro definition
        if (pLabel)
            return ERR_LABEL_NOT_ALLOWED;
        return expanding_macro ? STAT_EXITM : ERR_SPURIOUS_EXITM;

    case I_SET: // assigning a variable value
    {
        if (!pLabel) // There is no label before .SET?
            return ERR_LABEL_EXPECTED;

        Expr expr;
        ret = expression(leks, expr); // Expected expression

        if (ret)
            return ret;

        CIdent::IdentInfo info = expr.inf == Expr::EX_UNDEF ? CIdent::I_UNDEF : CIdent::I_VALUE;

        CIdent t1(info, expr.value, true);
        CIdent t2(info, expr.value, true);

        ret = pass == 1 ? def_ident(*pLabel, _Inout_ t1) : chk_ident_def(*pLabel, _Inout_ t2);

        if (ret)
            return ret;

        if (pass == 2 && listing.IsOpen())
            listing.AddValue(uint32_t(expr.value));
        break;
    }

    case I_REPEAT:			// powt�rka wierszy
    {
        Expr expr;
        ret = expression(leks, expr);	// oczekiwane wyra�enie
        if (ret)
            return ret;
        if (expr.inf == Expr::EX_UNDEF)	// nieokre�lona warto��
            return ERR_UNDEF_EXPR;
        if (expr.value < 0 || expr.value > 0xFFFF)
            return ERR_BAD_REPT_NUM;
        reptInit = expr.value;
        return STAT_REPEAT;
    }

    case I_ENDR: // End of replay
        return ERR_SPURIOUS_ENDR; // .EDR without prior .REPEAT

    case I_OPT: // Assembler options
    {
        if (pLabel)
            return ERR_LABEL_NOT_ALLOWED;

        static const char *opts[] =
        {
            "Proc6502",
            "Proc65c02",
            "Proc6501",
            "Proc65816",
            "CaseSensitive",
            "CaseInsensitive",
            "SwapBin"
        };

        for (;;)
        {
            if (leks.type == CTokenType::L_IDENT) // option name?
            {
                std::string literal = leks.GetString();

                if (strcasecmp(literal.c_str(), opts[0]) == 0)
                    m_procType = ProcessorType::M6502;
                else if (strcasecmp(literal.c_str(), opts[1]) == 0)
                    m_procType = ProcessorType::WDC65C02;
                else if (strcasecmp(literal.c_str(), opts[2]) == 0)
                    m_procType = ProcessorType::WDC65C02;
                else if (strcasecmp(literal.c_str(), opts[3]) == 0)
                    m_procType = ProcessorType::WDC65816;
                else if (strcasecmp(literal.c_str(), opts[4]) == 0)
                    case_insensitive = false;
                else if (strcasecmp(literal.c_str(), opts[5]) == 0)
                    case_insensitive = true;
                else if (strcasecmp(literal.c_str(), opts[6]) == 0)
                    swap_bin = true;
                else
                    return ERR_OPT_NAME_UNKNOWN; // Unrecognized option name
            }
            else
                return ERR_OPT_NAME_REQUIRED; // Expected option name

            leks = next_leks();

            if (leks.type == CTokenType::L_COMMA)
                leks = next_leks();
            else
                break;
        }
        break;
    }

    case I_ROM_AREA: // protected memory area
    {
        Expr addr_from;
        ret = expression(leks, addr_from); // Expected word

        if (ret)
            return ret;

        if (addr_from.inf == Expr::EX_UNDEF) // Undefined value
        {
            if (pass == 2)
                return ERR_UNDEF_EXPR;
        }
        else if (addr_from.value < 0)
            return ERR_NUM_NEGATIVE; // expected non-negative value
        //else if (addr_from.inf == Expr::EX_LONG) // Value too large
        else if ((uint32_t)addr_from.value > mem_mask)		// 65816
            return ERR_NUM_LONG;

        if (leks.type != CTokenType::L_COMMA) // Next data after the decimal point
            return ERR_CONST_EXPECTED;

        leks = next_leks();

        Expr addr_to;
        ret = expression(leks, addr_to); // expected word

        if (ret)
            return ret;

        if (addr_to.inf == Expr::EX_UNDEF) // Undefined value
        {
            if (pass == 2)
                return ERR_UNDEF_EXPR;
        }
        else if (addr_to.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value
        //else if (addr_to.inf == Expr::EX_LONG) // Value too large?
        else if ((uint32_t)addr_to.value > mem_mask) // Value too large?
            return ERR_NUM_LONG;

        //if (pass == 2) // do it once (avoid first pass; it's called for line checking)
        if (!check_line)
        {
            if (addr_from.value > addr_to.value) // valid range?
                return ERR_NO_RANGE;

            CSym6502::s_bWriteProtectArea = addr_to.value != addr_from.value;
            if (CSym6502::s_bWriteProtectArea)
            {
                CSym6502::s_uProtectFromAddr = addr_from.value;
                CSym6502::s_uProtectToAddr = addr_to.value;
            }
        }
    }
    break;

    case I_IO_WND: // size of terminal window
    {
        Expr width;
        ret = expression(leks, width); // Expected word

        if (ret)
            return ret;

        if (width.inf == Expr::EX_UNDEF) // Undefined value
        {
            if (pass == 2)
                return ERR_UNDEF_EXPR;
        }
        else if (width.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value
        else if (width.inf != Expr::EX_BYTE) // Value too large?
            return ERR_NUM_NOT_BYTE;

        if (leks.type != CTokenType::L_COMMA) // Next data after the decimal point
            return ERR_CONST_EXPECTED;

        leks = next_leks();
        Expr height;
        ret = expression(leks, height); // expected word

        if (ret)
            return ret;

        if (height.inf == Expr::EX_UNDEF) // Undefined value
        {
            if (pass == 2)
                return ERR_UNDEF_EXPR;
        }
        else if (height.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value
        else if (height.inf != Expr::EX_BYTE) // Value too large?
            return ERR_NUM_NOT_BYTE;

        if (pass == 2) // do it once (avoid first pass; it's called for line checking)
        {
            ASSERT(width.value >= 0 && height.value >= 0);

            if (width.value == 0 || height.value == 0)
                return ERR_NUM_ZERO;

            // TODO: potential problem if window exists
            //CIOWindow::m_nInitW = width.value;
            //CIOWindow::m_nInitH = height.value;
        }
    }
    break;

    case I_DATE:
    {
        Expr delim;
        ret = expression(leks, delim); // Expected word

        if (ret)
        {
            delim.value = '-';
            delim.inf = Expr::EX_BYTE;
        }

        //return ret;
        if (delim.inf == Expr::EX_UNDEF) // Undefined value
        {
            delim.value = '-';
            delim.inf = Expr::EX_BYTE;
        }

        //return ERR_UNDEF_EXPR;
        if (delim.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (delim.inf != Expr::EX_BYTE) // Value too large?
            return ERR_NUM_NOT_BYTE;

        uint32_t org = origin;
        int cnt = 0; // Data length (info for .STR)
        time_t now = time(0);
        tm *ltm = localtime(&now);
        wxString cs;

        if (delim.value == 0)
            cs.Printf("%d%02d%02d", ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday);
        else
            cs.Printf("%d%c%02d%c%02d", ltm->tm_year + 1900, delim.value, ltm->tm_mon + 1, delim.value, ltm->tm_mday);

        int len = cs.size();
        cnt += len;
        ret = inc_prog_counter(len);

        if (ret)
            return ret; // String will not fit in system memory 6502

        if (pass == 2 && out)
        {
            for (int i = 0; org < origin; org++, i++)
                out->set(org, cs[i]);
        }
    }
    break;

    case I_TIME:
    {
        Expr delim;
        ret = expression(leks, delim); // Expected word

        if (ret)
        {
            delim.value = ':';
            delim.inf = Expr::EX_BYTE;
        }

        //	return ret;
        if (delim.inf == Expr::EX_UNDEF) // Undefined value
        {
            delim.value = ':';
            delim.inf = Expr::EX_BYTE;
        }

        //return ERR_UNDEF_EXPR;
        if (delim.value < 0)
            return ERR_NUM_NEGATIVE; // Expected non-negative value

        if (delim.inf != Expr::EX_BYTE) // Value too large?
            return ERR_NUM_NOT_BYTE;

        uint32_t org = origin;
        int cnt = 0; // Data length (info for .STR)
        time_t now = time(0);
        tm *ltm = localtime(&now);

        wxString cs;

        if (delim.value == 0)
            cs.Printf("%02d%02d%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        else
            cs.Printf("%02d%c%02d%c%02d", ltm->tm_hour, delim.value, ltm->tm_min, delim.value, ltm->tm_sec);

        int len = cs.size();
        cnt += len;
        ret = inc_prog_counter(len);

        if (ret)
            return ret; // String will not fit in system memory 6502

        if (pass == 2 && out)
        {
            for (int i = 0; org < origin; org++, i++)
                out->set(org, cs[i]);
        }
    }
    break;

    default:
        ASSERT(false);
        break;
    }

    return OK;
}

/*=======================================================================*/
