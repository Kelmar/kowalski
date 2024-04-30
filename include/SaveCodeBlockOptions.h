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

#ifndef SAVE_CODE_BLOCK_OPT_H__
#define SAVE_CODE_BLOCK_OPT_H__

// SaveCodeBlockOptions.h : header file
//
#include "FormatNums.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeBlockOptions dialog

class CSaveCodeBlockOptions : public wxDialog //, CFormatNums
{
private:
    bool m_bModify;
    void CalculateNums(int pos);

    wxControl *GetControlById(int id)
    {
        return dynamic_cast<wxControl *>(FindWindow(id));
    }

public:
    /* constructor */ CSaveCodeBlockOptions();   // standard constructor

    UINT m_uEnd;
    UINT m_uLength;
    UINT m_uStart;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual bool OnInitDialog();
    //afx_msg void OnDeltaposSpinStart(NMHDR* pNMHDR, LRESULT* pResult);
    //afx_msg void OnDeltaposSpinEnd(NMHDR* pNMHDR, LRESULT* pResult);
    //afx_msg void OnDeltaposSpinLength(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeFieldStart();
    afx_msg void OnChangeFieldEnd();
    afx_msg void OnChangeFieldLength();
};

#endif /* SAVE_CODE_BLOCK_OPT_H__ */
