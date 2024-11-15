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

#ifndef MEMORYCHG_DLG_H__
#define MEMORYCHG_DLG_H__

#include "OutputMem.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryChg dialog

class CMemoryChg : public wxDialog
{
private:
    //COutputMem& m_Mem;

    void Modify();
// Construction
public:
    /* constructor */ CMemoryChg(COutputMem& mem);

    UINT m_uAddr;
    int	m_nData;
    int	m_nByte;
    bool m_bSigned;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    
    afx_msg void OnChangeMemoryAddr();
    afx_msg void OnMemorySigned();
    afx_msg void OnMemoryByte();
    afx_msg void OnMemoryWord();
    virtual void OnOK();
    afx_msg void OnMemoryChg();
};

#endif /* MEMORYCHG_DLG_H__ */
