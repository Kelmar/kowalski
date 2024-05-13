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
#include "InputBase.h"

/*************************************************************************/

class CInputFile : public CInputBase
{
private:
    std::FILE *m_file;

public:
    CInputFile(const std::string &str) 
        : CInputBase(str.c_str())
        , m_file(nullptr)
    {
    }

    ~CInputFile()
    {
    }

    virtual void open()
    {
        ASSERT(m_bOpened == false); // File is already open

        m_file = std::fopen(m_strFileName.c_str(), "r");

        if (m_file == nullptr)
            throw new CFileException(errno);

        m_bOpened = true;
    }

    virtual void close()
    {
        ASSERT(m_bOpened == true);	    // The file must be opened

        std::fclose(m_file);
        m_file = nullptr;
        m_bOpened = false;
    }

    virtual void seek_to_begin()
    {
        std::fseek(m_file, 0, SEEK_SET);
        m_nLine = 0;
    }

    virtual bool read_line(std::string &buffer);

//  virtual int get_line_no()

//  virtual const std::string &get_file_name()
};

//-----------------------------------------------------------------------------

class CInputStack
{
private:
    struct FileInfo
    {
        CInputBase *m_file;
        CAsm::FileUID m_id;
    };

    std::vector<FileInfo*> m_stack;
    FileInfo *m_current;

    int m_lastId;

    CAsm::FileUID CalcIndex();

    void SetCurrent(FileInfo *newFile);

public:
    CInputStack(const std::string& fname)
        : m_current(nullptr)
        , m_lastId(0)
    {
        OpenFile(fname);
    }

    CInputStack(wxWindow* window)
        : m_current(nullptr)
        , m_lastId(0)
    {
        OpenFile(window);
    }

    CInputStack()
        : m_current(nullptr)
        , m_lastId(0)
    {}

    ~CInputStack();

    void OpenFile(const std::string& fname);
    void OpenFile(wxWindow* pWin);
    bool CloseFile();

    bool ReadLine(std::string &buffer)
    {
        return m_current->m_file->read_line(buffer);
    }

    void SeekToBegin()
    {
        m_current->m_file->seek_to_begin();
    }

    int GetLineNumber() const
    {
        return m_current->m_file->get_line_no();
    }

    int GetCount() const { return m_stack.size() + (m_current ? 1 : 0); }

    const std::string& GetFileName() const { return m_current->m_file->get_file_name(); }

    CAsm::FileUID GetFileUID() const { return m_current->m_id; }
    void SetFileUID(CAsm::FileUID fuid) { m_current->m_id = fuid; }

    bool IsPresent() const { return m_current != nullptr; }
};

/*************************************************************************/

#endif /* INPUT_H__ */

/*************************************************************************/
