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

#ifndef SAVE_CODE_OPTS_H__
#define SAVE_CODE_OPTS_H__

// SaveCodeOptions.h : header file
//
#include "FormatNums.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeOptions dialog

class CSaveCodeOptions : public wxDialog //, CFormatNums
{
private:
    bool m_bModify;
    void CalculateNums(int pos);

public:
    /* constructor */ CSaveCodeOptions();

    UINT m_uEnd;
    UINT m_uLength;
    UINT m_uStart;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg void OnDeltaposSpinStart(NMHDR* pNMHDR, LRESULT* pResult);
    //afx_msg void OnDeltaposSpinEnd(NMHDR* pNMHDR, LRESULT* pResult);
    //afx_msg void OnDeltaposSpinLength(NMHDR* pNMHDR, LRESULT* pResult);
    virtual bool OnInitDialog();
    afx_msg void OnChangeFieldStart();
    afx_msg void OnChangeFieldEnd();
    afx_msg void OnChangeFieldLength();

};

#endif /* SAVE_CODE_OPTS_H__ */
