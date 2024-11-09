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

#include "StdAfx.h"
#include "Asm.h"

#include "InputStack.h"

/*************************************************************************/

void CInputFile::open()
{
    ASSERT(!m_file); // File is already open
    ASSERT(!m_path.empty()); // Must have a file name!

    m_file = std::fopen(m_path.c_str(), "r");

    if (m_file == nullptr)
        throw FileError(FileError::SysError);
}

/*************************************************************************/

void CInputFile::close()
{
    if (!m_file)
        return;

    std::fclose(m_file);
    m_file = nullptr;
}

/*************************************************************************/

void CInputFile::seek_to_begin()
{
    ASSERT(m_file); // The file must be opened

    std::fseek(m_file, 0, SEEK_SET);
    m_lineNo = 0;
}

/*************************************************************************/

bool CInputFile::read_line(std::string &buf)
{
    char *res = ::fgets(buf.data(), buf.capacity(), m_file);

    if (res)
    {
        m_lineNo++;
        return true;
    }

    return false;
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

void CInputStack::SetCurrent(CInputFile *newFile)
{
    if (m_current)
        m_stack.push_back(m_current);

    m_current = newFile;
}

/*************************************************************************/

void CInputStack::OpenFile(const std::string &fname)
{
    auto file = new CInputFile(fname, CalcIndex());

    SetCurrent(file);
}

/*************************************************************************/

bool CInputStack::CloseFile()
{
    if (!m_current)
        return false; // All files closed!

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
