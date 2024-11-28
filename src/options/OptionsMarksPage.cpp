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

COptionsMarksPage::COptionsMarksPage(wxBookCtrlBase *parent)
    : wxPanel(parent)
{
    m_nProc6502 = -1;
    m_uBusWidth = 16;
    m_nHelpFile = 0;  //^^help
    m_bSubclassed = false;
    m_colorChanged = false;
    m_bFontChanged = false;
}

COptionsMarksPage::~COptionsMarksPage()
{
}

/*************************************************************************/

#if REWRITE_TO_WX_WIDGET

void COptionsMarksPage::DoDataExchange(CDataExchange *pDX)
{
    //if (!pDX->m_bSaveAndValidate)
    //{
    //    CSpinButtonCtrl *pTab;
    //    pTab = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_BUS_SPIN);
    //    ASSERT(pTab != NULL);
    //    pTab->SetRange(10,24); // size of the address bus
    //}

    CPropertyPage::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_OPT_6502, m_nProc6502);
    DDX_Radio(pDX, IDC_OPT_HELP_CHM, m_nHelpFile);  //^^help
    //DDX_Text(pDX, IDC_OPT_BUS_WIDTH, m_uBusWidth);
    //if (m_nProc6502==2)
    //    DDV_MinMaxUInt(pDX, m_uBusWidth, 10, 24);
    //else
    //    DDV_MinMaxUInt(pDX, m_uBusWidth, 10, 16);

}

BEGIN_MESSAGE_MAP(COptionsMarksPage, CPropertyPage)
    ON_BN_CLICKED(IDC_OPT_MARK_BRKP_COL, OnBrkpColButton)
    ON_BN_CLICKED(IDC_OPT_MARK_ERR_COL, OnErrColButton)
    ON_BN_CLICKED(IDC_OPT_MARK_PTR_COL, OnPtrColButton)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_OPT_FONT_BTN, OnOptFontBtn)
END_MESSAGE_MAP()

#endif

/*************************************************************************/
// Message handlers

#if 0
bool COptionsMarksPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void *)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

void COptionsMarksPage::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(59995, HH_HELP_CONTEXT);
}
#endif


void COptionsMarksPage::OnBrkpColButton()
{
    m_colorChanged |= options::SelectColor(this, &m_rgbBreakpoint, &m_ColorButtonBreakpoint);
}

void COptionsMarksPage::OnErrColButton()
{
    m_colorChanged |= options::SelectColor(this, &m_rgbError, &m_ColorButtonError);
}

void COptionsMarksPage::OnPtrColButton()
{
    m_colorChanged |= options::SelectColor(this, &m_rgbPointer, &m_ColorButtonPointer);
}

bool COptionsMarksPage::OnSetActive()
{
#if REWRITE_TO_WX_WIDGET
    if (!m_bSubclassed)
    {
        m_ColorButtonPointer.SubclassDlgItem(IDC_OPT_MARK_PTR_COL, this);
        m_ColorButtonPointer.SetColorRef(&m_rgbPointer);
        m_ColorButtonBreakpoint.SubclassDlgItem(IDC_OPT_MARK_BRKP_COL, this);
        m_ColorButtonBreakpoint.SetColorRef(&m_rgbBreakpoint);
        m_ColorButtonError.SubclassDlgItem(IDC_OPT_MARK_ERR_COL, this);
        m_ColorButtonError.SetColorRef(&m_rgbError);

        m_bSubclassed = true;
    }

    return CPropertyPage::OnSetActive();
#endif

    return false;
}

void COptionsMarksPage::OnOptFontBtn()
{
#if REWRITE_TO_WX_WIDGET
    CFontDialog fnt(&m_LogFont, CF_SCREENFONTS | CF_FIXEDPITCHONLY |
        CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST | CF_SCRIPTSONLY);

    if (fnt.ShowModal() == wxID_OK)
    {
        m_bFontChanged = true;
        m_LogFont = fnt.m_lf;
        SetDlgItemText(IDC_OPT_FONT_NAME, m_LogFont.lfFaceName);
    }
#endif
}

/*************************************************************************/
