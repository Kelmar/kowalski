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

/*************************************************************************/

char* CLeksem::CLString::s_ptr_1 = 0;
char* CLeksem::CLString::s_ptr_2 = 0;
char CLeksem::CLString::s_buf_1[1028];
char CLeksem::CLString::s_buf_2[1028];
const size_t CLeksem::CLString::s_cnMAX = 1024;

CLeksem::CLeksem(const CLeksem &leks) : type(leks.type)
{
    memcpy(this, &leks, sizeof(CLeksem));	// copy whole union and type field
    if (leks.type == L_STR || leks.type == L_IDENT || leks.type == L_IDENT_N)	// contains string?
        str = new CLString(*leks.str);	// duplicate string
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
        str = new CLString(*leks.str); // duplicate the string
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
            if (str->GetLength())
                str->SetAt(0, 'X'); // change text for possible hanging references
#endif
            delete str;
        }
        break;
    }
}

/*************************************************************************/
