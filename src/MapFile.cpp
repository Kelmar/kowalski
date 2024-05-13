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
#include "MapFile.h"


std::string CMapFile::GetPath(CAsm::FileUID fuid)
{
    if (fuid == 0 || fuid > m_lastUID)
    {
        ASSERT(false); // invalid FUID value
        return NULL;
    }

    return m_fuidToPath[fuid];
}

CAsm::FileUID CMapFile::GetFileUID(const std::string &path)
{
    auto itr = m_pathToFuid.find(path);

    if (itr == m_pathToFuid.end())
    {
        // Not found, add a new mapping.
        ++m_lastUID;
        m_fuidToPath[m_lastUID] = path;
        m_pathToFuid[path] = (CAsm::FileUID)m_lastUID;

        return (CAsm::FileUID)m_lastUID;
    }

    return itr->second;
}
