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

// MemoryChg.cpp : implementation file
//

#include "StdAfx.h"
#include "MemoryChg.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryChg dialog


CMemoryChg::CMemoryChg(COutputMem& /*mem*/)
    : wxDialog()
    //, m_Mem(mem)
{
    m_uAddr = 0;
    m_nData = 0;
    m_nByte = 0;
    m_bSigned = false;
}

#if REWRITE_FOR_WX_WIDGET
void CMemoryChg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_HexDec(pDX, IDC_MEMORY_ADDR, m_uAddr);
    if (theApp.m_global.m_bProc6502==2)   // 65816
        DDV_MinMaxUInt(pDX, m_uAddr, 0, 0xFFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_uAddr, 0, 65535);
    DDX_HexDec(pDX, IDC_MEMORY_DATA, reinterpret_cast<unsigned int&>(m_nData));
    DDV_MinMaxInt(pDX, m_nData, -65535, 65535);
    //{{AFX_DATA_MAP(CMemoryChg)
//  DDX_Text(pDX, IDC_MEMORY_DATA, m_nData);
    DDX_Radio(pDX, IDC_MEMORY_BYTE, m_nByte);
    DDX_Check(pDX, IDC_MEMORY_SIGNED, m_bSigned);
    //}}AFX_DATA_MAP
}
#endif

#if REWRITE_TO_WX_WIDGET

BEGIN_MESSAGE_MAP(CMemoryChg, CDialog)
    //{{AFX_MSG_MAP(CMemoryChg)
    ON_EN_CHANGE(IDC_MEMORY_ADDR, OnChangeMemoryAddr)
    ON_BN_CLICKED(IDC_MEMORY_SIGNED, OnMemorySigned)
    ON_BN_CLICKED(IDC_MEMORY_BYTE, OnMemoryByte)
    ON_BN_CLICKED(IDC_MEMORY_WORD, OnMemoryWord)
    ON_BN_CLICKED(ID_MEMORY_CHG, OnMemoryChg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CMemoryChg message handlers

void CMemoryChg::OnChangeMemoryAddr()
{
#if REWRITE_TO_WX_WIDGET
    wxString strText;
    GetDlgItemText(IDC_MEMORY_ADDR, strText);
    const char *pText = strText;

    bool bErr= false;
    int num;

    if (pText[0] == '$' && sscanf(pText + 1, "%X", &num) <= 0)
        bErr = true;
    else if (pText[0] == '0' && (pText[1] == 'x' || pText[1] == 'X') && sscanf(pText + 2, "%X", &num) <= 0)
        bErr = true;
    else if (sscanf(pText, "%u", &num) <= 0)
        bErr = true;

    if (bErr)
    {
        SetDlgItemText(IDC_MEMORY_DATA, "");
        return;
    }

    int nData;
    bool bWord = GetCheckedRadioButton(IDC_MEMORY_BYTE,IDC_MEMORY_WORD) == IDC_MEMORY_WORD;

    if (!bWord)
        nData = m_Mem[num];
    else
        nData = m_Mem[num] + (m_Mem[num + 1] << 8);

    if (IsDlgButtonChecked(IDC_MEMORY_SIGNED)) // Signed number?
        strText.Printf("%d", nData);
    else // Unsigned number
        strText.Printf(bWord ? "0x%04X" : "0x%02X", nData);

    SetDlgItemText(IDC_MEMORY_DATA, strText);
#endif
}

void CMemoryChg::OnMemorySigned()
{
    OnChangeMemoryAddr();
}

void CMemoryChg::OnMemoryByte()
{
    OnChangeMemoryAddr();
}

void CMemoryChg::OnMemoryWord()
{
    OnChangeMemoryAddr();
}

void CMemoryChg::OnOK()
{
#if REWRITE_TO_WX_WIDGET
    if (!UpdateData(true))
    {
        TRACE0("UpdateData failed during dialog termination.\n");
        // the UpdateData routine will set focus to correct item
        return;
    }
#endif

    Modify();

    EndDialog(wxID_OK);
//  CDialog::OnOK();
}

void CMemoryChg::Modify()
{
#if REWRITE_TO_WX_WIDGET
    if (GetCheckedRadioButton(IDC_MEMORY_BYTE, IDC_MEMORY_WORD) == IDC_MEMORY_BYTE)
        m_Mem[m_uAddr] = m_nData;
    else
    {
        m_Mem[m_uAddr] = uint8_t(m_nData & 0xFF);
        m_Mem[m_uAddr + 1] = uint8_t((m_nData >> 8) & 0xFF);
    }
#endif
}

void CMemoryChg::OnMemoryChg()
{
#if REWRITE_TO_WX_WIDGET
    if (!UpdateData(true))
        return;
#endif

    Modify();
}
