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

// LoadCodeOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"

#include "FormatNums.h"
#include "HexValidator.h"
#include "LoadCodeOptions.h"

uint32_t CLoadCodeOptions::s_startAddress = 0;
bool     CLoadCodeOptions::s_clearMem = true;
uint32_t CLoadCodeOptions::s_fillValue = 0x00;

/////////////////////////////////////////////////////////////////////////////
// CLoadCodeOptions dialog

CLoadCodeOptions::CLoadCodeOptions()
    : wxDialog()
    , wxExtra(this)
    , m_byteValidate(&s_fillValue, NumberFormat::DollarHex, 2)
    , m_addrValidate(&s_startAddress)
{
    if (!wxXmlResource::Get()->LoadDialog(this, nullptr, "CodeLoadOptionsDlg"))
        throw ResourceError();

    wxTextCtrl *addrTxt = FindChild<wxTextCtrl>("m_addressTxt");
    wxCheckBox *clearChk = FindChild<wxCheckBox>("m_memClearChk");
    wxTextCtrl *byteTxt = FindChild<wxTextCtrl>("m_memByteTxt");

    addrTxt->SetValidator(m_addrValidate);

    clearChk->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=](wxCommandEvent &) {
        s_clearMem = clearChk->IsChecked();
    });

    byteTxt->SetValidator(m_byteValidate);

    addrTxt->SetFocus();
}

#if 0
extern void AFX_CDECL DDX_HexDec(CDataExchange *pDX, int nIDC, unsigned int &num, bool bWord = true);

void CLoadCodeOptions::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLoadCodeOptions)
    DDX_HexDec(pDX, IDC_LOAD_CODE_START, m_uStart);
    if (theApp.m_global.m_bProc6502 == 2)  // 1.3.3 support for 24-bit addressing
        DDV_MinMaxUInt(pDX, m_uStart, 0, 0xFFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_uStart, 0, 65535);
    DDX_Check(pDX, IDC_LOAD_CODE_CLR, m_bClearMem);
    DDX_HexDec(pDX, IDC_LOAD_CODE_FILL_VALUE, m_uFill, false);
    DDV_MinMaxUInt(pDX, m_uFill, 0, 255);
    //}}AFX_DATA_MAP
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadCodeOptions message handlers
