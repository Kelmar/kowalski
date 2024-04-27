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

// MemoryDoc.h : header file
//

#ifndef MEMORY_DOC_H__
#define MEMORY_DOC_H__

#include "Asm.h"

class COutputMem;

/////////////////////////////////////////////////////////////////////////////
// CMemoryDoc document
class CMemoryDoc : public wxDocument
{
protected:
    virtual bool DoSave(const char *pathName, bool replace = true);

    // Attributes
public:
    uint32_t m_address;
    uint16_t m_stackPtr;

    COutputMem *m_pMem;

    virtual ~CMemoryDoc();

    // Operations
public:
    CMemoryDoc();

    void SetData(COutputMem* pMem, uint32_t address)
    {
        m_address = address;
        m_pMem = pMem;
    }

public:
    virtual void Serialize(CArchive &ar);   // overridden for document i/o

protected:
    virtual bool OnNewDocument();

public:

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif /* MEMORY_DOC_H__ */
