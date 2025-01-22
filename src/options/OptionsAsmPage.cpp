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

COptionsAsmPage::COptionsAsmPage(wxBookCtrlBase *parent)
    : wxPanel()
{
    if (!wxXmlResource::Get()->LoadPanel(this, parent, "OptionsAsmPage"))
        throw ResourceError();

    m_nCaseSensitive = -1;
    m_nSwapBin = true;
    m_bGenerateListing = false;
    m_strListingFile = "";
    m_bGenerateBRKExtraByte = false;
    m_uBrkExtraByte = 0;
}

COptionsAsmPage::~COptionsAsmPage()
{
}

/*=======================================================================*/

#if REWRITE_TO_WX_WIDGET

void COptionsAsmPage::DoDataExchange(CDataExchange *pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_OPT_ASM_CASE_Y, m_nCaseSensitive);
    DDX_Radio(pDX, IDC_OPT_NO_SWAPBIN, m_nSwapBin);
    DDX_Check(pDX, IDC_OPT_ASM_GENERATE_LIST, m_bGenerateListing);
    DDX_Text(pDX, IDC_OPT_ASM_FILE_LISTING, m_strListingFile);
    DDX_Check(pDX, IDC_OPT_ASM_GENERATE_BYTE, m_bGenerateBRKExtraByte);
    DDX_HexDec(pDX, IDC_OPT_ASM_EXTRA_BYTE, m_uBrkExtraByte, false);
    DDV_MinMaxUInt(pDX, m_uBrkExtraByte, 0, 0xFF);
}

BEGIN_MESSAGE_MAP(COptionsAsmPage, CPropertyPage)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_OPT_ASM_CHOOSE_FILE, OnOptAsmChooseFile)
END_MESSAGE_MAP()

#endif

/*=======================================================================*/
// Message handlers

#if 0

bool COptionsAsmPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void *)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}
#endif 

void COptionsAsmPage::OnContextMenu(wxWindow *pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(59992, HH_HELP_CONTEXT);
}


void COptionsAsmPage::OnOptAsmChooseFile()
{
#if REWRITE_FOR_WX_WIDGET
    sd::string filter;
    filter.LoadString(IDS_ASM_OPT_LIST_FILES);

    // TODO: i18l title string

    wxFileDialog dlg(this, "Save Assembly Listing", "", "", filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    SetDlgItemText(IDC_OPT_ASM_FILE_LISTING, dlg.GetPath());
#endif
}

/*=======================================================================*/
