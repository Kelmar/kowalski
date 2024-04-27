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

#ifndef IDENT_INFO_DOC_H__
#define IDENT_INFO_DOC_H__

// IdentInfoDoc.h : header file
//
#include "DebugInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoDoc document

class CIdentInfoDoc : public wxDocument
{
private:
    CDebugInfo *m_debug;

public:
    /* constructor */ CIdentInfoDoc();
    virtual          ~CIdentInfoDoc();

    void GetIdent(int index, std::string &str, CIdent &info)
    {
        ASSERT(m_debug != nullptr);
        m_debug->GetIdent(index, str, info);
    }

    int GetIdentCount()
    {
        ASSERT(m_debug != NULL);
        return m_debug->GetIdentCount();
    }

    void SetDebugInfo(CDebugInfo *debug)
    {
        m_debug = debug;
    }

    virtual void Serialize(CArchive& ar);   // overridden for document i/o

protected:
    virtual bool OnNewDocument();

public:
    
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

#endif /* IDENT_INFO_DOC_H__ */
