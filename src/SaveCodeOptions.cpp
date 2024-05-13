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

// SaveCodeOptions.cpp : implementation file
//

#include "StdAfx.h"
//#include "6502.h"
#include "SaveCodeOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeOptions dialog

CSaveCodeOptions::CSaveCodeOptions()
    : wxDialog()
{
    m_uEnd = 0;
    m_uLength = 0;
    m_uStart = 0;
}

#if REWRITE_TO_WX_WIDGET

void CSaveCodeOptions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSaveCodeOptions)
    DDX_HexDec(pDX, IDC_SAVE_CODE_OPT_1_END, m_uEnd);
    if (theApp.m_global.m_bProc6502==2)  // 1.3.3 support for 24-bit addressing
        DDV_MinMaxUInt(pDX, m_uEnd, 0, 0xFFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_uEnd, 0, 0xFFFF);

    DDX_HexDec(pDX, IDC_SAVE_CODE_OPT_1_LENGTH, m_uLength);
    if (theApp.m_global.m_bProc6502==2)  // 1.3.3 support for 24-bit addressing
        DDV_MinMaxUInt(pDX, m_uLength, 0, 0xFFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_uLength, 0, 65536);
    DDX_HexDec(pDX, IDC_SAVE_CODE_OPT_1_START, m_uStart);
    if (theApp.m_global.m_bProc6502==2)  // 1.3.3 support for 24-bit addressing
        DDV_MinMaxUInt(pDX, m_uStart, 0, 0xFFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_uStart, 0, 65535);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveCodeOptions, CDialog)
    //{{AFX_MSG_MAP(CSaveCodeOptions)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SAVE_CODE_OPT_1_SPIN_START, OnDeltaposSpinStart)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SAVE_CODE_OPT_1_SPIN_END, OnDeltaposSpinEnd)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SAVE_CODE_OPT_1_SPIN_LENGTH, OnDeltaposSpinLength)
    ON_EN_CHANGE(IDC_SAVE_CODE_OPT_1_START, OnChangeFieldStart)
    ON_EN_CHANGE(IDC_SAVE_CODE_OPT_1_END, OnChangeFieldEnd)
    ON_EN_CHANGE(IDC_SAVE_CODE_OPT_1_LENGTH, OnChangeFieldLength)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeOptions message handlers

#if REWRITE_TO_WX_WIDGET

void CSaveCodeOptions::OnDeltaposSpinStart(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    pNMUpDown->iPos = 3000;
    if (pNMUpDown->iDelta)
        if (theApp.m_global.m_bProc6502==2)  // 1.3.3 support for 24-bit addressing
            IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_1_START), pNMUpDown->iDelta,0,0xFFFFFF);
        else
            IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_1_START), pNMUpDown->iDelta,0,0xFFFF);
    *pResult = 0;
}

void CSaveCodeOptions::OnDeltaposSpinEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    pNMUpDown->iPos = 3000;
    if (pNMUpDown->iDelta)
        if (theApp.m_global.m_bProc6502==2)  // 1.3.3 support for 24-bit addressing
            IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_1_END), pNMUpDown->iDelta,0,0xFFFFFF);
        else
            IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_1_END), pNMUpDown->iDelta,0,0xFFFF);
    *pResult = 0;
}

void CSaveCodeOptions::OnDeltaposSpinLength(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    pNMUpDown->iPos = 3000;
    if (pNMUpDown->iDelta)
        if (theApp.m_global.m_bProc6502==2)  // 1.3.3 support for 24-bit addressing
            IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_1_LENGTH), pNMUpDown->iDelta,1,0x1000000);
        else
            IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_1_LENGTH), pNMUpDown->iDelta,1,0x10000);
    *pResult = 0;
}

#endif

//-----------------------------------------------------------------------------

bool CSaveCodeOptions::OnInitDialog()
{
#if REWRITE_TO_WX_WIDGET
    CDialog::OnInitDialog();

    static UDACCEL accel[]= {0,1, 2,0x10, 5,0x100, 10,0x400};

    CSpinButtonCtrl *pSpin= (CSpinButtonCtrl *)GetDlgItem(IDC_SAVE_CODE_OPT_1_SPIN_START);
    if (pSpin)
    {
        pSpin->SetBase(16);
        pSpin->SetRange(0,6000);
        pSpin->SetAccel(sizeof(accel)/sizeof(accel[0]),accel);
    }
    pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_SAVE_CODE_OPT_1_SPIN_END);
    if (pSpin)
    {
        pSpin->SetBase(16);
        pSpin->SetRange(0,6000);
        pSpin->SetAccel(sizeof(accel)/sizeof(accel[0]),accel);
    }
    pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_SAVE_CODE_OPT_1_SPIN_LENGTH);
    if (pSpin)
    {
        pSpin->SetBase(16);
        pSpin->SetRange(0,6000);
        pSpin->SetAccel(sizeof(accel)/sizeof(accel[0]),accel);
    }

    m_bModify = FALSE;

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
#endif

    return true;
}

//-----------------------------------------------------------------------------

void CSaveCodeOptions::CalculateNums(int pos)
{
    UNUSED(pos);

#if REWRITE_TO_WX_WIDGET
    NumFmt fmt1;
    int start= ReadNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_START),fmt1);
    NumFmt fmt2;
    int end= ReadNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_END),fmt2);
    NumFmt fmt3;
    int len= ReadNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_LENGTH),fmt3);

    if (start > end)
        return;
    if (pos==3)		// change the length field?
    {
        if (end-start+1 != len && end-start+1 <= 0x10000 && !(theApp.m_global.m_bProc6502==2))  // 1.3.3 support for 24-bit addressing
            SetNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_LENGTH),end-start+1,fmt3);
        else if (end-start+1 != len && end-start+1 <= 0x1000000)
            SetNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_LENGTH),end-start+1,fmt3);
    }
    else if (pos==2)	// change the end field?
    {
        if (start+len-1 != end && start+len-1 <= 0xFFFF && len > 0 && !(theApp.m_global.m_bProc6502==2))  // 1.3.3 support for 24-bit addressing
            SetNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_END),start+len-1,fmt2);
        else if (start+len-1 != end && start+len-1 <= 0xFFFFFF && len > 0)
            SetNumber(GetDlgItem(IDC_SAVE_CODE_OPT_1_END),start+len-1,fmt2);
    }
#endif
}


void CSaveCodeOptions::OnChangeFieldStart()
{
    if (m_bModify)
        return;
    m_bModify = TRUE;
    CalculateNums(3);
    m_bModify = FALSE;
}

void CSaveCodeOptions::OnChangeFieldEnd()
{
    if (m_bModify)
        return;
    m_bModify = TRUE;
    CalculateNums(3);
    m_bModify = FALSE;
}

void CSaveCodeOptions::OnChangeFieldLength()
{
    if (m_bModify)
        return;
    m_bModify = TRUE;
    CalculateNums(2);
    m_bModify = FALSE;
}
