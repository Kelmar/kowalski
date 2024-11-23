/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Several useful debugging functions/annotations for code.
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

COptionsEditPage::COptionsEditPage(wxBookCtrlBase *parent)
    : wxPanel(parent)
{

    m_bAutoIndent = FALSE;
    m_nTabStep = 0;
    m_bAutoSyntax = FALSE;
    m_bAutoUppercase = FALSE;
    m_bFileNew = FALSE;
    m_nElement = 0;

    m_bColorChanged = false;
}

COptionsEditPage::~COptionsEditPage()
{
}

/*************************************************************************/

#if REWRITE_TO_WX_WIDGET

void COptionsEditPage::DoDataExchange(CDataExchange *pDX)
{
    if (!pDX->m_bSaveAndValidate)
    {
        CSpinButtonCtrl *pTab;
        pTab = (CSpinButtonCtrl *)GetDlgItem(IDC_OPT_ED_TAB_SPIN);
        ASSERT(pTab != NULL);
        pTab->SetRange(2, 32);		// krok tabulatora z zakresu 2..32
    }
    CPropertyPage::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_OPT_ED_BOLD_FONT, m_btnBold);
    DDX_Control(pDX, IDC_OPT_ED_COLOR, m_btnColor);
    DDX_Control(pDX, IDC_OPT_ED_ELEMENT, m_wndElement);
    DDX_Control(pDX, IDC_OPT_ED_EXAMPLE, m_wndExample);
    DDX_Check(pDX, IDC_OPT_ED_AUTO_INDENT, m_bAutoIndent);
    DDX_Text(pDX, IDC_OPT_ED_TAB_STEP, m_nTabStep);
    DDV_MinMaxInt(pDX, m_nTabStep, 2, 32);
    DDX_Check(pDX, IDC_OPT_ED_AUTO_SYNTAX, m_bAutoSyntax);
    DDX_Check(pDX, IDC_OPT_ED_AUTO_UPPER_CASE, m_bAutoUppercase);
    DDX_Check(pDX, IDC_OPT_ED_NEW_FILE, m_bFileNew);
    DDX_CBIndex(pDX, IDC_OPT_ED_ELEMENT, m_nElement);
}

BEGIN_MESSAGE_MAP(COptionsEditPage, CPropertyPage)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_EN_CHANGE(IDC_OPT_ED_TAB_STEP, OnChangeTabStep)
    ON_BN_CLICKED(IDC_OPT_ED_COLOR_SYNTAX, OnColorSyntax)
    ON_CBN_SELCHANGE(IDC_OPT_ED_ELEMENT, OnSelChangeElement)
    ON_BN_CLICKED(IDC_OPT_ED_COLOR, OnEditColor)
    ON_BN_CLICKED(IDC_OPT_ED_BOLD_FONT, OnBoldFont)
END_MESSAGE_MAP()

#endif

/*************************************************************************/
// Message handlers

bool COptionsEditPage::OnInitDialog()
{
    //CPropertyPage::OnInitDialog();

    // copy color settings
    m_wndExample.m_rgbInstruction = *ConfigSettings::color_syntax[0];
    m_wndExample.m_rgbDirective = *ConfigSettings::color_syntax[1];
    m_wndExample.m_rgbComment = *ConfigSettings::color_syntax[2];
    m_wndExample.m_rgbNumber = *ConfigSettings::color_syntax[3];
    m_wndExample.m_rgbString = *ConfigSettings::color_syntax[4];
    m_wndExample.m_rgbSelection = *ConfigSettings::color_syntax[5];

    m_wndExample.m_vbBold[0] = *ConfigSettings::syntax_font_style[0];
    m_wndExample.m_vbBold[1] = *ConfigSettings::syntax_font_style[1];
    m_wndExample.m_vbBold[2] = *ConfigSettings::syntax_font_style[2];
    m_wndExample.m_vbBold[3] = *ConfigSettings::syntax_font_style[3];
    m_wndExample.m_vbBold[4] = *ConfigSettings::syntax_font_style[4];

    OnSelChangeElement();

    return true;
}

bool COptionsEditPage::OnSetActive()
{
#if REWRITE_TO_WX_WIDGET
    // copy font settings
    m_wndExample.m_hEditorFont = COptionsViewPage::m_Text[0].font;
    m_wndExample.m_rgbBackground = COptionsViewPage::m_Text[0].bkgnd;
    m_wndExample.m_rgbText = COptionsViewPage::m_Text[0].text;

    return CPropertyPage::OnSetActive();
#endif

    return false;
}

#if 0
bool COptionsEditPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void *)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}


void COptionsEditPage::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(59994, HH_HELP_CONTEXT);
}
#endif

void COptionsEditPage::OnChangeTabStep()
{
}

void COptionsEditPage::OnColorSyntax()
{
}

void COptionsEditPage::OnBoldFont()
{
    if (bool *pBold = GetFontStyle())
    {
        *pBold = m_btnBold.GetValue();
        m_wndExample.Refresh();
        m_bColorChanged = true;
    }
}

bool *COptionsEditPage::GetFontStyle(int nIndex)
{
    if (nIndex >= 0 && nIndex < 5)
        return &m_wndExample.m_vbBold[nIndex];

    return 0;
}

bool *COptionsEditPage::GetFontStyle()
{
    return GetFontStyle(m_wndElement.GetSelection());
}

void COptionsEditPage::OnEditColor()
{
    wxColour *pColor = GetColorElement();

    if (pColor)
    {
        wxColourData data;
        data.SetColour(*pColor);

        wxColourDialog dlg(this, &data);

        if (dlg.ShowModal() == wxID_OK)
        {
            *pColor = data.GetColour();
            m_btnColor.Refresh();
            m_wndExample.Refresh();
            m_bColorChanged = true;
        }
    }
}

void COptionsEditPage::OnSelChangeElement()
{
#if REWRITE_TO_WX_WIDGET
    wxColour *p = GetColorElement();

    if (p)
        m_btnColor.SetColorRef(p);

    bool *pBold = GetFontStyle();
    if (pBold)
    {
        m_btnBold.EnableWindow();
        m_btnBold.SetCheck(*pBold ? 1 : 0);
    }
    else
    {
        m_btnBold.EnableWindow(false);
        m_btnBold.SetCheck(0);
    }
#endif
}

wxColour *COptionsEditPage::GetColorElement(int nIndex)
{
    switch (nIndex)
    {
    case 0:
        return &m_wndExample.m_rgbInstruction;

    case 1:
        return &m_wndExample.m_rgbDirective;

    case 2:
        return &m_wndExample.m_rgbComment;

    case 3:
        return &m_wndExample.m_rgbNumber;

    case 4:
        return &m_wndExample.m_rgbString;

    case 5:
        return &m_wndExample.m_rgbSelection;

    default:
        return 0;
    }
}

wxColour *COptionsEditPage::GetColorElement()
{
    return GetColorElement(m_wndElement.GetSelection());
}

/*************************************************************************/
