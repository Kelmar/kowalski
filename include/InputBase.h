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

#ifndef INPUT_BASE_H__
#define INPUT_BASE_H__

#include "Asm.h"
#include "Ident.h"
#include "DebugInfo.h"
#include "OutputMem.h"

//=============================================================================

class CInputBase    // Base class for classes reading source data
{
protected:
    int m_nLine;
    std::string m_strFileName;
    bool m_bOpened;

public:
    CInputBase(const char *str = nullptr)
        : m_strFileName(str)
    {
        m_nLine = 0;
        m_bOpened = false;
    }

    virtual ~CInputBase()
    {
        ASSERT(m_bOpened == false);
    }

    virtual void open()
    {
        m_bOpened = true;
    }

    virtual void close()
    {
        m_bOpened = false;
    }

    virtual void seek_to_begin()
    {
        m_nLine = 0;
    }

    virtual bool read_line(std::string &buffer) = 0;

    virtual int get_line_no() const
    {
        return m_nLine - 1; // Line numbering from 0
    }

    virtual const std::string &get_file_name() const { return CInputBase::m_strFileName; }
};

#endif /* INPUT_BASE_H__ */
