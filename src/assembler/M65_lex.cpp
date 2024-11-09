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

CToken::CToken(const CToken &leks)
    : type(leks.type)
{
    memcpy(block, leks.block, sizeof(block)); // copy whole union and type field

    if (leks.type == CTokenType::L_STR || leks.type == CTokenType::L_IDENT || leks.type == CTokenType::L_IDENT_N) // contains string?
        str = leks.str; // duplicate string
}

CToken & CToken::operator = (const CToken &leks)
{
//  ASSERT(type == leks.type);
    if (type == CTokenType::L_STR || type == CTokenType::L_IDENT || type == CTokenType::L_IDENT_N) // the target lexeme contains the string?
    {
        str = "";
    }

    type = leks.type;
    memcpy(block, leks.block, sizeof(block)); // copy the entire union and type field

    if (leks.type == CTokenType::L_STR || leks.type == CTokenType::L_IDENT || leks.type == CTokenType::L_IDENT_N) // the source lexeme contains a string of characters?
        str = leks.str; // duplicate the string

    return *this;
}

CToken::~CToken()
{
}

/*************************************************************************/
