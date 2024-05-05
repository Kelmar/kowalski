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

// Loading the arguments of the macro call
CAsm::Stat CMacroDef::ParseArguments(CLeksem &leks, CAsm6502 &asmb)
{
    std::string literal;
    bool get_param = true;
    bool first_param = true;
    CAsm::Stat ret;
    int count = 0;

    int required = m_nParams >= 0 ? m_nParams : -m_nParams - 1; // Number of required args.

    m_strarrArgs.clear();
    m_narrArgs.clear();
    m_arrArgType.clear();
    m_nParamCount = 0;
    //leks = asmb.next_leks();

    if (m_nParams == 0) // Parameterless macro?
        return CAsm::OK;

    for (;;)
    {
        if (get_param) // Possibly another argument
        {
            switch (leks.type)
            {
            case CLeksem::L_STR:
                literal = *leks.GetString();

                m_strarrArgs.push_back(literal);
                m_narrArgs.push_back(literal.size());
                m_arrArgType.push_back(STR);

                count++;
                
                get_param = false; // Parameter already interpreted
                first_param = false; // First parameter already loaded
                leks = asmb.next_leks();
                break;

            case CLeksem::L_ERROR:
                // Added to prevent looping C runtime errors if first param is ''
                // use this or ERR_PARAM_REQUIRED;
                return CAsm::ERR_EMPTY_PARAM;

            default:
                if (asmb.is_expression(leks)) // expression?
                {
                    Expr expr;
                    ret = asmb.expression(leks, expr);

                    if (ret)
                        return ret;

                    if (expr.inf == Expr::EX_UNDEF) // Uknown value
                    {
                        m_strarrArgs.push_back("");
                        m_narrArgs.push_back(0);
                        m_arrArgType.push_back(UNDEF_EXPR);
                    }
                    else if (expr.inf == Expr::EX_STRING)
                    {
                        m_strarrArgs.push_back(expr.string);
                        m_narrArgs.push_back(expr.string.size());
                        m_arrArgType.push_back(STR);
                    }
                    else
                    {
                        wxString num;
                        num.Printf("%ld", expr.value);

                        m_strarrArgs.push_back(num.ToStdString());
                        m_narrArgs.push_back(expr.value);
                        m_arrArgType.push_back(NUM);
                    }
                    count++;
                    get_param = false; // parameter already interpreted
                }
                else
                {
                    if (count < required)
                        return CAsm::ERR_PARAM_REQUIRED; // Too few macro calling parameters

                    if (!first_param)
                        return CAsm::ERR_PARAM_REQUIRED; // After the decimal point, you must enter another parameter

                    m_nParamCount = count;
                    return CAsm::OK;
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
                    return CAsm::ERR_PARAM_REQUIRED;	// Too few macro calling parameters

                if (count > required && m_nParams > 0) // if macro called with ..., then m_nParams will be negative
                    return CAsm::ERR_MACRO_PARAM_COUNT;

                m_nParamCount = count;
                return CAsm::OK;
            }
        }
    }
}

// type of macro parameter (to distinguish between numbers and strings)
CAsm::Stat CMacroDef::ParamType(const std::string &param_name, _Out_ bool &found, _Out_ int &type)
{
    CIdent ident;

    if (!TryLookup(param_names, param_name, ident)) // Find a parameter with a given name
    {
        found = false;
        return CAsm::OK;
    }

    return ParamType(ident.val, found, type);
}

CAsm::Stat CMacroDef::ParamType(int param_number, _Out_ bool &found, _Out_ int &type)
{
    if (param_number >= m_nParamCount || param_number < 0)
    {
        found = false;
        return CAsm::ERR_EMPTY_PARAM;
    }

    found = true;

    ASSERT(m_arrArgType.size() > param_number);
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

    return CAsm::OK;
}

// Finding the 'param_name' parameter of the current macro
CAsm::Stat CMacroDef::ParamLookup(CLeksem &leks, const std::string& param_name, Expr &expr, bool &found, CAsm6502 &asmb)
{
    CIdent ident;

    if (!TryLookup(param_names, param_name, _Out_ ident)) // Find a parameter with a given name
    {
        found = false;
        return CAsm::OK;
    }

    found = true;
    leks = asmb.next_leks(false);
    return ParamLookup(leks, ident.val, expr, asmb);
}

// Finding the value of parameter number 'param number' of the current macro
CAsm::Stat CMacroDef::ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb)
{
    bool special = param_number == -1; // Variable %0 ? (not a parameter)
    
    if (leks.type == CLeksem::L_STR_ARG) // Reference to the parameter's character value?
    {
        if (!special && (param_number >= m_nParamCount || param_number < 0))
            return CAsm::ERR_EMPTY_PARAM;

        if (special) // Reference to %0$ -> which means the macro name
            expr.string = m_strName;
        else
        {
            ASSERT(m_arrArgType.size() > param_number);
            
            if (m_arrArgType[param_number] != STR) // Check whether the variable has a text value
                return CAsm::ERR_NOT_STR_PARAM;

            ASSERT(m_strarrArgs.size() > param_number);
            expr.string = m_strarrArgs[param_number];
        }

        expr.inf = Expr::EX_STRING;
        leks = asmb.next_leks();
        return CAsm::OK;
    }
    else if (leks.type == CLeksem::L_SPACE)
        leks = asmb.next_leks();

    if (special) // Reference to %0 -> number of current parameters in the macro call
    {
        expr.inf = Expr::EX_LONG;
        expr.value = m_nParamCount;
    }
    else // reference to the current parameter!!
    {
        if (param_number >= m_nParamCount || param_number < 0)
            return CAsm::ERR_EMPTY_PARAM; // There is no parameter with this number
            
        ASSERT(m_arrArgType.size() > param_number);

        switch (m_arrArgType[param_number]) // current parameter type
        {
        case NUM: // Numeric parameter
        case STR: // Text parameter (its length is given)
            ASSERT(m_narrArgs.size() > param_number);
            expr.inf = Expr::EX_LONG;
            expr.value = m_narrArgs[param_number];
            break;

        case UNDEF_EXPR: // Numeric parameter, undefined value
            ASSERT(m_narrArgs.size() > param_number);
            expr.inf = Expr::EX_UNDEF;
            expr.value = 0;
            break;

        default:
            ASSERT(false);
            break;
        }
    }

    return CAsm::OK;
}

// Check macro parameter reference syntax (line checking mode)
CAsm::Stat CMacroDef::AnyParamLookup(CLeksem &leks, CAsm6502 &asmb)
{
    if (leks.type == CLeksem::L_STR_ARG) // Reference to the parameter's character value?
    {
        leks = asmb.next_leks();
        return CAsm::OK;
    }
    else if (leks.type == CLeksem::L_SPACE)
        leks = asmb.next_leks();

    return CAsm::OK;
}

bool CMacroDef::GetCurrLine(std::string &str) // Reading the current macro line
{
    ASSERT(m_nLineNo >= 0);
    if (m_nLineNo < GetSize())
    {
        str = GetLine(m_nLineNo++);
        return true;
    }
    else // End of lines?
    {
        ASSERT(m_nLineNo == GetSize());
        return false;
    }
}

/*************************************************************************/
