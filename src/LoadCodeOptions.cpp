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

#include "StdAfx.h"

#include "FormatNums.h"
#include "HexValidator.h"
#include "LoadCodeOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CLoadCodeOptions dialog

LoadCodeOptionsDlg::LoadCodeOptionsDlg(LoadCodeState *state)
    : wxDialog()
    , wxExtra(this)
    , m_state(state)
{
    if (!wxXmlResource::Get()->LoadDialog(this, nullptr, "CodeLoadOptionsDlg"))
        throw ResourceError();

    HexValidator addrValidate(&m_state->StartAddress);
    HexValidator byteValidate(&m_state->FillByte, NumberFormat::DollarHex, 2);

    if (wxGetApp().m_global.m_procType == ProcessorType::WDC65816)
        addrValidate.SetMaxValue(0x00FF'FFFF); // 1.3.3 support for 24-bit addressing
    else
        addrValidate.SetMaxValue(0x0000'FFFF);

    //byteValidate.SetMaxValue(255);

    wxTextCtrl *addrTxt = FindChild<wxTextCtrl>("m_addressTxt");
    wxCheckBox *clearChk = FindChild<wxCheckBox>("m_memClearChk");
    //wxTextCtrl *byteTxt = FindChild<wxTextCtrl>("m_memByteTxt");
    wxSpinCtrl *byteVal = FindChild<wxSpinCtrl>("m_byteSpin");

    byteVal->SetBase(16); // Display in Hex

    addrTxt->SetValidator(addrValidate);

    clearChk->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [=,this](wxCommandEvent &) {
        m_state->ClearMemory = clearChk->IsChecked();
    });

    //byteTxt->SetValidator(byteValidate);

    addrTxt->SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// CLoadCodeOptions message handlers
