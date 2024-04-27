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

#ifndef DEASM_SAVE_OPTIONS_H__
#define DEASM_SAVE_OPTIONS_H__

// DeasmSaveOptions.h : header file
//
#include "FormatNums.h"

/////////////////////////////////////////////////////////////////////////////
// CDeasmSaveOptions dialog

class CDeasmSaveOptions : public wxDialog
{
private:
    bool m_bModify;
    void CalculateNums(int pos);

public:
    CDeasmSaveOptions(wxWindow* parent = nullptr);   // standard constructor

    // Dialog Data
    
    UINT m_uEnd;
    UINT m_uLength;
    UINT m_uStart;
    bool m_bSaveAsData;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
    afx_msg void OnChangeDeasmEnd();
    afx_msg void OnChangeDeasmLength();
    afx_msg void OnChangeDeasmStart();

    //afx_msg void OnDeltaposDeasmSpinEnd(NMHDR* pNMHDR, LRESULT* pResult);
    //afx_msg void OnDeltaposDeasmSpinLength(NMHDR* pNMHDR, LRESULT* pResult);
    //afx_msg void OnDeltaposDeasmSpinStart(NMHDR* pNMHDR, LRESULT* pResult);

    virtual bool OnInitDialog();
};

#endif /* DEASM_SAVE_OPTIONS_H__ */
