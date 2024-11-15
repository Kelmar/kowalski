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

#ifndef IDENT_INFO_H__
#define IDENT_INFO_H__

// Window Display Support Class (CListView)
// displaying defined labels (after assembly)

#include "IdentInfoDoc.h"
#include "IdentInfoFrame.h"
#include "IdentInfoView.h"
#include "resource.h"

class CIdentInfo : public CIdentInfoFrame
{
private:
    //CDebugInfo *m_pDebugInfo;
    CIdentInfoDoc m_doc;

public:
    CIdentInfo()
        //: m_pDebugInfo(nullptr)
    {
    }

    /* constructor */ CIdentInfo(CDebugInfo *debugInfo);
    virtual          ~CIdentInfo();

    void SetDebugInfo(CDebugInfo *debugInfo)
    {
        m_doc.SetDebugInfo(debugInfo);
    }

    bool Create(CDebugInfo *debugInfo = nullptr);
};

#endif /* IDENT_INFO_H__ */
