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

#ifndef INPUT_H__
#define INPUT_H__

#include "StdAfx.h"

/*=======================================================================*/

class CInputFile
{
private:
    std::string m_path;
    CAsm::FileUID m_id;
    std::FILE *m_file;

    int m_lineNo;

    virtual void open();
    virtual void close();

public:
    CInputFile(const std::string &path, CAsm::FileUID id)
        : m_path(path)
        , m_id(id)
        , m_file(nullptr)
        , m_lineNo(0)
    {
        open();
    }

    virtual ~CInputFile() { close(); }

    CAsm::FileUID id(void) const { return m_id; }
    void id(CAsm::FileUID newID) { m_id = newID; }

    void seek_to_begin();

    bool read_line(std::string &buffer);

    int lineNumber(void) const { return m_lineNo; }

    std::string path(void) const { return m_path; }
};

//-----------------------------------------------------------------------------

class CInputStack
{
private:
    std::vector<CInputFile*> m_stack;
    CInputFile *m_current;

    int m_lastId;

    CAsm::FileUID CalcIndex();

    void SetCurrent(CInputFile *newFile);

public:
    CInputStack(const std::string& fname)
        : m_current(nullptr)
        , m_lastId(0)
    {
        OpenFile(fname);
    }

    CInputStack()
        : m_current(nullptr)
        , m_lastId(0)
    {}

    virtual ~CInputStack();

    void OpenFile(const std::string& fname);
    bool CloseFile();

    bool ReadLine(std::string &buffer)
    {
        return m_current->read_line(buffer);
    }

    void SeekToBegin()
    {
        m_current->seek_to_begin();
    }

    int GetLineNumber() const { return m_current->lineNumber(); }

    int GetCount() const { return m_stack.size() + (m_current ? 1 : 0); }

    std::string GetFileName() const { return m_current->path(); }

    CAsm::FileUID GetFileUID() const { return m_current->id(); }
    void SetFileUID(CAsm::FileUID fuid) { m_current->id(fuid); }

    bool IsPresent() const { return m_current != nullptr; }
};

/*=======================================================================*/

#endif /* INPUT_H__ */

/*=======================================================================*/
