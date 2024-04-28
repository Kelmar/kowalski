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

//#include <ctime>
#include "StdAfx.h"

#include "InputBase.h"
#include "InputStack.h"
#include "InputWin.h"

/*************************************************************************/

const std::string &CInputFile::read_line(std::string &buf)
{
#if 0
    LPTSTR ptr = ReadString(buf);

    if (ptr)
        m_nLine++;
#endif

    return buf;
}

/*************************************************************************/
/*************************************************************************/

CInputStack::~CInputStack()
{
    // Close all files
    while (CloseFile())
        ;
}

/*************************************************************************/

CAsm::FileUID CInputStack::CalcIndex()
{
    return static_cast<CAsm::FileUID>(++m_lastId);
}

/*************************************************************************/

void CInputStack::SetCurrent(FileInfo *newFile)
{
    if (m_current)
        m_stack.push_back(m_current);

    m_current = newFile;
}

/*************************************************************************/

void CInputStack::OpenFile(const std::string &fname)
{
    auto file = new FileInfo();
    file->m_file = new CInputFile(fname);
    file->m_id = CalcIndex();

    SetCurrent(file);
}

void CInputStack::OpenFile(wxWindow *window)
{
    auto file = new FileInfo();
    file->m_file = new CInputWin(window);
    file->m_id = CalcIndex();

    SetCurrent(file);
}

/*************************************************************************/

bool CInputStack::CloseFile()
{
    if (!m_current)
        return false; // All files closed!

    delete m_current->m_file;
    delete m_current;

    if (m_stack.empty())
    {
        m_current = nullptr;
        return false;
    }
    else
    {
        m_current = m_stack.back();
        m_stack.pop_back();
    }

    return true;
}

/*************************************************************************/
