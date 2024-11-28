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

#include <wx/colordlg.h>

#include "resource.h"
#include "Options.h"
#include "ConfigSettings.h"

/*************************************************************************/

COptionsSymPage::COptionsSymPage(wxBookCtrlBase *parent)
    : wxPanel()
{
    if (!wxXmlResource::Get()->LoadPanel(this, parent, "OptionsSimPage"))
        throw ResourceError();

    m_nIOAddress = 0;
    m_bIOEnable = FALSE;
    m_nFinish = -1;
    m_nWndWidth = 0;
    m_nWndHeight = 0;
    m_bProtectMemory = FALSE;
    m_nProtFromAddr = 0;
    m_nProtToAddr = 0;
}

COptionsSymPage::~COptionsSymPage()
{
}

/*************************************************************************/

#if REWRITE_TO_WX_WIDGET

void COptionsSymPage::DoDataExchange(CDataExchange *pDX)
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

BEGIN_MESSAGE_MAP(COptionsSymPage, CPropertyPage)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#endif

/*************************************************************************/

#if 0
bool COptionsSymPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void *)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

void COptionsSymPage::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(59991, HH_HELP_CONTEXT);
}
#endif

/*************************************************************************/

