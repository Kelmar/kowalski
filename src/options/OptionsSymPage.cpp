/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*************************************************************************/

#include "StdAfx.h"
#include "6502.h"

#include "IOWindow.h"

#include <wx/colordlg.h>

#include "resource.h"

#include "sim.h"
#include "sim_priv.h"

#include "Options.h"
#include "options/OptionsSymPage.h"

#include "ConfigSettings.h"

/*************************************************************************/

// TODO: Remove direct reference to IO Config
// Really the IO config belongs in its own configuration page of sorts.
extern IOWindowConfig s_ioConfig;

/*************************************************************************/

OptionsSymPage::OptionsSymPage(wxBookCtrlBase *parent)
    : OptionsPage()
    , wxExtra(this)
{
    if (!wxXmlResource::Get()->LoadPanel(this, parent, "OptionsSimPage"))
        throw ResourceError();

    BindChildren();
    InitChildren();
    ReadConfig();
}

OptionsSymPage::~OptionsSymPage()
{
}

/*************************************************************************/
/**
 * @brief Bind pointers to created controls.
 */
void OptionsSymPage::BindChildren()
{
    // Processor
    WX_BIND_CHILD(m_cpuSelect);

    // Finish Running Program By
    WX_BIND_CHILD(m_brkRadio);
    WX_BIND_CHILD(m_rtsRadio);
    WX_BIND_CHILD(m_dbRadio);

    // In/Out Memory Area
    WX_BIND_CHILD(m_ioActiveChk);
    WX_BIND_CHILD(m_ioAddrTxt);

    // Input/Output Window
    WX_BIND_CHILD(m_colSpin);
    WX_BIND_CHILD(m_rowSpin);

    // Memory Protection
    WX_BIND_CHILD(m_writeDetectChk);
    WX_BIND_CHILD(m_writeStartTxt);
    WX_BIND_CHILD(m_writeEndTxt);
}

/*************************************************************************/
/**
 * @brief Initialize selection values and validators.
 */
void OptionsSymPage::InitChildren()
{
    // Processor
    // TODO: Remove hard coded options
    m_cpuSelect->Append(_("Basic 6502"));
    m_cpuSelect->Append(_("65C02, 6501"));
    m_cpuSelect->Append(_("65816"));

    // Input/Output Window
    m_colSpin->SetRange(1, 255);
    m_rowSpin->SetRange(1, 255);

    // In/Out Memory Area
    HexValidator ioValidate(&m_ioAddress);
    m_ioAddrTxt->SetValidator(ioValidate);

    // Memory Protection
    HexValidator startValidator(&m_protectStart);
    HexValidator endValidator(&m_protectEnd);

    m_writeStartTxt->SetValidator(startValidator);
    m_writeEndTxt->SetValidator(endValidator);
}

/*************************************************************************/
/**
 * @brief Read values from the configuration into the controls.
 */
void OptionsSymPage::ReadConfig()
{
    // Processor
    m_cpuSelect->SetSelection((int)s_simConfig.Processor);

    // Finish Running Program by
    m_brkRadio->SetValue(s_simConfig.SimFinish == CAsm::Finish::FIN_BY_BRK);
    m_rtsRadio->SetValue(s_simConfig.SimFinish == CAsm::Finish::FIN_BY_RTS);
    m_dbRadio->SetValue(s_simConfig.SimFinish == CAsm::Finish::FIN_BY_DB);

    // In/Out Memory Area
    m_ioAddress = s_simConfig.IOAddress;

    // Input/Output Window
    m_colSpin->SetValue(s_ioConfig.Columns);
    m_rowSpin->SetValue(s_ioConfig.Rows);

    // Memory Protection
    m_protectStart = s_simConfig.ProtectStart;
    m_protectEnd = s_simConfig.ProtectEnd;
}

/*************************************************************************/
/**
 * @brief Set configuration from values presaented in the controls.
 */
void OptionsSymPage::SetConfig()
{
    // Processor
    s_simConfig.Processor = (ProcessorType)m_cpuSelect->GetSelection();

    // Finish Running Program by
    if (m_brkRadio->GetValue())
        s_simConfig.SimFinish = CAsm::Finish::FIN_BY_BRK;

    if (m_rtsRadio->GetValue())
        s_simConfig.SimFinish = CAsm::Finish::FIN_BY_RTS;

    if (m_dbRadio->GetValue())
        s_simConfig.SimFinish = CAsm::Finish::FIN_BY_DB;

    // In/Out Memory Area
    s_simConfig.IOEnable = m_ioActiveChk->GetValue();
    s_simConfig.IOAddress = m_ioAddress;

    // Input/Output Window
    s_ioConfig.Columns = m_colSpin->GetValue();
    s_ioConfig.Rows = m_rowSpin->GetValue();

    // Memory Protection
    s_simConfig.ProtectMemory = m_writeDetectChk->GetValue();
    s_simConfig.ProtectStart = m_protectStart;
    s_simConfig.ProtectEnd = m_protectEnd;
}

/*************************************************************************/

void OptionsSymPage::AbortChanges()
{
    // Nothing to do, we'll just re-read from the configuration on next load.
}

/*************************************************************************/

void OptionsSymPage::SaveChanges()
{
    SetConfig();

    wxGetApp().simulatorController().SaveConfig();
    wxGetApp().mainFrame()->ioWindow()->SaveConfig();
}

/*************************************************************************/

#if REWRITE_TO_WX_WIDGET

void OptionsSymPage::DoDataExchange(CDataExchange *pDX)
{
    if (!pDX->m_bSaveAndValidate)
    {
        CSpinButtonCtrl *pCols;
        pCols = (CSpinButtonCtrl *)GetDlgItem(IDC_OPT_SYM_W_SPIN);
        ASSERT(pCols != NULL);
        pCols->SetRange(1, 255); // number of terminal columns

        CSpinButtonCtrl *pRows;
        pRows = (CSpinButtonCtrl *)GetDlgItem(IDC_OPT_SYM_H_SPIN);
        ASSERT(pRows != NULL);
        pRows->SetRange(1, 255); // number of terminal lines
    }

    CPropertyPage::DoDataExchange(pDX);

    DDX_Check(pDX, IDC_OPT_SYM_IO_ENABLE, m_bIOEnable);
    DDX_Radio(pDX, IDC_OPT_SYM_FIN_BRK, m_nFinish);
    DDX_Text(pDX, IDC_OPT_SYM_IO_WND_W, m_nWndWidth);
    DDX_Text(pDX, IDC_OPT_SYM_IO_WND_H, m_nWndHeight);
    DDX_Check(pDX, IDC_OPT_SYM_PROTECT_MEM, m_bProtectMemory);

    DDX_HexDec(pDX, IDC_OPT_SYM_IO_ADDR, m_nIOAddress);
    DDV_MinMaxUInt(pDX, m_nIOAddress, 0, 65535);
    DDX_HexDec(pDX, IDC_OPT_SYM_PROT_FROM, m_nProtFromAddr);

    ProcessorType procType = wxGetApp().m_global.GetProcType();

    if (procType == ProcessorType::WDC65816)
        DDV_MinMaxUInt(pDX, m_nProtFromAddr, 0, 0x00FFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_nProtFromAddr, 0, 0x0000FFFF);

    DDX_HexDec(pDX, IDC_OPT_SYM_PROT_TO, m_nProtToAddr);

    if (procType == ProcessorType::WDC65816)
        DDV_MinMaxUInt(pDX, m_nProtToAddr, 0, 0x00FFFFFF);
    else
        DDV_MinMaxUInt(pDX, m_nProtToAddr, 0, 0x0000FFFF);
}

BEGIN_MESSAGE_MAP(OptionsSymPage, CPropertyPage)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#endif

/*************************************************************************/

#if 0
bool OptionsSymPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void *)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

void OptionsSymPage::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(59991, HH_HELP_CONTEXT);
}
#endif

/*************************************************************************/

