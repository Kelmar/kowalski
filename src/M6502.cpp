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

#include "resource.h"
#include "MarkArea.h"
#include "IOWindow.h"	// this is sloppy, but right now there's no mechanism to let framework know about requested new terminal wnd size

#include "ConditionalAsm.h"

bool CAsm6502::case_insensitive = false; // true -> small/capital letters in label names are treated as same
bool CAsm6502::swapbin = false;
uint8_t CAsm6502::forcelong = 0;
bool CAsm6502::generateBRKExtraByte = false; // generate extra byte after BRK command?
uint8_t CAsm6502::BRKExtraByte = 0x0; // value of extra byte generated after BRK command

/*************************************************************************/

void CAsm6502::init_members()
{
    if (bProc6502 == 2)
        mem_mask = 0xFFFFFF; // memory limit mask  $65816
    else
        mem_mask = 0xFFFF; // memory limit mask  $6502

    abort_asm = false;
    program_start = ~0u;
    check_line = false;
    in_macro = NULL;
    expanding_macro = NULL;
    repeating = NULL;
    reptNested = 0;
    originWrapped = false;
    pRept = NULL;
}

void CAsm6502::init()
{
    if (out == NULL)
    {
        out = ::new COutputMem;
        temporary_out = true;
    }
    else
        temporary_out = false;
    init_members();
}

/*************************************************************************/

CLeksem CAsm6502::next_leks(bool nospace) // Get the next symbol
{
    if (!ptr)
        return CLeksem(CLeksem::L_FIN);

    char c = *ptr++;

    switch (c)
    {
    case '\\':			// \1, \2, \3 override for argument size
        if (*ptr == '1')
            forcelong = 1;
        else if (*ptr == '2')
            forcelong = 2;
        else if (*ptr == '3')
            forcelong = 3;
        else
            return CLeksem(CLeksem::L_ERROR);

        ptr++;
        return next_leks();

    case '\n':
    case '\r':
        return CLeksem(CLeksem::L_CR);

    case '\0':
        ptr--;
        return CLeksem(CLeksem::L_FIN);

    case '$':
        if (!isxdigit(*ptr)) // the '$' character at the end of a macro parameter?
            return CLeksem(CLeksem::L_STR_ARG);
        break;

    case ';':
        return CLeksem(CLeksem::L_COMMENT);

    case ':':
        return CLeksem(CLeksem::L_LABEL);

    case '=':
        if (*ptr == '=') // operator '==' equal?
        {
            ptr++;
            return CLeksem(O_EQ);
        }
        return CLeksem(CLeksem::L_EQUAL);

    case '\'':
        return get_char_num();

    case '"':
        return get_string('"');

    case ',':
        return CLeksem(CLeksem::L_COMMA);

    case '(':
        return CLeksem(CLeksem::L_BRACKET_L);

    case ')':
        return CLeksem(CLeksem::L_BRACKET_R);

    case '[':
        return CLeksem(CLeksem::L_LBRACKET_L); // 65816

    case ']':
        return CLeksem(CLeksem::L_LBRACKET_R); // 65816

    case '{':
        return CLeksem(CLeksem::L_EXPR_BRACKET_L); // 65816

    case '}':
        return CLeksem(CLeksem::L_EXPR_BRACKET_R); // 65816

    case '>':
        if (*ptr == '>') // operator '>>'
        {
            ptr++;
            return CLeksem(O_SHR);
        }
        else if (*ptr == '=') // operator '>='
        {
            ptr++;
            return CLeksem(O_GTE);
        }
        return CLeksem(O_GT);

    case '<':
        if (*ptr == '<') // operator '<<'
        {
            ptr++;
            return CLeksem(O_SHL);
        }
        else if (*ptr == '=') // operator '<='
        {
            ptr++;
            return CLeksem(O_LTE);
        }
        return CLeksem(O_LT);

    case '&':
        if (*ptr == '&') // operator '&&' ?
        {
            ptr++;
            return CLeksem(O_AND);
        }
        return CLeksem(O_B_AND);

    case '|':
        if (*ptr == '|') // operator '||' ?
        {
            ptr++;
            return CLeksem(O_OR);
        }
        return CLeksem(O_B_OR);

    case '^':
        return CLeksem(O_B_XOR);

    case '+':
        return CLeksem(O_PLUS);

    case '-':
        return CLeksem(O_MINUS);

    case '*':
        if (*ptr=='=')	// operator '*=' .ORG?
        {
            ptr++;
            return CLeksem(I_ORG);
        }
        return CLeksem(O_MUL);

    case '/':
        return CLeksem(O_DIV);

    case '%':
        if (!swapbin) return CLeksem(O_MOD);
        break;

    case '@':
        if (swapbin) return CLeksem(O_MOD);
        break;

    case '~':
        return CLeksem(O_B_NOT);

    case '!':
        if (*ptr=='=')	// operator '!='?
        {
            ptr++;
            return CLeksem(O_NE);
        }
        else if (*ptr=='#')	// operator '!#' ?
        {
            ptr++;
            return CLeksem(CLeksem::L_LHASH);
        }
        return CLeksem(O_NOT);

    case '#':
        return CLeksem(CLeksem::L_HASH);

    case '.':
        if (*ptr == '=') // operator '.='?
        {
            ptr++;
            return CLeksem(I_SET);
        }
        else if (ptr[0] == '.' && ptr[1] == '.') // Ellipsis '...' ?
        {
            ptr += 2;
            return CLeksem(CLeksem::L_MULTI);
        }
        break;
    };

    if (isspace(c))
    {
        if (!nospace) // Return the L_SPACE token?
            return eat_space();
        eat_space();
        return next_leks();
    }
    else if (isdigit(c)) // a decimal digit?
    {
        ptr--;
        return get_dec_num();
    }
    else if (c == '$') // number hex?
        return get_hex_num();
    else if (!swapbin && c == '@') // number bin?
        return get_bin_num();
    else if (swapbin && c == '%') // number bin?
        return get_bin_num();
    else if (isalpha(c) || c == '_' || c == '.' || c == '?') // || c == '$') - this is dead, cannot get here due to 4 lines above
    {
        ptr--;
        //const CLeksem &leks=
        std::string* pStr = get_ident();
        if (pStr == nullptr)
            return CLeksem(CLeksem::ERR_BAD_CHR);

        //% Bug Fix 1.2.12.18 - .commands commented out
        OpCode code;
        InstrType it;

        if (c == '.') // This could be a directive
        {
            if (asm_instr(*pStr, it))  // only need to do this if c='.'
            {
                if (it == CAsm::I_DB) //***
                {
                    delete pStr;
                    return CLeksem(it);
                }
                else
                {
                    delete pStr;
                    return CLeksem(it);
                }
            }
        }

        if (pStr->size() == 3 && proc_instr(*pStr, code)) // This could be a statement
        {
            delete pStr;
            return CLeksem(code);
        }
        else if (*pStr == "high")
        {
            delete pStr;
            return CLeksem(O_GT);
        }
        else if (*pStr == "low")
        {
            delete pStr;
            return CLeksem(O_LT);
        }

        if (*ptr == '#') // '#' character at the end of the label?
        {
            ptr++;
            return CLeksem(pStr, 1L); // Numbered identifier (expected number after '#')
        }
        return CLeksem(pStr, 1);	// L_IDENT
    }

    return CLeksem(CLeksem::L_UNKNOWN); // Unknown character -error
}

/*************************************************************************/

CLeksem CAsm6502::get_hex_num() // Interpretation of a hexadecimal number
{
    uint32_t val = 0;
    const char *tmp = ptr;

    if (!isxdigit(*ptr))
    {
        err_start = tmp;
        return CLeksem(CLeksem::ERR_NUM_HEX); // Expected hexadecimal digit
    }

    do
    {
        if (val & 0xF0000000)
        {
            err_start = tmp;
            return CLeksem(CLeksem::ERR_NUM_BIG); // Exceeding the range of 32-bit numbers
        }

        char c= *ptr++;
        val <<= 4;

        if (c >= 'a')
            val += c - 'a' + 10;
        else if (c >= 'A')
            val += c - 'A' + 10;
        else
            val += c - '0';
    }
    while (isxdigit(*ptr));

    return CLeksem(CLeksem::N_HEX, int32_t(val));
}

/*************************************************************************/

CLeksem CAsm6502::get_dec_num() // Interpretation of a decimal number
{
    uint32_t val = 0;
    const char *tmp = ptr;

    if (!isdigit(*ptr))
    {
        err_start = tmp;
        return CLeksem(CLeksem::ERR_NUM_DEC); // Expected digit
    }

    do
    {
        if (val > ~0u / 10)
        {
            err_start = tmp;
            return CLeksem(CLeksem::ERR_NUM_BIG); // Exceeding the range of 32-bit numbers
        }

        val *= 10;
        val += *ptr++ - '0';
    }
    while (isdigit(*ptr));

    return CLeksem(CLeksem::N_DEC, int32_t(val));
}

/*************************************************************************/

CLeksem CAsm6502::get_bin_num() // Interpretation of binary number
{
    uint32_t val = 0;
    const char *tmp = ptr;

    if (*ptr != '0' && *ptr != '1')
    {
        err_start = tmp;
        return CLeksem(CLeksem::ERR_NUM_HEX); // Expected hexadecimal digit
    }

    do
    {
        if (val & 0x80000000u)
        {
            err_start = tmp;
            return CLeksem(CLeksem::ERR_NUM_BIG); // Exceeding the range of 32-bit numbers
        }

        val <<= 1;
        if (*ptr++ == '1')
            val++;

    }
    while (*ptr == '0' || *ptr == '1');

    return CLeksem(CLeksem::N_BIN, int32_t(val));
}

/*************************************************************************/

CLeksem CAsm6502::get_char_num() // interpretation of the character constant
{
    char c1 = *ptr++; // first character in apostrophe

    if (*ptr != '\'')
    {
        if (*ptr == '\t' || *ptr == ' ' || *ptr == '\0' || *ptr == '\n')	// end of line?
            return CLeksem(CLeksem::N_CHR2, c1 & 0xFF);

        char c2 = *ptr++;
        if (*ptr != '\'')
        {
            err_start = ptr - 2;
            return CLeksem(CLeksem::ERR_NUM_CHR);
        }
        ptr++; // omitting the closing apostrophe
        return CLeksem(CLeksem::N_CHR2, ((c2 & 0xFF) << 8) + (c1 & 0xFF));
    }
    else
    {
        ptr++; // omitting the closing apostrophe
        return CLeksem(CLeksem::N_CHR, c1 & 0xFF);
    }
}

/*************************************************************************/

//CLeksem
std::string* CAsm6502::get_ident()
{
    const char *start = ptr;
    char c = *ptr++;

    if (!(isalpha(c) || c == '_' || c == '.' || c == '?'))
    {
        err_start = start;
        return nullptr;  //CLeksem(CLeksem::ERR_BAD_CHR);
    }

    while (isalnum(*ptr) || *ptr == '.') // Letter, digit or '_'
        ptr++;

    std::string *pstr = new std::string(start, ptr - start);
    ident_start = start; // Remembering the position of the identifier on the line
    ident_fin = ptr;

//  return CLeksem(pstr,0);
    return pstr;
}

/*************************************************************************/

CLeksem CAsm6502::get_string(char lim) // extracting a string of characters
{
    const char *fin = strchr(ptr, lim);

    if (fin == nullptr)
    {
        err_start = ptr;
        return CLeksem(CLeksem::ERR_STR_UNLIM);
    }

    std::string *pstr = new std::string(ptr, fin - ptr);

    ptr += *(fin + 1);

    return CLeksem(pstr);
}

/*************************************************************************/

CLeksem CAsm6502::eat_space()
{
    ptr--;
    while (isspace(*++ptr) && *ptr != '\n' && *ptr != '\r')
        ;

    // Whitespace characters (but not CR)
    return CLeksem(CLeksem::L_SPACE);
}

int CAsm6502::asm_str_key_cmp(const void *elem1, const void *elem2)
{
    return strcasecmp(((CAsm6502::ASM_STR_KEY *)elem1)->str, ((CAsm6502::ASM_STR_KEY *)elem2)->str);
}

/*************************************************************************/

bool CAsm6502::asm_instr(const std::string &str, InstrType &it)
{
    // Check whether 'str' is an assembler directive
    //% Bug Fix 1.2.12.18 - .commands commented out
    static const ASM_STR_KEY instr[] =
    {
        // Assembly language directives in alphabetical order
        ".ASCII",	I_DB,		// def byte
        ".ASCIS",	I_ASCIS,	// ascii + $80 ostatni bajt
        ".BYTE",	I_DB,
        ".DATE",	I_DATE,		// insert date data
        ".DB",		I_DB,		// def byte
        ".DBYTE",	I_DD,		// def double byte
        ".DCB",		I_DCB,		// declare block
        ".DD",		I_DD,		// def double byte
        ".DDW",     I_DDW,      // 32 bit word
        ".DS",		I_RS,		// reserve space (define space)
        ".DW",		I_DW,		// def word
        ".DWORD",	I_DDW,      // 32 bit word
        ".DX",      I_DX,		// 24 bit number
        ".ELSE",	I_ELSE,
        ".END",		I_END,		// ending the program (file)
        ".ENDIF",	I_ENDIF,	// end .IF
        ".ENDM",	I_ENDM,		// end .MACRO
        ".ENDR",	I_ENDR,		// end .REPEAT
        ".EQU",		I_SET,		// value assignment
        ".ERROR",	I_ERROR,	// Error report
        ".EXITM",	I_EXITM,	// end of macro expansion
        ".IF",		I_IF,		// conditional assembly
        ".INCLUDE",	I_INCLUDE,	// including the file in assembly
        ".IO_WND",	I_IO_WND,	// I/O terminal window size
        ".LSTR",	I_LS,		// Long string
        ".LSTRING",	I_LS,		// Long string
        ".MACRO",	I_MACRO,	// makrodefinicja
        ".OPT",		I_OPT,		// opcje asemblera
        ".ORG",		I_ORG,		// origin
        ".REPEAT",	I_REPEAT,	// replay
        ".REPT",	I_REPEAT,
        ".ROM_AREA",I_ROM_AREA,	// protected memory area
        ".RS",		I_RS,		// reserve space
        ".SET",		I_SET,		// value assignment
        ".START",	I_START,	// program start (for simulator)
        ".STR",		I_DS,		// def string
        ".STRING",	I_DS,		// def string
        ".TIME",	I_TIME,		// insert time data
        ".WORD",	I_DW,		// 16 bit number
        ".XWORD",   I_DX        // 24 bit number
    };

    //% Bug Fix 1.2.12.18 - .commands commented out - next_leks changed back to only come here if 1st char is '.'
    ASM_STR_KEY find;
    //std::string temp = str[0] == '.' ? str : "." + (str[0] == '$' ? str.Right(str.size() - 1) : str);
    find.str = str.c_str();
    // end bug fix

    void *ret = bsearch(&find, instr, sizeof(instr) / sizeof(ASM_STR_KEY),
                       sizeof(ASM_STR_KEY), asm_str_key_cmp);

    if (ret)
    {
        it = ((ASM_STR_KEY *)ret)->it;
        return true;
    }

    return false;
}

/*************************************************************************/

// Loading the arguments of the macro call
CAsm::Stat CMacroDef::ParseArguments(CLeksem &leks, CAsm6502 &asmb)
{
    std::string literal;
    bool get_param = true;
    bool first_param = true;
    Stat ret;
    int count = 0;

    int required = m_nParams >= 0 ? m_nParams : -m_nParams - 1; // Number of required args.

    m_strarrArgs.clear();
    m_narrArgs.clear();
    m_arrArgType.clear();
    m_nParamCount = 0;
    //leks = asmb.next_leks();

    if (m_nParams == 0) // Parameterless macro?
        return OK;

    for (;;)
    {
        if (get_param) // Possibly another argument
        {
            switch (leks.type)
            {
            case CLeksem::L_STR:
                literal = *leks.GetString();
                m_strarrArgs.Add(literal);
                m_narrArgs.Add(literal.size());
                m_arrArgType.Add(STR);
                count++;
                get_param = false; // Parameter already interpreted
                first_param = false; // First parameter already loaded
                leks = asmb.next_leks();
                break;

            case CLeksem::L_ERROR:
                // Added to prevent looping C runtime errors if first param is ''
                // use this or ERR_PARAM_REQUIRED;
                return ERR_EMPTY_PARAM;

            default:
                if (asmb.is_expression(leks))	// expression?
                {
                    Expr expr;
                    ret = asmb.expression(leks, expr);

                    if (ret)
                        return ret;

                    if (expr.inf == Expr::EX_UNDEF)	// warto�� niezdeterminowana
                    {
                        m_strarrArgs.Add(_T(""));
                        m_narrArgs.Add(0);
                        m_arrArgType.Add(UNDEF_EXPR);
                    }
                    else if (expr.inf == Expr::EX_STRING)
                    {
                        m_strarrArgs.Add(expr.string);
                        m_narrArgs.Add(expr.string.size());
                        m_arrArgType.Add(STR);
                    }
                    else
                    {
                        wxString num;
                        num.Printf("%ld", expr.value);

                        m_strarrArgs.Add(num);
                        m_narrArgs.Add(expr.value);
                        m_arrArgType.Add(NUM);
                    }
                    count++;
                    get_param = false; // parameter already interpreted
                }
                else
                {
                    if (count < required)
                        return ERR_PARAM_REQUIRED; // Too few macro calling parameters

                    if (!first_param)
                        return ERR_PARAM_REQUIRED; // After the decimal point, you must enter another parameter

                    m_nParamCount = count;
                    return OK;
                }
            }
        }
        else // after an argument, a comma, semicolon, or end
        {
            // removed to support parameter mismatch error  v1.3.4.5
            /*			if (count==required && m_nParams>0)  // this exits even if all params are not evaluated
            			{											Comment ed out to fix too many parameters
            				m_nParamCount = count;
            				return OK;		// all required parameters already loaded
            			}
            */

            switch (leks.type)
            {
            case CLeksem::L_COMMA:
                get_param = true; // Next Parameter

                leks = asmb.next_leks();
                break;

            default:
                if (count < required)
                    return ERR_PARAM_REQUIRED;	// Too few macro calling parameters

                if (count > required && m_nParams>0) // if macro called with ..., then m_nParams will be negative
                    return ERR_MACRO_PARAM_COUNT;

                m_nParamCount = count;
                return OK;
            }
        }
    }
}

// type of macro parameter (to distinguish between numbers and strings)
CAsm::Stat CMacroDef::ParamType(const CString param_name, bool& found, int& type)
{
    CIdent ident;

    if (!param_names.lookup(param_name, ident))	// odszukanie parametru o danej nazwie
    {
        found = false;
        return OK;
    }
    return ParamType(ident.val, found, type);
}


CAsm::Stat CMacroDef::ParamType(int param_number, bool& found, int& type)
{
    if (param_number >= m_nParamCount || param_number < 0)
    {
        found = false;
        return ERR_EMPTY_PARAM;
    }

    found = true;

    ASSERT(m_arrArgType.GetSize() > param_number);
    switch (m_arrArgType[param_number])
    {
    case NUM:
        type = 1;
        break;
    case STR:
        type = 2;
        break;
    default:
        type = 0;
        break;
    }

    return OK;
}


// odszukanie parametru 'param_name' aktualnego makra
CAsm::Stat CMacroDef::ParamLookup(CLeksem &leks, const CString& param_name, Expr &expr, bool &found, CAsm6502 &asmb)
{
    CIdent ident;

    if (!param_names.lookup(param_name, ident))	// odszukanie parametru o danej nazwie
    {
        found = false;
        return OK;
    }
    found = true;
    leks = asmb.next_leks(false);
    return ParamLookup(leks, ident.val, expr, asmb);
}


// odszukanie warto�ci parametru numer 'param_number' aktualnego makra
CAsm::Stat CMacroDef::ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb)
{
    bool special= param_number == -1;	// zmienna %0 ? (nie parametr)
    if (leks.type == CLeksem::L_STR_ARG)	// odwo�anie do warto�ci znakowej parametru?
    {
        if (!special && (param_number >= m_nParamCount || param_number < 0))
            return ERR_EMPTY_PARAM;
        if (special)			// odwo�anie do %0$ -> co oznacza nazw� makra
            expr.string = m_strName;
        else
        {
            ASSERT(m_arrArgType.GetSize() > param_number);
            if (m_arrArgType[param_number] != STR)	// spr. czy zmienna ma warto�� tekstow�
                return ERR_NOT_STR_PARAM;
            ASSERT(m_strarrArgs.GetSize() > param_number);
            expr.string = m_strarrArgs[param_number];
        }
        expr.inf = Expr::EX_STRING;
        leks = asmb.next_leks();
        return OK;
    }
    else if (leks.type == CLeksem::L_SPACE)
        leks = asmb.next_leks();

    if (special)	// odwo�anie do %0 -> ilo�� aktualnych parametr�w w wywo�aniu makra
    {
        expr.inf = Expr::EX_LONG;
        expr.value = m_nParamCount;
    }
    else		// reference to the current parameter !!
    {
        if (param_number >= m_nParamCount || param_number < 0)
            return ERR_EMPTY_PARAM;		// parametru o takim numerze nie ma
        ASSERT(m_arrArgType.GetSize() > param_number);
        switch (m_arrArgType[param_number])	// aktualny typ parametru
        {
        case NUM:		// parametr liczbowy
        case STR:		// parametr tekstowy (podawana jest jego d�ugo��)
            ASSERT(m_narrArgs.GetSize() > param_number);
            expr.inf = Expr::EX_LONG;
            expr.value = m_narrArgs[param_number];
            break;
        case UNDEF_EXPR:	// parametr liczbowy, warto�� niezdefiniowana
            ASSERT(m_narrArgs.GetSize() > param_number);
            expr.inf = Expr::EX_UNDEF;
            expr.value = 0;
            break;
        default:
            ASSERT(false);
            break;
        }
    }

    return OK;
}


// spr. sk�adni odwo�ania do parametru makra (tryb sprawdzania wiersza)
CAsm::Stat CMacroDef::AnyParamLookup(CLeksem &leks, CAsm6502 &asmb)
{
    if (leks.type == CLeksem::L_STR_ARG)	// odwo�anie do warto�ci znakowej parametru?
    {
        leks = asmb.next_leks();
        return OK;
    }
    else if (leks.type == CLeksem::L_SPACE)
        leks = asmb.next_leks();

    return OK;
}


const char* CMacroDef::GetCurrLine(CString &str)	// odczyt aktualnego wiersza makra
{
    ASSERT(m_nLineNo >= 0);
    if (m_nLineNo < GetSize())
    {
        str = GetLine(m_nLineNo++);
        return (const char *)str;
    }
    else				// koniec wierszy?
    {
        ASSERT(m_nLineNo == GetSize());
        return NULL;
    }
}

//-----------------------------------------------------------------------------


CAsm6502::Stat CAsm6502::CheckLine(const char *str, int &instr_idx_start, int &instr_idx_fin)
{
    Stat ret;
    ptr = str;  //.GetBuffer(0);

    instr_idx_start = instr_idx_fin = 0;

    try
    {
        pass = 1;
        local_area = 0;
        proc_area = 0;
        macro_local_area = 0;
        origin = 0;
        ret = assemble_line();
    }
    catch (CMemoryException *)
    {
        ret = ERR_OUT_OF_MEM;
    }

    if (instr_start)
    {
        instr_idx_start = instr_start - str;
        instr_idx_fin = instr_fin - str;
    }

//  str.ReleaseBuffer();
    if (ret < OK)
        ret = OK;
//  if (ret == STAT_FIN || ret == STAT_USER_DEF_ERR || ret == STAT_INCLUDE)
//    ret = OK;
    switch (ret)	// nie wszystke b��dy s� b��dami w trybie analizowania jednego wiersza
    {
    case ERR_UNDEF_EXPR:
    case ERR_UNKNOWN_INSTR:
    case ERR_SPURIOUS_ENDM:
    case ERR_SPURIOUS_EXITM:
    case ERR_SPURIOUS_ENDR:
        ret = OK;
        break;
    }
    ASSERT(ret >= OK);

    return ret;
}


CAsm6502::Stat CAsm6502::look_for_endif()		// szukanie .IF, .ENDIF lub .ELSE
{
    CLeksem leks= next_leks(false);	// kolejny leksem, by� mo�e pusty (L_SPACE)
    bool labelled= false;

    switch (leks.type)
    {
    case CLeksem::L_IDENT:	// etykieta
    case CLeksem::L_IDENT_N:	// etykieta numerowana
        leks = next_leks();
        if (leks.type == CLeksem::L_IDENT_N)
        {
            Expr expr(0);
            Stat ret = factor(leks,expr);
            if (ret)
                return ret;
        }
        labelled = true;
        switch (leks.type)
        {
        case CLeksem::L_LABEL:	// znak ':'
            leks = next_leks();
            if (leks.type!=CLeksem::L_ASM_INSTR)
                return OK;
            break;
        case CLeksem::L_ASM_INSTR:
            break;
        default:
            return OK;
        }
        //      leks = next_leks();
        break;

    case CLeksem::L_SPACE:	// odst�p
        leks = next_leks();
        if (leks.type!=CLeksem::L_ASM_INSTR)	// nie dyrektywa asemblera?
            return OK;
        break;

    case CLeksem::L_COMMENT:	// komentarz
    case CLeksem::L_CR:		// koniec wiersza
        return OK;

    case CLeksem::L_FIN:	// koniec tekstu
        return STAT_FIN;
        break;

    default:
        return ERR_UNEXP_DAT;
    }

    ASSERT(leks.type==CLeksem::L_ASM_INSTR);	// dyrektywa asemblera

    switch (leks.GetInstr())
    {
    case I_IF:
        return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_IF_UNDETERMINED;
    case I_ELSE:
        return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_ELSE;
    case I_ENDIF:
        return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_ENDIF;
    default:
        return OK;
    }
}


CAsm6502::Stat CAsm6502::assemble_line()	// interpretacja wiersza
{
    enum			// stany automatu
    {
        START,
        AFTER_LABEL,
        INSTR,
        EXPR,
        COMMENT,
        FINISH
    } state= START;
    CString label;	// pomocnicza zmienna do zapami�tania identyfikatora
    bool labelled= false;	// flaga wyst�pienia etykiety
    Stat ret, ret_stat= OK;
    instr_start = NULL;
    instr_fin = NULL;
    ident_start = NULL;
    ident_fin = NULL;

    CLeksem leks= next_leks(false);	// kolejny leksem, by� mo�e pusty (L_SPACE)

    for (;;)
    {
        switch (state)
        {
        case START:			// pocz�tek wiersza
            switch (leks.type)
            {
            case CLeksem::L_IDENT:	// etykieta
                label = *leks.GetString();	// zapami�tanie identyfikatora
                state = AFTER_LABEL;
                leks = next_leks();
                break;
            case CLeksem::L_IDENT_N:	// etykieta numerowana
            {
                CLeksem ident(leks);
                Expr expr(0);
                leks = next_leks();
                ret = factor(leks,expr);
                if (ret)
                    return ret;
                if (!check_line)
                {
                    if (expr.inf==Expr::EX_UNDEF)	// nieokre�lona warto��
                        return ERR_UNDEF_EXPR;
                    ident.Format(expr.value);	// znormalizowanie postaci etykiety
                    label = *ident.GetString();	// zapami�tanie identyfikatora
                }
                else
                    label = _T("x");
                state = AFTER_LABEL;
                break;
            }
            case CLeksem::L_SPACE:	// po odst�pie ju� nie mo�e by� etykiety
                state = INSTR;
                leks = next_leks();
                break;
            case CLeksem::L_COMMENT:	// komentarz
                state = COMMENT;
                break;
            case CLeksem::L_CR:		// koniec wiersza
            case CLeksem::L_FIN:		// koniec tekstu
                state = FINISH;
                break;
            default:
                return ERR_UNEXP_DAT;	// nierozpoznany napis
            }
            break;


        case AFTER_LABEL:			// wyst�pi�a etykieta
            switch (leks.type)
            {
            case CLeksem::L_SPACE:
                ASSERT(false);
                break;
            case CLeksem::L_LABEL:	// znak ':'
                state = INSTR;
                leks = next_leks();
                break;
            case CLeksem::L_EQUAL:	// znak '='
                state = EXPR;
                leks = next_leks();
                break;
            default:
                state = INSTR;
                break;
            }
            labelled = true;
            break;


        case INSTR:			// oczekiwana instrukcja, komentarz lub nic
            if (labelled &&					// przed instr. by�a etykieta
                    !(leks.type == CLeksem::L_ASM_INSTR &&	// i za etykiet�
                      (leks.GetInstr() == I_MACRO ||		// nie wyst�puje dyrektywa .MACRO
                       leks.GetInstr() == I_SET)))			// ani dyrektywa .SET?
            {
                if (origin > mem_mask) //65816

                    return ERR_UNDEF_ORIGIN;
                ret = pass == 1 ? def_ident(label, CIdent(CIdent::I_ADDRESS, origin)) :
                      chk_ident_def(label, CIdent(CIdent::I_ADDRESS, origin));
                if (ret)
                    return ret;
            }

            switch (leks.type)
            {
            case CLeksem::L_SPACE:
                ASSERT(false);
                break;
            case CLeksem::L_ASM_INSTR:	// dyrektywa asemblera
            {
                InstrType it= leks.GetInstr();
                instr_start = ident_start;	// po�o�enie instrukcji w wierszu
                instr_fin = ident_fin;

                leks = next_leks();
                ret_stat = asm_instr_syntax_and_generate(leks,it,labelled ? &label : NULL);
                if (ret_stat > OK)		// b��d? (ERR_xxx)
                    return ret_stat;
                if (pass==2 && out && debug &&
                        ret_stat!=STAT_MACRO && ret_stat!=STAT_EXITM && it!=I_SET &&
                        ret_stat!=STAT_REPEAT && ret_stat!=STAT_ENDR)
                {
                    ret = generate_debug(it,text->GetLineNo(),text->GetFileUID());
                    if (ret)
                        return ret;
                }
                if (pass==2 && ret_stat==STAT_MACRO)
                    return ret_stat;		// in the second pass we skip .MACRO
                state = COMMENT;
                break;
            }

            case CLeksem::L_PROC_INSTR:	// processor order (opcode)
            {
                instr_start = ident_start;	// the location of the instructions on the line
                instr_fin = ident_fin;
                OpCode code= leks.GetCode();	// order no
                CodeAdr mode;
                Expr expr, expr_bit, expr_zpg;
                leks = next_leks();
                ret = proc_instr_syntax(leks,mode,expr,expr_bit,expr_zpg);  // for 3 byte operands!
                forcelong = 0;
                if (ret)			// syntax error
                    return ret;
                int len;			// the length of the command with the argument
                ret = chk_instr_code(code,mode,expr,len);

                if (ret)
                    return ret;		// error in the addressing or range mode
                if (pass==2 && out && debug)
                {
//						if (origin > 0xFFFF)
                    if (origin > mem_mask)
                        return ERR_UNDEF_ORIGIN;
                    CMacroDef* pMacro= dynamic_cast<CMacroDef*>(text);
                    if (pMacro && pMacro->m_bFirstCodeLine)
                    {
                        // debug info for the first line of the macro containing the instr. 6502
                        pMacro->m_bFirstCodeLine = false;
                        generate_debug((uint32_t)origin,pMacro->GetFirstLineNo(),pMacro->GetFirstLineFileUID());
                    }
                    else
                        generate_debug((uint32_t)origin,text->GetLineNo(),text->GetFileUID());
                    generate_code(code,mode,expr,expr_bit,expr_zpg);
                }
                ret = inc_prog_counter(len);
                if (ret)
                    return ret;
                state = COMMENT;		// dozwolony ju� tylko komentarz
                break;
            }

            case CLeksem::L_IDENT:	// label - here only as a macro name
            case CLeksem::L_IDENT_N:	// numbered label - here only as a macro name
            {
                if (leks.type == CLeksem::L_IDENT)
                {
                    label = *leks.GetString();	// remembering the identifier
                    leks = next_leks();
                }
                else
                {
                    CLeksem ident(leks);
                    Expr expr(0);
                    leks = next_leks();
                    ret = factor(leks,expr);
                    if (ret)
                        return ret;
                    if (expr.inf==Expr::EX_UNDEF)	// undefined value
                        return ERR_UNDEF_EXPR;
                    ident.Format(expr.value);		// standardizing the form of the label
                    label = *ident.GetString();	// remembering the identifier
                }
                CIdent macro;
                if (!macro_name.lookup(label, macro))	// if the label is not in the array
                    return ERR_UNKNOWN_INSTR;
                ASSERT(macros.GetSize() > macro.val && macro.val >= 0);
                CMacroDef* pMacro= &macros[macro.val];
                ret = pMacro->ParseArguments(leks,*this);	// load macro arguments
                if (ret)
                    return ret;
                if (label == "proc")
                    proc_area++;
                // remember condition nesting level
                pMacro->Start(&conditional_asm, text->GetLineNo(), text->GetFileUID());
                source.Push(text);		// the current row source on the stack
                text = expanding_macro = pMacro;
                //	    text->Start();
                macro_local_area++;		// new local area for macro labels
                //	    MacroExpandStart(pMacro);	// switch to macro development mode
                state = COMMENT;		// only comment allowed
                break;
            }

            case CLeksem::L_CR:
            case CLeksem::L_FIN:
                state = FINISH;
                break;
            case CLeksem::L_COMMENT:	// komentarz
                state = COMMENT;
                break;
            default:
                return ERR_INSTR_OR_NULL_EXPECTED;
            }
            break;


        case EXPR:			// expected expression - val. labels
            switch (leks.type)
            {
            case CLeksem::L_SPACE:
                ASSERT(false);
                break;

            default:
                Expr expr;
                ret = expression(leks, expr);// interpreting the expression
                if (ret)
                    return ret;

                // is it predefined label?
                int nConstant= find_const(label);
                if (nConstant >= 0)
                {
                    // assignment to io_area is fine
                    if (nConstant == 1)	// io_area label?
                    {
                        if (expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE)
                        {
                            if (!check_line)
//							if (pass == 2)		// do it once (avoid first pass; it's called for line checking)
                                CSym6502::io_addr = uint16_t(expr.value);
                        }
                        else if (expr.inf == Expr::EX_LONG)
                            return ERR_NUM_LONG;
                        else if (expr.inf == Expr::EX_UNDEF)
                            ; // not yet defined; this is fine
                        else
                            return ERR_CONST_EXPECTED;
                    }
                    else
                    {
                        err_ident= label;
                        return ERR_CONST_LABEL_REDEF;
                    }
                }
                else if (pass==1)		// first pass?
                {
                    if (expr.inf != Expr::EX_UNDEF)
                        ret = def_ident(label, CIdent(CIdent::I_VALUE, expr.value));
                    else
                        ret = def_ident(label, CIdent(CIdent::I_UNDEF, 0));
                }
                else			// second pass
                {
                    if (expr.inf != Expr::EX_UNDEF)
                        ret = chk_ident_def(label, CIdent(CIdent::I_VALUE, expr.value));
                    else
                        return ERR_UNDEF_EXPR;
                    if (listing.IsOpen())
                        listing.AddValue((uint32_t)expr.value); //65816 does this need updating?
                }
                if (ret)
                    return ret;
                state = COMMENT;
                break;
            }
            break;

        case COMMENT:			// just a comment or end of line
            switch (leks.type)
            {
            case CLeksem::L_SPACE:
                ASSERT(false);
            //	    break;
            case CLeksem::L_COMMENT:
                return ret_stat;
            case CLeksem::L_CR:
            case CLeksem::L_FIN:
                state = FINISH;
                break;
            default:
                return ERR_DAT;
            }
            break;


        case FINISH:
            switch (leks.type)
            {
            case CLeksem::L_CR:
                return ret_stat;
            case CLeksem::L_FIN:
                return ret_stat ? ret_stat : STAT_FIN;
            default:
                return ERR_DAT;
            }
            break;
        }

    }

}


//-----------------------------------------------------------------------------


bool CAsm6502::is_expression(const CLeksem &leks)	// pocz�tek wyra�enia?
{
    switch (leks.type)
    {
    case CLeksem::L_NUM:			// liczba (dec, hex, bin, lub znak)
    case CLeksem::L_IDENT:			// identyfikator
    case CLeksem::L_IDENT_N:		// identyfikator numerowany
    case CLeksem::L_OPER:			// operator
    case CLeksem::L_EXPR_BRACKET_L:	// lewy nawias dla wyra�e� '['
    case CLeksem::L_EXPR_BRACKET_R:	// prawy nawias dla wyra�e� ']'
        return true;				// to pocz�tek wyra�enia

    default:
        return false;	// to nie pocz�tek wyra�enia
    }
}


int CAsm6502::find_const(const CString& str)
{
    static const char cnst1[]= "ORG";		// predefiniowana sta�a
    static const char cnst2[]= "IO_AREA";	// predefiniowana sta�a

    if (str.CompareNoCase(cnst1) == 0)
        return 0;
    if (str.CompareNoCase(cnst2) == 0)
        return 1;

    return -1;
}


CAsm6502::Stat CAsm6502::predef_const(const CString& str, Expr& expr, bool& found)
{
    int nConstant= find_const(str);

    if (nConstant == 0)
    {
//		if (origin > 0xFFFF)
        if (origin > mem_mask)
            return ERR_UNDEF_ORIGIN;
        expr.value = origin;	// warto�� licznika rozkaz�w
        found = true;
        return OK;
    }
    else if (nConstant == 1)
    {
        expr.value = CSym6502::io_addr;		// io simulator area
        found = true;
        return OK;
    }

    found = false;
    return OK;
}


CAsm6502::Stat CAsm6502::predef_function(CLeksem &leks, Expr &expr, bool &fn)
{
    static const char def[]= ".DEF";	// predefiniowana funkcja .DEF
    static const char ref[]= ".REF";	// predefiniowana funkcja .REF
    static const char strl[]= ".STRLEN";	// predefiniowana funkcja .STRLEN
    static const char pdef[]= ".PASSDEF";// predefiniowana funkcja .PASSDEF
    static const char paramtype[]= ".PARAMTYPE";// predefiniowana funkcja .PARAMTYPE

    const CString &str= *leks.GetString();
    bool LocParamNo = false;

    int hit= 0;
    if (str.CompareNoCase(def) == 0)
        hit = 1;
    else if (str.CompareNoCase(ref) == 0)
        hit = 2;
    else if (str.CompareNoCase(pdef) == 0)
        hit = 3;
    else if (str.CompareNoCase(paramtype) == 0)
        hit = 4;
    else if (str.CompareNoCase(strl) == 0)
        hit = -1;

    if (hit > 0)
    {
        leks = next_leks(false);
        if (leks.type != CLeksem::L_BRACKET_L)
            return ERR_BRACKET_L_EXPECTED;	// wymagany nawias '(' (bez odst�pu)
        leks = next_leks();
        CString Label;
        if (leks.type == CLeksem::L_IDENT)
        {
            Label = *leks.GetString();
            leks = next_leks();
        }
        else if (leks.type == CLeksem::L_IDENT_N)
        {
            CLeksem ident(leks);
            Expr expr(0);
            leks = next_leks();
            Stat ret = factor(leks,expr);
            if (ret)
                return ret;
            if (expr.inf==Expr::EX_UNDEF)	// nieokre�lona warto��
                return ERR_UNDEF_EXPR;
            ident.Format(expr.value);		// znormalizowanie postaci etykiety
            Label = *ident.GetString();	// zapami�tanie identyfikatora
        }
        else if (hit== 4 && leks.type == CLeksem::L_OPER && leks.GetOper() == O_MOD) //!! add code for numbered parameters
        {
            leks = next_leks(false);
            if (leks.type == CLeksem::L_SPACE)	// odst�p niedozwolony
                return ERR_PARAM_NUMBER_EXPECTED;
            Stat ret= factor(leks,expr,false);	// expected macro parameter number
            if (ret)
                return ret;
            if (!check_line)
                if (expr.inf == Expr::EX_UNDEF)
                    return ERR_UNDEF_PARAM_NUMBER;	// numer parametru musi by� zdefiniowany

            LocParamNo = true;
        }
        else
        {
            return ERR_LABEL_EXPECTED;	// wymagana etykieta - argument .DEF lub .REF
        }
        if (leks.type != CLeksem::L_BRACKET_R)
            return ERR_BRACKET_R_EXPECTED;	// wymagany nawias ')'

        if (Label[0] == LOCAL_LABEL_CHAR)	// etykiety lokalne niedozwolone
            return ERR_LOCAL_LABEL_NOT_ALLOWED;
        CIdent ident;

        if (case_insensitive)				// fix case sensitive issue
            Label.MakeLower();

        if (hit == 1)			// .DEF?
        {
            if (global_ident.lookup(Label,ident) && ident.info!=CIdent::I_UNDEF)
            {
                ASSERT(ident.info != CIdent::I_INIT);
                expr.value = 1;			// 1 - etykieta zdefiniowana
            }
            else
                expr.value = 0;			// 0 - etykieta niezdefiniowana
        }
        else if (hit == 2)			// .REF?
        {
            expr.value = global_ident.lookup(Label,ident) ? 1 : 0; // 1 - je�li etykieta jest w tablicy
        }
        else if (hit == 3)			// .PASSDEF?
        {
            if (global_ident.lookup(Label,ident) && ident.info!=CIdent::I_UNDEF)
            {
                ASSERT(ident.info != CIdent::I_INIT);
                if (pass==1)
                    expr.value = 1;		// 1 - etykieta zdefiniowana
                else		// drugie przej�cie asemblacji
                    expr.value = ident.checked ? 1 : 0;	// 1 - def. etykiety znaleziona w 2. przej�ciu
            }
            else		// etykieta jeszcze nie zdefiniowana
                expr.value = 0;			// 0 - etykieta niezdefiniowana
        }
        else				// .PARAMTYPE
        {
            if (expanding_macro)
            {
                bool found= false;
                int type= 0;
                Stat ret;
                if (LocParamNo)
                    ret = expanding_macro->ParamType(expr.value-1, found, type);
                else
                    ret= expanding_macro->ParamType(Label, found, type);

                if (ret)
                    return ret;
                if (!found)
                    return ERR_EMPTY_PARAM;

                expr.value = type;			// type of macro parameter (number or string)
            }
            else if (check_line)
            {
                expr.value = 0;
            }
            else
                return ERR_DAT;	//todo
        }

        leks = next_leks();
        fn = true;
        return OK;
    }
    else if (hit == -1)		// funkcja .STRLEN?
    {
        leks = next_leks(false);
        if (leks.type != CLeksem::L_BRACKET_L)
            return ERR_BRACKET_L_EXPECTED;	// wymagany nawias '(' (bez odst�pu)
        leks = next_leks();

        if (check_line && leks.type == CLeksem::L_OPER && leks.GetOper() == O_MOD)    //!! add code for numbered parameters
        {

            leks = next_leks(false);
            if (leks.type == CLeksem::L_SPACE)	// odst�p niedozwolony
                return ERR_PARAM_NUMBER_EXPECTED;
            Stat ret= factor(leks,expr,false);	// expected macro parameter number
            if (ret)
                return ret;
            if (!check_line)
                if (expr.inf == Expr::EX_UNDEF)
                    return ERR_UNDEF_PARAM_NUMBER;	// numer parametru musi by� zdefiniowany
            if (leks.type != CLeksem::L_BRACKET_R)
                return ERR_BRACKET_R_EXPECTED;	// wymagany nawias ')'
            expr.value = 0;
            leks = next_leks();
        }
        else
        {
            Expr strexpr;
            Stat ret= expression(leks,strexpr,true);
            if (ret)
                return ret;
            if (strexpr.inf != Expr::EX_STRING)
                return ERR_STR_EXPECTED;		// wymagany �a�cuch znak�w jako argument
            if (leks.type != CLeksem::L_BRACKET_R)
                return ERR_BRACKET_R_EXPECTED;	// wymagany nawias ')'
            expr.value = strlen(strexpr.string);
            leks = next_leks();
        }
        fn = true;
        return OK;
    }

    fn = false;
    return OK;
}


// warto�� sta�a - etykieta, parametr, liczba, predef. sta�a lub funkcja
CAsm6502::Stat CAsm6502::constant_value(CLeksem &leks, Expr &expr, bool nospace)
{
    switch (leks.type)
    {
    case CLeksem::L_NUM:		// liczba (dec, hex, bin, lub znak)
        expr.value = leks.GetValue();	// warto�� liczby lub znaku
        break;

    case CLeksem::L_STR:		// ci�g znak�w w cudzys�owach
        expr.string = *leks.GetString();
        expr.inf = Expr::EX_STRING;
        break;

    case CLeksem::L_IDENT:		// identyfikator
    {
        bool found= false;
        Stat ret= predef_const(*leks.GetString(), expr, found);
        if (ret)
            return ret;
        if (found)
        {
            leks = next_leks();
            return OK;
        }
        ret =  predef_function(leks,expr,found);
        if (ret)
            return ret;
        if (found)
            return OK;
        if (expanding_macro)
        {
            // przeszukanie tablicy parametr�w makra, je�li jest rozwijane makro
            Stat ret= expanding_macro->ParamLookup(leks, *leks.GetString(), expr, found, *this);
            if (ret)
                return ret;
            if (found)
                return OK;
        }
        CIdent id(CIdent::I_UNDEF);	// niezdefiniowany identyfikator
        if (!add_ident(*leks.GetString(),id) && id.info!=CIdent::I_UNDEF)	// ju� zdefiniowany?
            expr.value = id.val;		// odczytana warto�� etykiety
        else
        {
            expr.inf = Expr::EX_UNDEF;	// jeszcze bez warto�ci
            if (pass==2)
                return err_ident=*leks.GetString(), ERR_UNDEF_LABEL;	// niezdefiniowana etykieta w drugim przebiegu
        }
        if (check_line)			// tryb sprawdzania jednego wiersza?
        {
            leks = next_leks(false);
            if (leks.type == CLeksem::L_STR_ARG)	// omini�cie znaku '$' na ko�cu etykiety
                leks = next_leks();
            if (leks.type == CLeksem::L_SPACE)
                leks = next_leks();
            return OK;
        }
        break;
    }

    case CLeksem::L_IDENT_N:		// identyfikator numerowany
    {
        CLeksem ident(leks);
        Expr expr2(0);
        leks = next_leks();
        Stat ret = factor(leks,expr2);
        if (ret)
            return ret;
        if (expr2.inf==Expr::EX_UNDEF)	// nieokre�lona warto��
            return ERR_UNDEF_EXPR;
        ident.Format(expr2.value);	// znormalizowanie postaci etykiety

        CIdent id(CIdent::I_UNDEF);	// niezdefiniowany identyfikator
        if (!add_ident(*ident.GetString(),id) && id.info!=CIdent::I_UNDEF)	// ju� zdefiniowany?
            expr.value = id.val;		// odczytana warto�� etykiety
        else
        {
            expr.inf = Expr::EX_UNDEF;	// jeszcze bez warto�ci
            if (pass==2)
                return err_ident=*ident.GetString(), ERR_UNDEF_LABEL;	// niezdefiniowana etykieta w drugim przebiegu
        }
        return OK;
    }

    case CLeksem::L_OPER:		//
        if (expanding_macro && leks.GetOper() == O_MOD)	// '%' (odwo�anie do parametru makra)?
        {
            leks = next_leks(false);
            if (leks.type == CLeksem::L_SPACE)	// odst�p niedozwolony
                return ERR_PARAM_NUMBER_EXPECTED;
            Stat ret= factor(leks,expr,false);	// oczekiwany numer parametru makra
            if (ret)
                return ret;
            if (expr.inf == Expr::EX_UNDEF)
                return ERR_UNDEF_PARAM_NUMBER;	// numer parametru musi by� zdefiniowany
            ret = expanding_macro->ParamLookup(leks, expr.value - 1, expr, *this);
            if (ret)
                return ret;
            return OK;
        }
        else if (check_line && leks.GetOper() == O_MOD)	// tryb sprawdzania wiersza?
        {
            leks = next_leks(false);
            if (leks.type == CLeksem::L_SPACE)	// odst�p niedozwolony
                return ERR_PARAM_NUMBER_EXPECTED;
            Stat ret= factor(leks,expr,false);	// oczekiwany numer parametru makra
            if (ret)
                return ret;
            ret = expanding_macro->AnyParamLookup(leks,*this);
            if (ret)
                return ret;
            return OK;
        }
        else if (leks.GetOper() == O_MUL)	// '*' ?
        {
//			if (origin > 0xFFFF)
            if (origin > mem_mask)
                return ERR_UNDEF_ORIGIN;
            expr.value = origin;		// warto�� licznika rozkaz�w
            break;
        }
    // no break here
    default:
        return ERR_CONST_EXPECTED;
    }

    leks = next_leks(nospace);
    return OK;
}


CAsm6502::Stat CAsm6502::factor(CLeksem &leks, Expr &expr, bool nospace)
// [~|!|-|>|<] sta�a | '['wyra�enie']'
{
    OperType oper;
//  bool operation= false;

    if (leks.type==CLeksem::L_OPER)
    {
        oper = leks.GetOper();
        switch (oper)
        {
        case O_B_NOT:	// negacja bitowa '~'
        case O_NOT:	// negacja logiczna '!'
        case O_MINUS:	// minus unarny '-'
        case O_GT:	// g�rny bajt s�owa '>'
        case O_LT:	// dolny bajt s�owa '<'
        {
            //        operation = true;
            leks = next_leks();	// kolejny niepusty leksem
            Stat ret= factor(leks,expr,nospace);
            if (ret)
                return ret;
            if (expr.inf==Expr::EX_STRING)
                return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
            //        leks = next_leks(nospace);
            if (expr.inf!=Expr::EX_UNDEF)
                switch (oper)
                {
                case O_B_NOT:	// negacja bitowa '~'
                    expr.value = ~expr.value;
                    break;
                case O_NOT:		// negacja logiczna '!'
                    expr.value = !expr.value;
                    break;
                case O_MINUS:	// minus unarny '-'
                    expr.value = -expr.value;
                    break;
                case O_GT:		// g�rny bajt s�owa '>'
                    expr.value = (expr.value >> 8) & 0xFF;
                    break;
                case O_LT:		// dolny bajt s�owa '<'
                    expr.value = expr.value & 0xFF;
                    break;
                }
            return OK;
        }
        default:
            break;
        }
    }

    if (leks.type==CLeksem::L_EXPR_BRACKET_L)
    {
        leks = next_leks();		// kolejny niepusty leksem
        Stat ret= expression(leks,expr,true);
        if (ret)
            return ret;
        if (leks.type!=CLeksem::L_EXPR_BRACKET_R)
            return ERR_EXPR_BRACKET_R_EXPECTED;
        leks = next_leks(nospace);	// kolejny leksem
    }
    else
    {
        Stat ret= constant_value(leks,expr,nospace);
        if (ret)
            return ret;
    }
    return OK;
}


CAsm6502::Stat CAsm6502::mul_expr(CLeksem &leks, Expr &expr)
// czynnik [*|/|% czynnik]
{
    Stat ret= factor(leks,expr);		// obliczenie czynnika
    if (ret)
        return ret;
//  leks = next_leks();

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;
        OperType oper= leks.GetOper();
        if (oper!=O_MUL && oper!=O_DIV && oper!=O_MOD)
            return OK;
        if (expr.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        leks = next_leks();		// omini�cie operatora '*', '/' lub '%'

        Expr expr2(0);
        ret = factor(leks,expr2);		// kolejny czynnik
        if (ret)
            return ret;
        if (expr2.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
        //    leks = next_leks();

        if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
            switch (oper)
            {
            case O_MUL:
                expr.value *= expr2.value;
                break;
            case O_DIV:
                if (expr2.value == 0)
                    return ERR_DIV_BY_ZERO;
                expr.value /= expr2.value;
                break;
            case O_MOD:
                if (expr2.value == 0)
                    return ERR_DIV_BY_ZERO;
                expr.value %= expr2.value;
                break;
            }
        else
            expr.inf = Expr::EX_UNDEF;
    }
}


CAsm6502::Stat CAsm6502::shift_expr(CLeksem &leks, Expr &expr)
// czynnik1 [<<|>> czynnik1]
{
    Stat ret= mul_expr(leks,expr);	// obliczenie sk�adnika
    if (ret)
        return ret;		// b��d

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;

        bool left;
        switch (leks.GetOper())
        {
        case O_SHL:			// przesuni�cie w lewo?
            left = true;
            break;
        case O_SHR:			// przesuni�cie w prawo?
            left = false;
            break;
        default:
            return OK;
        }
        if (expr.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
        leks = next_leks();		// omini�cie operatora '>>' lub '<<'
        Expr expr2(0);
        ret= mul_expr(leks,expr2);		// obliczenie kolejnego sk�adnika
        if (ret)
            return ret;		// b��d
        if (expr2.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
        {
            if (left)
                expr.value <<= expr2.value;
            else
                expr.value >>= expr2.value;
        }
        else
            expr.inf = Expr::EX_UNDEF;
    }
}


CAsm6502::Stat CAsm6502::add_expr(CLeksem &leks, Expr &expr)
// sk�adnik [+|- sk�adnik]
{
    Stat ret= shift_expr(leks,expr);	// obliczenie sk�adnika
    if (ret)
        return ret;		// b��d

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;

        bool add;
        switch (leks.GetOper())
        {
        case O_MINUS:			// odejmowanie?
            add = false;
            if (expr.inf==Expr::EX_STRING)
                return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
            break;
        case O_PLUS:			// dodawanie?
            add = true;
            break;
        default:
            return OK;
        }
        leks = next_leks();		// omini�cie operatora '+' lub '-'
        Expr expr2(0);
        ret= shift_expr(leks,expr2);	// obliczenie kolejnego sk�adnika
        if (ret)
            return ret;		// b��d
        if (!add && expr2.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
        if ((expr.inf==Expr::EX_STRING) ^ (expr2.inf==Expr::EX_STRING))	// albo, albo
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
        {
            if (add)
            {
                if (expr.inf==Expr::EX_STRING && expr2.inf==Expr::EX_STRING)
                    expr.string += expr2.string;
                else
                {
                    ASSERT(expr.inf!=Expr::EX_STRING && expr2.inf!=Expr::EX_STRING);
                    expr.value += expr2.value;
                }
            }
            else
                expr.value -= expr2.value;
        }
        else
            expr.inf = Expr::EX_UNDEF;
    }
}

// wyr_proste [ & | '|' | ^ wyr_proste ]
CAsm6502::Stat CAsm6502::bit_expr(CLeksem &leks, Expr &expr)
{
    Stat ret= add_expr(leks,expr);	// obliczenie wyra�enia prostego
    if (ret)
        return ret;

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;
        OperType oper= leks.GetOper();
        if (oper!=O_B_AND && oper!=O_B_OR && oper!=O_B_XOR)
            return OK;
        if (expr.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        leks = next_leks();		// omini�cie operatora '&', '|' lub '^'

        Expr expr2(0);
        ret = add_expr(leks,expr2);		// kolejne wyra�enie proste
        if (ret)
            return ret;
        if (expr2.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
            switch (oper)
            {
            case O_B_AND:
                expr.value &= expr2.value;
                break;
            case O_B_OR:
                expr.value |= expr2.value;
                break;
            case O_B_XOR:
                expr.value ^= expr2.value;
                break;
            }
        else
            expr.inf = Expr::EX_UNDEF;
    }
}


CAsm6502::Stat CAsm6502::cmp_expr(CLeksem &leks, Expr &expr)	// wyr [>|<|>=|<=|==|!= wyr]
{
    Stat ret= bit_expr(leks,expr);	// obliczenie sk�adnika
    if (ret)
        return ret;				// b��d

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;
        OperType oper= leks.GetOper();
        if (oper!=O_GT && oper!=O_GTE && oper!=O_LT && oper!=O_LTE && oper!=O_EQ && oper!=O_NE)
            return OK;

        leks = next_leks();			// omini�cie operatora logicznego
        Expr expr2(0);
        ret= bit_expr(leks,expr2);		// obliczenie kolejnego sk�adnika
        if (ret)
            return ret;			// b��d
        if ((expr.inf==Expr::EX_STRING) ^ (expr2.inf==Expr::EX_STRING))	// albo, albo
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
            if (expr.inf==Expr::EX_STRING && expr2.inf==Expr::EX_STRING)
            {
                switch (oper)
                {
                case O_GT:
                    expr.value = expr.string > expr2.string;
                    break;
                case O_LT:
                    expr.value = expr.string < expr2.string;
                    break;
                case O_GTE:
                    expr.value = expr.string >= expr2.string;
                    break;
                case O_LTE:
                    expr.value = expr.string <= expr2.string;
                    break;
                case O_EQ:
                    expr.value = expr.string == expr2.string;
                    break;
                case O_NE:
                    expr.value = expr.string != expr2.string;
                    break;
                }
                expr.inf = Expr::EX_BYTE;
            }
            else
            {
                ASSERT(expr.inf!=Expr::EX_STRING && expr2.inf!=Expr::EX_STRING);
                switch (oper)
                {
                case O_GT:
                    expr.value = expr.value > expr2.value;
                    break;
                case O_LT:
                    expr.value = expr.value < expr2.value;
                    break;
                case O_GTE:
                    expr.value = expr.value >= expr2.value;
                    break;
                case O_LTE:
                    expr.value = expr.value <= expr2.value;
                    break;
                case O_EQ:
                    expr.value = expr.value == expr2.value;
                    break;
                case O_NE:
                    expr.value = expr.value != expr2.value;
                    break;
                }
            }
        else
            expr.inf = Expr::EX_UNDEF;
    }
}


CAsm6502::Stat CAsm6502::bool_expr_and(CLeksem &leks, Expr &expr)	// wyr [&& wyr]
{
    bool skip= false;

    Stat ret= cmp_expr(leks,expr);	// obliczenie sk�adnika
    if (ret)
        return ret;				// b��d

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;
        if (leks.GetOper() != O_AND)
            return OK;
        if (expr.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (expr.inf!=Expr::EX_UNDEF && expr.value==0)
            skip = true;			// ju� false - nie potrzeba dalej liczy�

        leks = next_leks();			// omini�cie operatora '&&'
        Expr expr2(0);
        ret= cmp_expr(leks,expr2);		// obliczenie kolejnego sk�adnika
        if (ret)
            return ret;			// b��d
        if (expr2.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (!skip)
            if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
                expr.value = expr2.value ? 1 : 0;
            else
                expr.inf = Expr::EX_UNDEF;
    }
}


CAsm6502::Stat CAsm6502::bool_expr_or(CLeksem &leks, Expr &expr)	// wyr [|| wyr]
{
    bool skip= false;

    Stat ret= bool_expr_and(leks,expr);	// obliczenie sk�adnika
    if (ret)
        return ret;				// b��d

    for (;;)
    {
        if (leks.type!=CLeksem::L_OPER)	// nie operator?
            return OK;
        if (leks.GetOper() != O_OR)
            return OK;

        if (expr.inf!=Expr::EX_UNDEF && expr.value!=0)
            skip = true;			// ju� true - nie potrzeba dalej liczy�

        leks = next_leks();			// omini�cie operatora '||'
        Expr expr2(0);
        ret= bool_expr_and(leks,expr2);	// obliczenie kolejnego sk�adnika
        if (ret)
            return ret;			// b��d
        if (expr2.inf==Expr::EX_STRING)
            return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

        if (!skip)
            if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczy� warto��?
                expr.value = expr2.value ? 1 : 0;
            else
                expr.inf = Expr::EX_UNDEF;
    }
}

// interpretacja wyra�enia
CAsm6502::Stat CAsm6502::expression(CLeksem &leks, Expr &expr, bool str)
{
    expr.inf = Expr::EX_LONG;
    Stat ret= bool_expr_or(leks,expr);

    if (ret)
        return ret;
    if (expr.inf == Expr::EX_STRING)
    {
        if (!str)			// wyra�enie znakowe dozwolone?
            return ERR_STR_NOT_ALLOWED;
    }
    else if (expr.inf != Expr::EX_UNDEF)
    {
        int32_t value = int32_t(expr.value);
        
        if (value > -0x100 && value < 0x100)
            expr.inf = Expr::EX_BYTE;
        else if (value>-0x10000 && value<0x10000)
            expr.inf = Expr::EX_WORD;
        else
            expr.inf = Expr::EX_LONG;  //undo change EX_LONG to EX_WORD
    }

    if (forcelong == 1)
    {
        expr.inf = Expr::EX_BYTE;
        //forcelong=0;
    }
    else if (forcelong == 2)
    {
        expr.inf = Expr::EX_WORD;
        //forcelong=0;
    }
    else if (forcelong == 3)
    {
        expr.inf = Expr::EX_LONG;
        //forcelong=0;
    }

    return OK;
}

//-----------------------------------------------------------------------------

CAsm6502::Stat CAsm6502::look_for_endm()	// szukanie .ENDM lub .MACRO
{
    CLeksem leks= next_leks(false);	// kolejny leksem, by� mo�e pusty (L_SPACE)
    bool labelled= false;

    switch (leks.type)
    {
    case CLeksem::L_IDENT:	// etykieta
    case CLeksem::L_IDENT_N:	// etykieta numerowana
        leks = next_leks();
        if (leks.type == CLeksem::L_IDENT_N)
        {
            Expr expr(0);
            Stat ret = factor(leks,expr);
            if (ret)
                return ret;
        }
        //      leks = next_leks();
        labelled = true;
        switch (leks.type)
        {
        case CLeksem::L_LABEL:	// znak ':'
            leks = next_leks();
            if (leks.type!=CLeksem::L_ASM_INSTR)
                return OK;
            break;
        case CLeksem::L_ASM_INSTR:
            break;
        default:
            return OK;
        }
        //      leks = next_leks();
        break;
    case CLeksem::L_SPACE:	// odst�p
        leks = next_leks();
        if (leks.type!=CLeksem::L_ASM_INSTR)	// nie dyrektywa asemblera?
            return OK;
        break;
    case CLeksem::L_COMMENT:	// komentarz
    case CLeksem::L_CR:		// koniec wiersza
        return OK;
    case CLeksem::L_FIN:	// koniec tekstu
        return STAT_FIN;
    default:
        return ERR_UNEXP_DAT;
    }

    ASSERT(leks.type==CLeksem::L_ASM_INSTR);	// dyrektywa asemblera

    switch (leks.GetInstr())
    {
    case I_MACRO:
        return ERR_NESTED_MACRO;	// definicja makra w makrodefinicji jest zabroniona
    case I_ENDM:
        return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_ENDM;	// koniec makra
    default:
        return OK;
    }
}


CAsm6502::Stat CAsm6502::record_macro()	// wczytanie kolejnego wiersza makrodefinicji
{
    CMacroDef *pMacro= get_last_macro_entry();

    Stat ret= look_for_endm();
    if (ret > 0)
        return ret;

    if (ret != STAT_ENDM)			// wiersza z .ENDM ju� nie potrzeba zapami�tywa�
        pMacro->AddLine(current_line,text->GetLineNo());

    return ret;
}

//-----------------------------------------------------------------------------

CString CAsm6502::format_local_label(const CString &ident, int area)
{
    CString local(' ',ident.GetLength()+8);
    local.Format("%08X%s",area,(LPCTSTR)ident);
    return local;
}

// spr. czy dana etykieta jest ju� zdefiniowana
bool CAsm6502::add_ident(const CString &id, CIdent &inf)
{
    CString tmp;
    const CString &ident= case_insensitive ? tmp : id;
    if (case_insensitive)
        tmp=id, tmp.MakeLower();

    if (ident[0]==LOCAL_LABEL_CHAR)	// etykieta lokalna?
    {
        if (expanding_macro)		// etykieta lokalna w makrorozszerzeniu?
            return macro_ident.insert(format_local_label(ident,macro_local_area),inf);
        else if (ident[1] == LOCAL_LABEL_CHAR) // file local?
            return proc_local_ident.insert(format_local_label(ident, proc_area), inf);
        else
            return local_ident.insert(format_local_label(ident,local_area),inf);
    }
    else					// etykieta globalna
        return global_ident.insert(ident,inf);
}


// wprowadzenie definicji etykiety (1. przebieg asemblacji)
CAsm6502::Stat CAsm6502::def_ident(const CString &id, CIdent &inf)
{
    ASSERT(pass==1);

    if (find_const(id) >= 0)
        return err_ident=id, ERR_CONST_LABEL_REDEF;

    CString tmp;
    const CString &ident= case_insensitive ? tmp : id;
    if (case_insensitive)
        tmp=id, tmp.MakeLower();

    if (ident[0]==LOCAL_LABEL_CHAR)	// etykieta lokalna?
    {
        if (expanding_macro)
        {
            if (!macro_ident.replace(format_local_label(ident,macro_local_area),inf))
                return err_ident=ident, ERR_LABEL_REDEF;	// ju� zdefiniowana
        }
        //% Bug Fix 1.2.13.1 - fix local labels causing duplicate label errors
        //else if (!proc_local_ident.replace(format_local_label(ident, proc_area), inf))
        else if ((ident[1]==LOCAL_LABEL_CHAR) & !proc_local_ident.replace(format_local_label(ident, proc_area), inf))
            return err_ident = ident, ERR_LABEL_REDEF;	// ju� zdefiniowana
        else if (!local_ident.replace(format_local_label(ident, local_area), inf))
            return err_ident = ident, ERR_LABEL_REDEF;	// ju� zdefiniowana
    }
    else					// etykieta globalna
    {
        if (!global_ident.replace(ident,inf))
            return err_ident=ident, ERR_LABEL_REDEF;	// ju� zdefiniowana
        //    if (inf.info == CIdent::I_ADDRESS)// etykieta z adresem odgradza etykiety lokalne
        local_area++;			// nowy obszar lokalny
    }
    return OK;
}


// sprawdzenie czy etykieta jest zdefiniowana (2. przebieg asemblacji)
CAsm6502::Stat CAsm6502::chk_ident(const CString &id, CIdent &inf)
{
    ASSERT(pass==2);
    CString tmp;
    const CString& ident= case_insensitive ? tmp : id;
    if (case_insensitive)
        tmp=id, tmp.MakeLower();

    CIdent info;
    bool exist= false;

    if (ident[0] == LOCAL_LABEL_CHAR)	// etykieta lokalna?
    {
        if (expanding_macro)		// etykieta lokalna w makrorozszerzeniu?
            exist = macro_ident.lookup(format_local_label(ident,macro_local_area),info);
        else if (ident[1] == LOCAL_LABEL_CHAR) // file local?
            exist = proc_local_ident.lookup(format_local_label(ident, proc_area), info);
        else
            exist = local_ident.lookup(format_local_label(ident,local_area),info);
    }
    else					// etykieta globalna
    {
        exist = global_ident.lookup(ident,info);
        local_area++;	// nowy obszar lokalny
    }

    if (exist)	// sprawdzana etykieta znaleziona w tablicy
    {
        if (info.info == CIdent::I_UNDEF)
            return err_ident=ident, ERR_UNDEF_LABEL;	// etykieta bez definicji
        ASSERT(info.variable && inf.variable || !info.variable && !inf.variable);

        if (info.val != inf.val && !info.variable)
            return err_ident=ident, ERR_PHASE;// niezgodne warto�ci mi�dzy przebiegami - b��d fazy
    }
    else		// sprawdzanej etykiety nie ma w tablicy
        return err_ident=ident, ERR_UNDEF_LABEL;

    inf = info;
    return OK;
}


// sprawdzenie definicji etykiety (2. przebieg asemblacji)
CAsm6502::Stat CAsm6502::chk_ident_def(const CString &id, CIdent &inf)
{
    CString tmp;
    const CString &ident= case_insensitive ? tmp : id;
    if (case_insensitive)
        tmp=id, tmp.MakeLower();

    SINT32 val= inf.val;		// zapami�tanie warto�ci
    Stat ret= chk_ident(ident, inf);
    if (ret != OK && ret != ERR_UNDEF_LABEL)
        return ret;
    if (inf.variable)		// etykieta zmiennej?
    {
        inf.val = val;		// nowa warto�� zmiennej
        bool ret;
        if (ident[0] == LOCAL_LABEL_CHAR)	// etykieta lokalna?
        {
            if (expanding_macro)		// etykieta lokalna w makrorozszerzeniu?
                ret = macro_ident.replace(format_local_label(ident, macro_local_area), inf);
            else if (ident[1] == LOCAL_LABEL_CHAR) // file local?
                ret = proc_local_ident.replace(format_local_label(ident, proc_area), inf);
            else
                ret = local_ident.replace(format_local_label(ident, local_area), inf);
        }
        else
            ret = global_ident.replace(ident, inf);
        ASSERT(ret);
    }
    else if (ident[0] != LOCAL_LABEL_CHAR)	// etykieta globalna sta�ej?
    {
        ASSERT(!inf.checked);
        inf.checked = true;		// potwierdzenie definicji w drugim przej�ciu asemblacji
        bool ret= global_ident.replace(ident, inf);
//		ASSERT(!ret && inf.info == I_ADDRESS || ret);		// etykieta musi by� redefiniowana
    }
    return OK;
}


CAsm6502::Stat CAsm6502::def_macro_name(const CString &id, CIdent &inf)
{
    ASSERT(pass==1);
    CString tmp;
    const CString &ident= case_insensitive ? tmp : id;
    if (case_insensitive)
        tmp=id, tmp.MakeLower();

    if (!macro_name.replace(ident,inf))
        return err_ident=ident, ERR_LABEL_REDEF;	// nazwa ju� zdefiniowana

    return OK;
}


CAsm6502::Stat CAsm6502::chk_macro_name(const CString &id)
{
    ASSERT(pass==2);
    CString tmp;
    const CString &ident= case_insensitive ? tmp : id;
    if (case_insensitive)
        tmp=id, tmp.MakeLower();

    CIdent info;

    if (macro_name.lookup(ident,info))	// sprawdzana etykieta znaleziona w tablicy
    {
        ASSERT(info.info==CIdent::I_MACRONAME);
        return OK;
        //    if (info.val != inf.val)
        //      return err_ident=ident, ERR_PHASE;// niezgodne warto�ci mi�dzy przebiegami - b��d fazy
    }
    else		// sprawdzanej etykiety nie ma w tablicy
        return err_ident=ident, ERR_UNDEF_LABEL;

    return OK;
}

//-----------------------------------------------------------------------------

const char* CRepeatDef::GetCurrLine(CString &str)	// odczyt aktualnego wiersza do powt�rki
{
    ASSERT(m_nLineNo >= 0);
    if (m_nLineNo == GetSize())	// koniec wierszy?
    {
        if (m_nRepeat == 0)		// koniec powt�rze�?
            return NULL;
        if (GetSize() == 0)		// puste powt�rzenie (bez wierszy)?
            return NULL;
        m_nRepeat--;		// odliczanie powt�rze�
        //    m_nRepeatLocalArea++;	// nowy obszar etykiet
        ASSERT(m_nRepeat >= 0);
        m_nLineNo = 0;
    }
    ASSERT(m_nLineNo < GetSize());
    str = GetLine(m_nLineNo++);
    return (const char *)str;
}


CAsm6502::Stat CAsm6502::record_rept(CRepeatDef *pRept)	// wczytanie kolejnego wiersza do powt�rki
{
    Stat ret= look_for_repeat();
    if (ret > 0)
        return ret;

    if (ret == STAT_REPEAT)	// zagnie�d�one .REPEAT
    {
        ret = OK;
        reptNested++;
    }
    else if (ret == STAT_ENDR)
        if (reptNested == 0)	// koniec .REPEAT?
            return ret;
        else
        {
            reptNested--;		// koniec zagnie�d�onego .REPEAT
            ret = OK;
        }

    pRept->AddLine(current_line,text->GetLineNo());

    return ret;
}


CAsm6502::Stat CAsm6502::look_for_repeat()	// szukanie .ENDR lub .REPEAT
{
    CLeksem leks= next_leks(false);	// kolejny leksem, by� mo�e pusty (L_SPACE)

    switch (leks.type)
    {
    case CLeksem::L_IDENT:		// etykieta
    case CLeksem::L_IDENT_N:	// etykieta numerowana
        leks = next_leks();
        if (leks.type == CLeksem::L_IDENT_N)
        {
            Expr expr(0);
            Stat ret = factor(leks,expr);
            if (ret)
                return ret;
        }
        //      leks = next_leks();
        switch (leks.type)
        {
        case CLeksem::L_LABEL:	// znak ':'
            leks = next_leks();
            if (leks.type!=CLeksem::L_ASM_INSTR)
                return OK;
            break;
        case CLeksem::L_ASM_INSTR:
            break;
        default:
            return OK;
        }
        //      leks = next_leks();
        break;
    case CLeksem::L_SPACE:	// odst�p
        leks = next_leks();
        if (leks.type!=CLeksem::L_ASM_INSTR)	// nie dyrektywa asemblera?
            return OK;
        break;
    case CLeksem::L_COMMENT:	// komentarz
    case CLeksem::L_CR:		// koniec wiersza
        return OK;
    case CLeksem::L_FIN:	// koniec tekstu
        return STAT_FIN;
    default:
        return ERR_UNEXP_DAT;
    }

    ASSERT(leks.type==CLeksem::L_ASM_INSTR);	// dyrektywa asemblera

    switch (leks.GetInstr())
    {
    case I_REPEAT:
        return STAT_REPEAT;		// zagnie�d�one .REPEAT
    case I_ENDR:
        return STAT_ENDR;		// koniec .REPEAT
    default:
        return OK;
    }
}

//-----------------------------------------------------------------------------
/*
const char *CAsm6502::get_next_line() // Load the next line into the assembly.
{
  LPTSTR pstr= current_line.GetBuffer(1024+4);
  char *ret= input.read_line(pstr,1024+4);
  current_line.ReleaseBuffer(-1);
  return ret;
}
*/

void CAsm6502::asm_start()
{
    user_error_text.Empty();

    if (markArea)
        markArea->Clear();

    if (debug)
    {
        debug->ResetFileMap();
        entire_text.SetFileUID(debug); // Generate FUID for the source text
    }
}

void CAsm6502::asm_fin()
{
    if (debug)
        generate_debug();

    if (markArea && markArea->IsStartSet())
        markArea->SetEnd(uint32_t(origin - 1) & mem_mask); // fix for last area not being marked.
}

void CAsm6502::asm_start_pass()
{
    source.Push(&entire_text);
    text = source.Peek();
    local_area = 0;
    proc_area = 0;
    macro_local_area = 0;
    //input.seek_to_begin();
    text->Start(0);
    origin = ~0u;
    originWrapped = false;
    if (pass == 2)
        debug->Empty();
}

void CAsm6502::asm_fin_pass()
{
    text->Fin(0);
}

//$$asm starts here
CAsm6502::Stat CAsm6502::assemble()	// translacja programu
{
    Stat ret;
    swapbin=false;
    bool skip= false;
    bool skip_macro= false;
//  CRepeatDef *pRept= NULL;

    try
    {
        asm_start();

        for (pass=1; pass<=2; pass++)	// dwa przej�cia asemblacji
        {
            asm_start_pass();
//      func = read.Peek();

            for (bool fin=false; !fin; )
            {
                while (!(ptr=text->GetCurrLine(current_line)))	// funkcja nie zwraca ju� wierszy?
                {
                    if (source.Peek())		// jest jeszcze jaka� funkcja odczytu wierszy?
                    {
                        text->Fin(&conditional_asm);
                        expanding_macro = (CMacroDef *)source.FindMacro();
                        repeating = (CRepeatDef *)source.FindRepeat();
                        text = source.Pop();
                    }
                    else
                        break;		// nie ma, zwracamy ptr==NULL na oznaczenie ko�ca tekstu programu
                }
                if (current_line.GetLength() > 1024)	// spr. max d�ugo�� wiersza
                    return ERR_LINE_TO_LONG;

                if (is_aborted())
                    return ERR_USER_ABORT;

                if (skip)			// asemblacja warunkowa (po .IF) ?
                    ret = look_for_endif();	// omijanie instrukcji a� do .ENDIF lub .ELSE
                else if (in_macro)		// zapami�tywanie makra (po .MACRO) ?
                    ret = record_macro();		// zapami�tanie wiersza makra
                else if (skip_macro)		// omijanie makra (w 2. przej�ciu po .MACRO) ?
                    ret = look_for_endm();	// omijanie wierszy a� do .ENDM
                else if (pRept)			// zapami�tywanie wiersza powt�rze�?
                    ret = record_rept(pRept);	// zapami�tanie wiersza do powt�rze�
                else
                {
                    ret = assemble_line();	// asemblacja wiersza
                    if (pass==2 && listing.IsOpen())
                    {
                        listing.AddSourceLine(current_line);
                        listing.NextLine();
                    }
                }

                switch (ret)
                {
                case STAT_INCLUDE:
                {
//	    if (text->IsMacro() || text->IsRepeat())
//	    if (typeid(text) == typeid(CMacroDef) || typeid(text) == typeid(CRepeatDef))
                    CSourceText* pSrc= dynamic_cast<CSourceText*>(text);
                    if (pSrc == NULL)
                        return ERR_INCLUDE_NOT_ALLOWED;	// .INCLUDE w makrze/powt�rce niedozwolone
//						proc_area++;
                    pSrc->Include(include_fname,debug);
                    break;
                }

                case STAT_IF_TRUE:
                case STAT_IF_FALSE:
                case STAT_IF_UNDETERMINED:
                    ret = conditional_asm.instr_if_found(ret);
                    if (ret > OK)
                        return ret;		// b��d
                    skip = ret==STAT_SKIP;	// omijanie instrukcji a� do .ELSE lub .ENDIF?
                    break;
                case STAT_ELSE:
                    ret = conditional_asm.instr_else_found();
                    if (ret > OK)
                        return ret;		// b��d
                    skip = ret==STAT_SKIP;	// omijanie instrukcji a� do .ELSE lub .ENDIF?
                    break;
                case STAT_ENDIF:
                    ret = conditional_asm.instr_endif_found();
                    if (ret > OK)
                        return ret;		// b��d
                    skip = ret==STAT_SKIP;	// omijanie instrukcji a� do .ELSE lub .ENDIF?
                    break;

                case STAT_MACRO:		// makrodefinicja
                    if (pass == 2)		// drugie przej�cie?
                        skip_macro = true;	// omijanie makrodefinicji (ju� zarejestrowanej)
                    //if (expanding_macro)
                    //	expanding_macro->StoreConditionLevel(conditional_asm.get_level());
                    break;
                case STAT_ENDM:		// koniec makrodefinicji
                    if (pass == 1)		// pierwsze przej�cie?
                    {
                        ASSERT(in_macro);
                        in_macro = NULL;		// rejestracja makra zako�czona
                    }
                    else
                    {
                        ASSERT(skip_macro);
                        skip_macro = false;	// omijanie definicji makra zako�czone
                    }
                    break;
                case STAT_EXITM:
                    ASSERT(expanding_macro);
                    while (expanding_macro != text)	// szukanie opuszczanego makra
                    {
                        text->Fin(&conditional_asm);
                        text = source.Pop();
                    }
                    text->Fin(&conditional_asm);		// zako�czenie rozwijania makra
                    expanding_macro = (CMacroDef *)source.FindMacro();
                    repeating = (CRepeatDef *)source.FindRepeat();
                    text = source.Pop();	// poprzednie �r�d�o wierszy
                    break;

                case STAT_REPEAT:		// zarejestrowanie wierszy po .REPEAT
                    pRept = new CRepeatDef(reptInit);
                    if (pass == 2)
                        pRept->SetFileUID(text->GetFileUID());
                    break;
                case STAT_ENDR:		// koniec rejestracji, teraz powtarzanie
                    //	    RepeatStart(pRept);
                    source.Push(text);		// bie��ce �r�d�o wierszy na stos
                    text = pRept;
                    pRept = NULL;
                    text->Start(&conditional_asm);
                    break;

                case STAT_FIN:		// koniec pliku
                    ASSERT(dynamic_cast<CSourceText*>(text));
//	    ASSERT( typeid(text) != typeid(CMacroDef) && typeid(text) != typeid(CRepeatDef));
//	    ASSERT(!text->IsMacro() && !text->IsRepeat());
                    if (!static_cast<CSourceText*>(text)->TextFin()) // koniec zagnie�d�onego odczytu (.include) ?
                    {
                        if (conditional_asm.in_cond())	// w �rodku dyrektywy .IF ?
                            return ERR_ENDIF_REQUIRED;
                        if (in_macro)
                            return ERR_ENDM_REQUIRED;
                        //proc_area--;
                        fin = true;		// koniec przej�cia asemblacji
                        ret = OK;
                    }
                    break;

                case OK:
                    break;

                default:
                    if (listing.IsOpen())	// usuni�cie listingu ze wzgl�du na b��dy
                        listing.Remove();
                    return ret;			// b��d asemblacji
                }
            }

            asm_fin_pass();
        }

        asm_fin();

    }
    /*
    catch (CMemoryException*)
    {
        return ERR_OUT_OF_MEM;
    }
    */
    catch (CFileException*)
    {
        return ERR_FILE_READ;
    }

    return ret;
}

//-----------------------------------------------------------------------------
static const uint8_t NA = 0x42;   // WDM on 65816

CAsm6502::Stat CAsm6502::chk_instr_code(OpCode &code, CodeAdr &mode, Expr expr, int &length)
{
    uint8_t byte;
    const uint8_t (&trans)[C_ILL][A_NO_OF_MODES] = TransformTable(bProc6502);

    if (mode >= A_NO_OF_MODES) // Undetermined addressing modes
    {
        switch (mode)
        {
        case A_ABS_OR_ZPG:
            byte = trans[code][mode = A_ABS]; // We choose ABS
            if (byte == NA) // if there is no ABS_X,
                byte = trans[code][mode=A_ZPG];
            break;

        case A_ABSX_OR_ZPGX:
            byte = trans[code][mode = A_ABS_X];
            if (byte == NA) // if there is no ABS_X,
                byte = trans[code][mode = A_ZPG_X]; // to spr. ZPG_X
            break;

        case A_ABSY_OR_ZPGY:
            byte = trans[code][mode = A_ABS_Y];
            if (byte == NA) // if there is no ABS_Y,
                byte = trans[code][mode = A_ZPG_Y]; // to spr. ZPG_Y
            break;

        case A_ABSI_OR_ZPGI:
            byte = trans[code][mode = A_ABSI];
            if (byte == NA) // if there is no ABSI,
                byte = trans[code][mode = A_ZPGI]; // to spr. ZPGI
            break;

        case A_IMP_OR_ACC:
            byte = trans[code][mode = A_IMP];
            if (byte == NA) // if there is no IMP,
                byte = trans[code][mode = A_ACC]; // to spr. ACC
            if (code == C_BRK)
                mode = A_IMP2;
            break;

        case A_ABSIX_OR_ZPGIX:
            byte = trans[code][mode = A_ZPGI_X];
            if (byte == NA) // if there is no ZPGI_X,
                byte = trans[code][mode = A_ABSI_X]; // to spr. ABSI_X
            break;

        default:
            ASSERT(false);
            break;
        }
    }
    else
        byte = trans[code][mode];

    if (bProc6502 == 2 && code == C_WDM && mode == A_IMM) // allow WDM in 65816 mode
    {
        length = 1 + 1;
        return OK;
    }
    else if (byte == NA) // Addressing mode not allowed?
    {
        switch (mode) // Promotion of the ZPG addressing mode to the corresponding ABS
        {
        case A_IMM:
            mode = A_IMM2;
            break;

        case A_ZPG: // zero page
            if (bProc6502 == 2)
                mode = A_ABSL;

            if (trans[code][mode]==NA)
                mode = A_ABS;

            if (trans[code][mode]==NA)
                mode = A_REL; // move to REL

            if (trans[code][mode]==NA)
                mode = A_RELL; // move to RELL
            break;

        case A_ZPG_X: // zero page indexed X
            if (bProc6502 == 2)
                mode = A_ABSL_X;

            if (trans[code][mode]==NA)
                mode = A_ABS_X;
            break;

        case A_ZPG_Y: // zero page indexed Y
            mode = A_ABS_Y;
            break;

        case A_ZPGI: // zero page indirect
            mode = A_ABSI;
            break;

        case A_ABS: // There is ABS but maybe it's about REL
            byte = trans[code][mode = A_ABSL]; // We choose ABS

            if (byte == NA) // If there is no ABS_X,
                mode = A_REL;

            if (trans[code][mode] == NA)
                mode = A_RELL; // move to REL
            break;

        case A_ZPGI_X:	// (zp,X) illigal, try (abs,X) then
            mode = A_ABSI_X;
            break;

        case A_ABSI: // Absolute Indirect
            if (code == C_JML)
            {
                code = C_JMP;
                mode = A_INDL;
                byte = trans[code][mode];
                break;
            }
            else
                return ERR_MODE_NOT_ALLOWED;

        case A_ABSL: // 65816 in case its a label value > 0xFFFF
            if (code == C_JMP)
            {
                code = C_JML;
                byte = trans[code][mode];
                break;
            }
            else if (code == C_JSR)
            {
                code = C_JSL;
                byte = trans[code][mode];
                break;
            }
            else
            {
                byte = trans[code][mode = A_ABS]; // We choose ABS
                
                if (byte == NA) // if there is no ABS_X,
                    byte = trans[code][mode = A_REL];

                if (byte == NA) // if there is no ABS_X,
                    mode = A_RELL;

                break;
            }

        default:
            return ERR_MODE_NOT_ALLOWED;
        }

        byte = trans[code][mode];
        if (byte == NA) // Still illegal addressing mode?
            return ERR_MODE_NOT_ALLOWED;
    }

    switch (mode) // Specify the length of the instruction
    {
    case A_IMP: // implied
    case A_ACC: // accumulator
        length = 1 + 0;
        break;

    case A_IMP2: // implied dla BRK
        length = generateBRKExtraByte ? 1 + 1 : 1 + 0;
        break;

    case A_ZPG:    // zero page
    case A_ZPG_X:  // zero page indexed X
    case A_ZPG_Y:  // zero page indexed Y
    case A_ZPGI:   // zero page indirect
    case A_ZPGI_X: // zero page indirect, indexed X
    case A_ZPGI_Y: // zero page indirect, indexed Y
    case A_IMM:    // immediate
    case A_ZPG2:   // zero page (dla SMB i RMB)
        length = 1 + 1;
        break;

    case A_REL: // relative
        if (pass == 2)
        {
            if (origin > mem_mask)
                return ERR_UNDEF_ORIGIN;

            int32_t dist = expr.value - (int32_t(origin) + 2);

            if (((expr.value >> 16) & 0xFF) != (((origin) >> 16) & 0xFF))
                return ERR_INVALID_BANK_CROSSING;

            if (dist > 127 || dist < -128)
                return ERR_REL_OUT_OF_RNG;
        }
        length = 1 + 1;
        break;

    case A_ABS: // absolute
//		if (pass == 2 &&(((expr.value >> 16) & 0xFF) != ((origin >> 16) & 0xFF))) {
//		}
//		if ((pass == 2) && (byte == 0x20 || byte == 0x4C || byte == 0x6C || byte == 0xFC) && (((expr.value >> 16) & 0xFF) != ((origin >> 16) & 0xFF)))
//			return ERR_INVALID_BANK_CROSSING;

        length = 1 + 2;
        break;

    case A_ABS_X:  // absolute indexed X
    case A_ABS_Y:  // absolute indexed Y
    case A_ABSI:   // absolute indirect
    case A_ABSI_X: // absolute indirect, indexed X
        length = 1 + 2;
        break;

    case A_ZREL:	// zero page / relative (BBS i BBR z 65c02)
        if (pass == 2)
        {
            if (origin > 0xFFFF)
                return ERR_UNDEF_ORIGIN;

            ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);
            int32_t dist = expr.value - (int32_t(origin & 0xFFFF) + 3);

            if (dist > 127 || dist < -128)
                return ERR_REL_OUT_OF_RNG;

            if (((expr.value >> 16) & 0xFF) != (((origin) >> 16) & 0xFF))
                return ERR_INVALID_BANK_CROSSING;
        }
        length = 1 + 1 + 1;
        break;

    case A_ABSL:
    case A_ABSL_X:
        length = 1 + 3;
        break;

    case A_ZPIL:
    case A_ZPIL_Y:
        length = 1 + 1;
        break;

    case A_INDL:
        length = 1 + 2;
        break;

    case A_SR:
    case A_SRI_Y:
        length = 1 + 1;
        break;

    case A_RELL: //$$
        if ((pass == 2) &&((expr.value >> 16) & 0xFF) != (((origin) >> 16) & 0xFF))
            return ERR_INVALID_BANK_CROSSING;
        length = 1 + 2;
        break;

    case A_XYC:
        length = 1 + 2;
        break;

    case A_IMM2:
        length = 1 + 2;
        break;

    default:
        ASSERT(false);
        break;
    }

    if ((pass == 2) &&(((origin + length - 1) >> 16) & 0xFF) != (((origin) >> 16) & 0xFF))
        return ERR_INVALID_BANK_CROSSING;

    return OK;
}

// Code generation
void CAsm6502::generate_code(OpCode code, CodeAdr mode, Expr expr, Expr expr_bit, Expr expr_zpg)
{
    ASSERT(TransformTable(bProc6502)[code][mode] != NA || mode == A_IMP2 && code = =C_BRK);
    ASSERT(origin <= 0xFFFF);

    if (mode == A_IMP2 && code == C_BRK)
        (*out)[origin] = 0;
    else
        (*out)[origin] = TransformTable(bProc6502)[code][mode]; // command

    switch (mode) // Command argument
    {
    case A_IMP: // implied
    case A_ACC: // accumulator
        if (listing.IsOpen())
            listing.AddCodeBytes(origin,(*out)[origin]);
        return;

    case A_IMP2: // implied dla BRK
        ASSERT(origin + 1 <= mem_mask);

        if (generateBRKExtraByte)
            (*out)[origin + 1] = BRKExtraByte;

        if (listing.IsOpen())
            listing.AddCodeBytes(origin,(*out)[origin],generateBRKExtraByte ? (*out)[origin + 1] : -1);
        return;

    case A_ZPG:    // zero page
    case A_ZPG_X:  // zero page indexed X
    case A_ZPG_Y:  // zero page indexed Y
    case A_ZPGI:   // zero page indirect
    case A_ZPGI_X: // zero page indirect, indexed X
    case A_ZPGI_Y: // zero page indirect, indexed Y
    case A_IMM:    // immediate
        ASSERT(origin + 1 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_BYTE);
        (*out)[origin + 1] = uint8_t(expr.value & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1]);
        break;

    case A_REL: // relative
    {
        //ASSERT(origin + 1 <= mem_mask);
        //ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);
        int32_t dist= expr.value - ( int32_t(origin & mem_mask) + 2 ); // 65816
        //ASSERT(dist < 128 && dist > -129);
        (*out)[origin + 1] = uint8_t(dist & 0xFF);
        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1]);
        break;
    }

    case A_ABS:    // absolute
    case A_ABS_X:  // absolute indexed X
    case A_ABS_Y:  // absolute indexed Y
    case A_ABSI:   // absolute indirect
    case A_ABSI_X: // absolute indirect, indexed X
    case A_INDL:   // Indirect Long
        ASSERT(origin + 2 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);
        (*out)[origin + 1] = uint8_t(expr.value & 0xFF);
        (*out)[origin + 2] = uint8_t((expr.value >> 8) & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1], (*out)[origin + 2]);
        break;

    case A_ZPG2: // zeropage for SMB and RMB
    {
        (*out)[origin] = TransformTable(bProc6502)[code][mode];  // SMB or RMB command
        ASSERT(expr_bit.inf == Expr::EX_BYTE && abs(expr_bit.value) < 8);
        (*out)[origin] += uint8_t(expr_bit.value << 4); // Response bit number for the SMBn or RMBn instruction
        ASSERT(origin + 1 <= mem_mask);

        ASSERT(expr_zpg.inf == Expr::EX_BYTE); // Argument address (on zeropage)
        (*out)[origin + 1] = uint8_t(expr_zpg.value & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1]);
        break;
    }

    case A_ZREL: // zeropage / relative
    {
        (*out)[origin] = TransformTable(bProc6502)[code][mode]; // BBS or BBR command
        ASSERT(expr_bit.inf == Expr::EX_BYTE && abs(expr_bit.value) < 8);
        (*out)[origin] += uint8_t(expr_bit.value << 4); // Response bit number for the BBSn or BBRn instruction
        ASSERT(origin + 2 <= mem_mask);

        ASSERT(expr_zpg.inf == Expr::EX_BYTE); // Argument address (on zeropage)
        (*out)[origin + 1] = uint8_t(expr_zpg.value & 0xFF);

        ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);
        int32_t dist = expr.value - ( int32_t(origin & 0xFFFF) + 3 );

        ASSERT(dist < 128 && dist > -129);
        (*out)[origin + 2] = uint8_t(dist & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1], (*out)[origin + 2]);
        break;
    }
    case A_ABSL:   // absolute Long
    case A_ABSL_X: // absolute Long indexed X
        ASSERT(origin+2 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);

        (*out)[origin + 1] = uint8_t(expr.value & 0xFF);
        (*out)[origin + 2] = uint8_t((expr.value >> 8) & 0xFF);
        (*out)[origin + 3] = uint8_t((expr.value >> 16) & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1], (*out)[origin + 2], (*out)[origin + 3]);
        break;

    case A_ZPIL:
    case A_ZPIL_Y:
    case A_SR:
    case A_SRI_Y:
        ASSERT(origin + 1 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_BYTE);

        (*out)[origin + 1] = uint8_t(expr.value & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin+1]);
        break;

    case A_RELL:
    {
        ASSERT(origin + 1 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);
        int32_t dist = expr.value - ( int32_t(origin & mem_mask) + 3 );
        ASSERT(dist < 128 && dist > -129);

        (*out)[origin + 1] = uint8_t(dist & 0xFF);
        (*out)[origin + 2] = uint8_t((dist >>8) & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1], (*out)[origin + 2]);
        break;
    }

    case A_XYC:
        ASSERT(origin + 1 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_BYTE);

        (*out)[origin + 1] = uint8_t(expr.value & 0xFF);
        (*out)[origin + 2] = uint8_t((expr.value >> 8) & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1], (*out)[origin + 2]);
        break;

    case A_IMM2:
        ASSERT(origin+1 <= mem_mask);
        ASSERT(expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE);

        (*out)[origin + 1] = uint8_t(expr.value & 0xFF);
        (*out)[origin + 2] = uint8_t((expr.value>>8) & 0xFF);

        if (listing.IsOpen())
            listing.AddCodeBytes(origin, (*out)[origin], (*out)[origin + 1], (*out)[origin + 2]);
        break;

    default:
        ASSERT(false);
        break;
    }
}

CAsm6502::Stat CAsm6502::inc_prog_counter(int dist)
{
    if (origin > mem_mask)
        return ERR_UNDEF_ORIGIN;

    if (originWrapped)
        return ERR_PC_WRAPED;

    origin += dist;

    if (origin == mem_mask +1)
    {
        originWrapped = true;
        origin = 0;
    }
    else if (origin > mem_mask)
        return ERR_PC_WRAPED;

    return OK;
}

//-----------------------------------------------------------------------------
/*
int CAsm6502::get_line_no()		// numer wiersza (dla debug info)
{
  if (repeating)
    return repeating->GetLineNo();
  return expanding_macro ? expanding_macro->GetLineNo() : input.get_line_no();
}

CAsm::FileUID CAsm6502::get_file_UID()	// id pliku (dla debug info)
{
  if (repeating)
    return repeating->GetFileUID();
  return expanding_macro ? expanding_macro->GetFileUID() : input.get_file_UID();
}
*/

void CAsm6502::generate_debug(uint32_t addr, int line_no, FileUID file_UID)
{
    ASSERT(debug);

    CDebugLine line(line_no, file_UID,addr, typeid(text) == typeid(CMacroDef) ? DBG_CODE | DBG_MACRO : DBG_CODE);
    debug->AddLine(line);
}

CAsm::Stat CAsm6502::generate_debug(InstrType it, int line_no, FileUID file_UID)
{
    ASSERT(debug);

    switch (it)
    {
    case I_DD:      // def double byte
    case I_DW:      // def word
    case I_DB:      // def byte
    case I_DS:      // def string
    case I_DCB:     // declare block
    case I_RS:      // reserve space
    case I_ASCIS:   // ascii + $80 ostatni bajt
    {
        if (origin > mem_mask)
//			if (origin > 0xFFFF)		// 65816 - rollover error
            return ERR_UNDEF_ORIGIN;

        CDebugLine dl(line_no,file_UID, (uint32_t)origin, DBG_DATA);
        debug->AddLine(dl);
        break;
    }

    case I_ORG:
    case I_START:
    case I_END:
    case I_ERROR:
    case I_INCLUDE:
    case I_IF:
    case I_ELSE:
    case I_ENDIF:
    case I_OPT:
    case I_ROM_AREA:
    case I_IO_WND:
        break;

    default:
        ASSERT(false);
        break;
    }

    return OK;
}

/*************************************************************************/

void CAsm6502::generate_debug()
{
    int index = 0;

    debug->SetIdentArrSize(global_ident.size() + local_ident.size() + proc_local_ident.size());

    for (auto item : global_ident)
        debug->SetIdent(index++, item.first, item.second);

    for (auto item : proc_local_ident)
        debug->SetIdent(index++, item.first, item.second);

    for (auto item : local_ident)
        debug->SetIdent(index++, item.first, item.second);
}

/*************************************************************************/

std::string CAsm6502::GetErrMsg(Stat stat)
{
#if 0
    if ((stat < OK || stat >= ERR_LAST) && stat != STAT_USER_DEF_ERR)
    {
        ASSERT(false); // Invalid value 'stat'
        return std::string("???");
    }

    wxString msg, form, txt;

    //if (!text->IsPresent()) // Line assembly?
    if (check_line)
    {
        ASSERT(stat > 0);

        if (form.LoadString(IDS_ASM_FORM3) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST + stat))
            msg.Printf(form, (int)stat, txt);

        return msg.ToStdString();
    }

    switch (stat)
    {
    case OK:
        msg.LoadString(IDS_ASM_ERR_MSG_FIRST);
        break;

    case ERR_OUT_OF_MEM:
        if (form.LoadString(IDS_ASM_FORM3) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST + stat))
            msg.Printf(form, (int)stat, txt);
        break;

    case ERR_FILE_READ:
        if (form.LoadString(IDS_ASM_FORM2) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST + stat))
            msg.Printf(form, (int)stat, txt, (LPCTSTR)text->GetFileName());
        break;

    case ERR_UNDEF_LABEL: // Undefined label
    case ERR_PHASE:
    case ERR_LABEL_REDEF: // Label already defined
        if (form.LoadString(IDS_ASM_FORM4) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST + stat))
            msg.Printf(form, (int)stat, txt, (LPCTSTR)err_ident, text->GetLineNo() + 1, (LPCTSTR)text->GetFileName());
        break;

    case STAT_USER_DEF_ERR:
        if (!user_error_text.IsEmpty())
        {
            if (form.LoadString(IDS_ASM_FORM5))
                msg.Printf(form, (LPCTSTR)user_error_text, text->GetLineNo() + 1, (LPCTSTR)text->GetFileName());
        }
        else
        {
            if (form.LoadString(IDS_ASM_FORM6))
                msg.Printf(form, text->GetLineNo() + 1, (LPCTSTR)text->GetFileName());
        }
        break;

    default:
        if (form.LoadString(IDS_ASM_FORM1) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST+stat))
        {
            try
            {
                msg.Printf(form, (int)stat, txt, text->GetLineNo() + 1, (LPCTSTR)text->GetFileName());
            }
            catch (CInvalidArgException *)
            {
                form.LoadString(IDS_ASM_FORM3);
                msg.Printf(form, (int)stat, txt);
            }
        }
        break;
    }

    return msg;
#endif

    return "";
}

/*************************************************************************/
