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

// DeasmSaveOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "DeasmSaveOptions.h"

//extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord = true);

/////////////////////////////////////////////////////////////////////////////
// CDeasmSaveOptions dialog

#if 0
CDeasmSaveOptions::CDeasmSaveOptions(wxWindow* parent /*=NULL*/)
    : wxDialog(parent, CDeasmSaveOptions::IDD)
{
    //{{AFX_DATA_INIT(CDeasmSaveOptions)
    m_uEnd = 0;
    m_uLength = 0;
    m_uStart = 0;
    m_bSaveAsData = FALSE;
    //}}AFX_DATA_INIT
}

void CDeasmSaveOptions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_HexDec(pDX, IDC_DEASM_SAVE_START, m_uStart);
    DDV_MinMaxUInt(pDX, m_uLength, 0, 65536);
    DDX_HexDec(pDX, IDC_DEASM_SAVE_END, m_uEnd);
    DDV_MinMaxUInt(pDX, m_uEnd, 0, 65535);
    DDX_HexDec(pDX, IDC_DEASM_SAVE_LENGTH, m_uLength);
    DDV_MinMaxUInt(pDX, m_uStart, 0, 65535);
    //{{AFX_DATA_MAP(CDeasmSaveOptions)
    DDX_Check(pDX, IDC_DATA, m_bSaveAsData);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDeasmSaveOptions, CDialog)
    //{{AFX_MSG_MAP(CDeasmSaveOptions)
    ON_EN_CHANGE(IDC_DEASM_SAVE_END, OnChangeDeasmEnd)
    ON_EN_CHANGE(IDC_DEASM_SAVE_LENGTH, OnChangeDeasmLength)
    ON_EN_CHANGE(IDC_DEASM_SAVE_START, OnChangeDeasmStart)
    ON_NOTIFY(UDN_DELTAPOS, IDC_DEASM_SAVE_SPIN_END, OnDeltaposDeasmSpinEnd)
    ON_NOTIFY(UDN_DELTAPOS, IDC_DEASM_SAVE_SPIN_LENGTH, OnDeltaposDeasmSpinLength)
    ON_NOTIFY(UDN_DELTAPOS, IDC_DEASM_SAVE_SPIN_START, OnDeltaposDeasmSpinStart)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeasmSaveOptions message handlers

void CDeasmSaveOptions::OnChangeDeasmEnd()
{
    if (m_bModify)
        return;

    m_bModify = true;
    CalculateNums(3);
    m_bModify = false;
}

void CDeasmSaveOptions::OnChangeDeasmLength()
{
    if (m_bModify)
        return;

    m_bModify = true;
    CalculateNums(2);
    m_bModify = false;
}

void CDeasmSaveOptions::OnChangeDeasmStart()
{
    if (m_bModify)
        return;

    m_bModify = true;
    CalculateNums(3);
    m_bModify = false;
}


#if 0

void CDeasmSaveOptions::OnDeltaposDeasmSpinEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    pNMUpDown->iPos = 3000;

    if (pNMUpDown->iDelta)
        IncEditField(GetDlgItem(IDC_DEASM_SAVE_END), pNMUpDown->iDelta,0,0xFFFF);

    *pResult = 0;
}

void CDeasmSaveOptions::OnDeltaposDeasmSpinLength(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    pNMUpDown->iPos = 3000;

    if (pNMUpDown->iDelta)
        IncEditField(GetDlgItem(IDC_DEASM_SAVE_LENGTH), pNMUpDown->iDelta,1,0x10000);

    *pResult = 0;
}

void CDeasmSaveOptions::OnDeltaposDeasmSpinStart(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    pNMUpDown->iPos = 3000;

    if (pNMUpDown->iDelta)
        IncEditField(GetDlgItem(IDC_DEASM_SAVE_START), pNMUpDown->iDelta,0,0xFFFF);

    *pResult = 0;
}
#endif

//-----------------------------------------------------------------------------

void CDeasmSaveOptions::CalculateNums(int pos)
{
    UNUSED(pos);

#if 0
    NumFmt fmt1;
    int start = ReadNumber(GetDlgItem(IDC_DEASM_SAVE_START), fmt1);

    NumFmt fmt2;
    int end = ReadNumber(GetDlgItem(IDC_DEASM_SAVE_END), fmt2);

    NumFmt fmt3;
    int len = ReadNumber(GetDlgItem(IDC_DEASM_SAVE_LENGTH), fmt3);

    if (start > end)
        return;

    if (pos == 3) // Change the length field?
    {
        if (end - start + 1 != len && end - start + 1 <= 0x10000)
            SetNumber(GetDlgItem(IDC_DEASM_SAVE_LENGTH), end - start + 1, fmt3);
    }
    else if (pos == 2) // Change the end field?
    {
        if (start + len - 1 != end && start + len - 1 <= 0xFFFF && len > 0)
            SetNumber(GetDlgItem(IDC_DEASM_SAVE_END), start + len - 1, fmt2);
    }
#endif
}

bool CDeasmSaveOptions::OnInitDialog()
{
    //CDialog::OnInitDialog();
#if 0

    static uint32_t  accel[] = {0, 1, 2, 0x10, 5, 0x100, 10, 0x400};

    CSpinButtonCtrl *pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_DEASM_SAVE_SPIN_START);
    if (pSpin)
    {
        pSpin->SetBase(16);
        pSpin->SetRange(0, 6000);
        pSpin->SetAccel(sizeof(accel) / sizeof(accel[0]), accel);
    }

    pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_DEASM_SAVE_SPIN_END);
    if (pSpin)
    {
        pSpin->SetBase(16);
        pSpin->SetRange(0, 6000);
        pSpin->SetAccel(sizeof(accel) / sizeof(accel[0]), accel);
    }

    pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_DEASM_SAVE_SPIN_LENGTH);
    if (pSpin)
    {
        pSpin->SetBase(16);
        pSpin->SetRange(0, 6000);
        pSpin->SetAccel(sizeof(accel) / sizeof(accel[0]), accel);
    }

    m_bModify = false;

#endif

    return true;
}