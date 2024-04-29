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

CLeksem::CLeksem(const CLeksem &leks)
    : type(leks.type)
{
    memcpy(this, &leks, sizeof(CLeksem)); // copy whole union and type field
    if (leks.type == L_STR || leks.type == L_IDENT || leks.type == L_IDENT_N) // contains string?
        str = new std::string(*leks.str); // duplicate string
}


CLeksem & CLeksem::operator = (const CLeksem &leks)
{
//  ASSERT(type == leks.type);
    if (type == L_STR || type == L_IDENT || type == L_IDENT_N) // the target lexeme contains the string?
    {
        delete str;
        str = NULL;
    }
//  type = leks.type;
    memcpy(this, &leks, sizeof(CLeksem)); // copy the entire union and type field
    if (leks.type == L_STR || leks.type == L_IDENT || leks.type == L_IDENT_N) // the source lexeme contains a string of characters?
        str = new std::string(*leks.str); // duplicate the string
    return *this;
}

CLeksem::~CLeksem()
{
    switch (type)
    {
    case L_STR:
    case L_IDENT:
    case L_IDENT_N:
        if (str)
        {
#ifdef _DEBUG
            if (str->string())
                str[0] = 'X'; // change text for possible hanging references
#endif
            delete str;
        }
        break;
    }
}

/*************************************************************************/
