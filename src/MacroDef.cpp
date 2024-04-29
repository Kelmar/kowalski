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
                if (asmb.is_expression(leks)) // expression?
                {
                    Expr expr;
                    ret = asmb.expression(leks, expr);

                    if (ret)
                        return ret;

                    if (expr.inf == Expr::EX_UNDEF) // Uknown value
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

    if (!param_names.lookup(param_name, ident)) // Find a parameter with a given name
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

bool CMacroDef::GetCurrLine(std::string &str) // Reading the current macro line
{
    ASSERT(m_nLineNo >= 0);
    if (m_nLineNo < GetSize())
    {
        str = GetLine(m_nLineNo++);
        return true;
    }
    else				// koniec wierszy?
    {
        ASSERT(m_nLineNo == GetSize());
        return false;
    }
}

/*************************************************************************/
