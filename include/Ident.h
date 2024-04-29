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

#ifndef _ident_h_
#define _ident_h_

struct CIdent
{
    // ID info
    enum IdentInfo
    {
        I_INIT,
        I_UNDEF,        // Undefined identifier
        I_ADDRESS,      // The identifier contains the address
        I_VALUE,        // The identifier contains a numeric value
        I_MACRONAME,    // The identifier is the name of the macro definition
        I_MACROADDR     // The identifier contains the address in the macro definition
    } info;
    
    int32_t val;   // numerical value
    bool checked;  // identifier definition confirmed in the second pass of assembly
    bool variable; // variable identifier

    CIdent()
        : info(I_INIT)
        , checked(false)
        , variable(false)
    { }

    CIdent(IdentInfo info, int32_t value = 0, bool variable = false)
        : info(info)
        , val(value)
        , checked(false)
        , variable(variable)
    { }

    CIdent(const CIdent &ident)
        : info(ident.info)
        , val(ident.val)
        , checked(ident.checked)
        , variable(ident.variable)
    {
    }

    CIdent(CIdent &&ident)
        : info(std::move(ident.info))
        , val(std::move(ident.val))
        , checked(std::move(ident.checked))
        , variable(std::move(ident.variable))
    {
    }

    const CIdent &operator =(const CIdent &rhs)
    {
        info = rhs.info;
        val = rhs.val;
        checked = rhs.checked;
        variable = rhs.variable;

        return *this;
    }

    const CIdent &operator =(CIdent &&rhs)
    {
        info = std::move(rhs.info);
        val = std::move(rhs.val);
        checked = std::move(rhs.checked);
        variable = std::move(rhs.variable);

        return *this;
    }
};

#endif
