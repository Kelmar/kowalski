/*=======================================================================*/
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
/*=======================================================================*/

#include "StdAfx.h"

#include <wx/colordlg.h>

#include "resource.h"
#include "Options.h"
#include "ConfigSettings.h"

/*=======================================================================*/

COptionsDeasmPage::COptionsDeasmPage(wxBookCtrlBase *parent)
    : wxPanel(parent)
{
    m_ShowCode = false;
    m_bSubclassed = false;
    m_bColorChanged = false;
}

COptionsDeasmPage::~COptionsDeasmPage()
{
}

/*=======================================================================*/

#if REWRITE_TO_WX_WIDGET

void COptionsDeasmPage::DoDataExchange(CDataExchange *pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_OPT_DA_CODE, m_ShowCode);
}

BEGIN_MESSAGE_MAP(COptionsDeasmPage, CPropertyPage)
    ON_BN_CLICKED(IDC_OPT_DA_ADDR_COL, OnAddrColButton)
    ON_BN_CLICKED(IDC_OPT_DA_CODE_COL, OnCodeColButton)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#endif

/*=======================================================================*/
// Message handlers

bool COptionsDeasmPage::OnSetActive()
{
#if REWRITE_TO_WX_WIDGET
    if (!m_bSubclassed)
    {
        m_ColorButtonAddress.SubclassDlgItem(IDC_OPT_DA_ADDR_COL, this);
        m_ColorButtonAddress.SetColorRef(&m_rgbAddress);
        m_ColorButtonCode.SubclassDlgItem(IDC_OPT_DA_CODE_COL, this);
        m_ColorButtonCode.SetColorRef(&m_rgbCode);

        m_bSubclassed = true;
    }

    return CPropertyPage::OnSetActive();
#endif

    return false;
}

/*=======================================================================*/

#if 0
bool COptionsDeasmPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void *)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}


void COptionsDeasmPage::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(59993, HH_HELP_CONTEXT);
}
#endif

/*=======================================================================*/
