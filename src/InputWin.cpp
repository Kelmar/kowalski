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

#include "InputWin.h"

/*************************************************************************/

bool CInputWin::read_line(std::string &buf)
{
    UNUSED(buf);

#if 0
    int ret = SendMessage(m_window->GetSafeHwnd(), WM_USER_GET_NEXT_LINE,
                          WPARAM(max_len), LPARAM(str));
    ASSERT(ret);

    if (ret > 0)
        m_nLine++;

    return str;
#endif

    return false;
}


const std::string &CInputWin::get_file_name()
{
    if (m_strFileName.empty())
        m_strFileName = m_window->GetLabel();

    return m_strFileName;
}


void CInputWin::seek_to_begin()
{
#if 0
    int ret = SendMessage(m_window->GetSafeHwnd(), WM_USER_NEXT_PASS, WPARAM(0), LPARAM(0));
    ASSERT(ret);
    m_nLine = 0;
#endif
}

/*************************************************************************/
