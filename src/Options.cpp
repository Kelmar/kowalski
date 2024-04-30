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

// Options.cpp : implementation file
//

#include "StdAfx.h"

#include <wx/colordlg.h>

#include "resource.h"
#include "Options.h"
#include "ConfigSettings.h"

//static HH_POPUP hPop;

bool SelectColor(wxPanel *parent, wxColour *baseColor, CColorButton *colorBtn)
{
    ASSERT(parent);
    ASSERT(baseColor);
    ASSERT(cldBtn);

    wxColourData data;
    data.SetColour(*baseColor);

    wxColourDialog dlg(parent, &data);

    if (dlg.ShowModal() == wxID_OK)
    {
        *baseColor = data.GetColour();
        return true;
        colorBtn->Refresh();
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// COptions

/*
COptions::COptions(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
  : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

COptions::COptions(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
  : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}
*/

COptions::COptions(UINT iSelectPage)
    : wxNotebook()
{
    //m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_HASHELP | PSH_USECALLBACK;
    //m_psh.pfnCallback = &PropSheetProc;

    // TODO: i18n for titles

    InsertPage(0, &m_SymPage, "Symbols", iSelectPage == 0);
    InsertPage(1, &m_AsmPage, "Assembly", iSelectPage == 1);
    InsertPage(2, &m_EditPage, "Editor", iSelectPage == 2);
    InsertPage(3, &m_DeasmPage, "Disassembler", iSelectPage == 3);
    InsertPage(4, &m_MarksPage, "Marks", iSelectPage == 4);
    InsertPage(5, &m_ViewPage, "Views", iSelectPage == 5);

    m_nLastActivePageIndex = iSelectPage;

    // set up HH_POPUP defaults for all context sensitive help
    // Initialize structure to NULLs
#if REWRITE_TO_WX_WIDGET
    memset(&hPop, 0, sizeof(hPop));
    // Set size of structure
    hPop.cbStruct = sizeof(hPop);
    hPop.clrBackground = RGB(255, 255, 208); // Yellow background color
    hPop.clrForeground = -1; // Font color //  black font
    hPop.rcMargins.top = -1;
    hPop.rcMargins.left = -1;
    hPop.rcMargins.bottom = -1;
    hPop.rcMargins.right = -1;
    hPop.pszFont = NULL; // Font
#endif

}

COptions::~COptions()
{
}

#if REWRITE_TO_WX_WIDGET

int CALLBACK COptions::PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    if (uMsg == PSCB_INITIALIZED && ::IsWindow(hwndDlg))
        CWnd::FromHandle(hwndDlg)->ModifyStyleEx(0, WS_EX_CONTEXTHELP);  // w��czenie pomocy kontekstowej

    return 0;
}

#endif

int COptions::GetLastActivePage()
{
    return m_nLastActivePageIndex;
}

#if REWRITE_TO_WX_WIDGET

BEGIN_MESSAGE_MAP(COptions, CPropertySheet)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

bool COptions::OnCommand(WPARAM wParam, LPARAM lParam)
{
#if REWRITE_TO_WX_WIDGET
    m_nLastActivePageIndex = GetActiveIndex();
    return CPropertySheet::OnCommand(wParam, lParam);
#endif

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// COptionsSymPage property page

COptionsSymPage::COptionsSymPage()
    : wxPanel()
{
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

#if REWRITE_TO_WX_WIDGET

void COptionsSymPage::DoDataExchange(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate)
    {
        CSpinButtonCtrl *pCols;
        pCols = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_SYM_W_SPIN);
        ASSERT(pCols != NULL);
        pCols->SetRange(1, 255);		// ilo�� kolumn terminala

        CSpinButtonCtrl *pRows;
        pRows = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_SYM_H_SPIN);
        ASSERT(pRows != NULL);
        pRows->SetRange(1, 255);		// ilo�� wierszy terminala
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
    DDV_MinMaxUInt(pDX, m_nProtFromAddr, 0, 0xffff);
    DDX_HexDec(pDX, IDC_OPT_SYM_PROT_TO, m_nProtToAddr);
    DDV_MinMaxUInt(pDX, m_nProtToAddr, 0, 0xffff);
}

BEGIN_MESSAGE_MAP(COptionsSymPage, CPropertyPage)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsEditPage property page

COptionsEditPage::COptionsEditPage()
    : wxPanel()
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

#if REWRITE_TO_WX_WIDGET

void COptionsEditPage::DoDataExchange(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate)
    {
        CSpinButtonCtrl *pTab;
        pTab = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_ED_TAB_SPIN);
        ASSERT(pTab != NULL);
        pTab->SetRange(2,32);		// krok tabulatora z zakresu 2..32
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

/////////////////////////////////////////////////////////////////////////////
// COptionsEditPage message handlers

bool COptionsEditPage::OnInitDialog()
{
    //CPropertyPage::OnInitDialog();

    // copy color settings
    m_wndExample.m_rgbInstruction = *ConfigSettings::color_syntax[0];
    m_wndExample.m_rgbDirective   = *ConfigSettings::color_syntax[1];
    m_wndExample.m_rgbComment     = *ConfigSettings::color_syntax[2];
    m_wndExample.m_rgbNumber      = *ConfigSettings::color_syntax[3];
    m_wndExample.m_rgbString      = *ConfigSettings::color_syntax[4];
    m_wndExample.m_rgbSelection   = *ConfigSettings::color_syntax[5];

    m_wndExample.m_vbBold[0]      = *ConfigSettings::syntax_font_style[0];
    m_wndExample.m_vbBold[1]      = *ConfigSettings::syntax_font_style[1];
    m_wndExample.m_vbBold[2]      = *ConfigSettings::syntax_font_style[2];
    m_wndExample.m_vbBold[3]      = *ConfigSettings::syntax_font_style[3];
    m_wndExample.m_vbBold[4]      = *ConfigSettings::syntax_font_style[4];

    OnSelChangeElement();

    return true;
}

bool COptionsEditPage::OnSetActive()
{
#if REWRITE_TO_WX_WIDGET
    // copy font settings
    m_wndExample.m_hEditorFont   = COptionsViewPage::m_Text[0].font;
    m_wndExample.m_rgbBackground = COptionsViewPage::m_Text[0].bkgnd;
    m_wndExample.m_rgbText       = COptionsViewPage::m_Text[0].text;

    return CPropertyPage::OnSetActive();
#endif

    return false;
}


void COptionsEditPage::OnChangeTabStep()
{
}

void COptionsEditPage::OnColorSyntax()
{
}

void COptionsEditPage::OnBoldFont()
{
    if (bool* pBold = GetFontStyle())
    {
        *pBold = m_btnBold.GetValue() > 0;
        m_wndExample.Refresh();
        m_bColorChanged = true;
    }
}

bool* COptionsEditPage::GetFontStyle(int nIndex)
{
    if (nIndex >= 0 && nIndex < 5)
        return &m_wndExample.m_vbBold[nIndex];

    return 0;
}

bool* COptionsEditPage::GetFontStyle()
{
    return GetFontStyle(m_wndElement.GetSelection());
}

void COptionsEditPage::OnEditColor()
{
    wxColour* pColor = GetColorElement();

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
    wxColour* p = GetColorElement();

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

wxColour* COptionsEditPage::GetColorElement(int nIndex)
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

wxColour* COptionsEditPage::GetColorElement()
{
    return GetColorElement(m_wndElement.GetSelection());
}

/////////////////////////////////////////////////////////////////////////////
// COptionsAsmPage property page

COptionsAsmPage::COptionsAsmPage()
    : wxPanel()
{
    m_nCaseSensitive = -1;
    m_nAsmInstrWithDot = -1;
    m_bGenerateListing = false;
    m_strListingFile = "";
    m_bGenerateBRKExtraByte = false;
    m_uBrkExtraByte = 0;
}

COptionsAsmPage::~COptionsAsmPage()
{
}

#if REWRITE_TO_WX_WIDGET

void COptionsAsmPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_OPT_ASM_CASE_Y, m_nCaseSensitive);
    DDX_Radio(pDX, IDC_OPT_ASM_INSTR_DOT, m_nAsmInstrWithDot);
    DDX_Check(pDX, IDC_OPT_ASM_GENERATE_LIST, m_bGenerateListing);
    DDX_Text(pDX, IDC_OPT_ASM_FILE_LISTING, m_strListingFile);
    DDX_Check(pDX, IDC_OPT_ASM_GENERATE_BYTE, m_bGenerateBRKExtraByte);
    DDX_HexDec(pDX, IDC_OPT_ASM_EXTRA_BYTE, m_uBrkExtraByte, false);
    DDV_MinMaxUInt(pDX,m_uBrkExtraByte,0,0xFF);
}

BEGIN_MESSAGE_MAP(COptionsAsmPage, CPropertyPage)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_OPT_ASM_CHOOSE_FILE, OnOptAsmChooseFile)
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsAsmPage message handlers


/////////////////////////////////////////////////////////////////////////////
// COptionsDeasmPage property page

COptionsDeasmPage::COptionsDeasmPage() 
    : wxPanel()
{
    m_ShowCode = false;
    m_bSubclassed = false;
    m_bColorChanged = false;
}

COptionsDeasmPage::~COptionsDeasmPage()
{
}

#if REWRITE_TO_WX_WIDGET

void COptionsDeasmPage::DoDataExchange(CDataExchange* pDX)
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

/////////////////////////////////////////////////////////////////////////////
// COptionsDeasmPage message handlers

void COptionsDeasmPage::OnAddrColButton()
{
    m_bColorChanged |= SelectColor(this, &m_rgbAddress, &m_ColorButtonAddress);
}

void COptionsDeasmPage::OnCodeColButton()
{
    m_bColorChanged |= SelectColor(this, &m_rgbCode, &m_ColorButtonCode);
}

bool COptionsDeasmPage::OnSetActive()
{
#if REWRITE_TO_WX_WIDGET
    if (!m_bSubclassed)
    {
        m_ColorButtonAddress.SubclassDlgItem(IDC_OPT_DA_ADDR_COL,this);
        m_ColorButtonAddress.SetColorRef(&m_rgbAddress);
        m_ColorButtonCode.SubclassDlgItem(IDC_OPT_DA_CODE_COL,this);
        m_ColorButtonCode.SetColorRef(&m_rgbCode);

        m_bSubclassed = true;
    }

    return CPropertyPage::OnSetActive();
#endif

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// COptionsMarksPage property page

COptionsMarksPage::COptionsMarksPage()
    : wxPanel()
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

#if REWRITE_TO_WX_WIDGET

void COptionsMarksPage::DoDataExchange(CDataExchange* pDX)
{
    if (!pDX->m_bSaveAndValidate)
    {
        CSpinButtonCtrl *pTab;
        pTab = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_BUS_SPIN);
        ASSERT(pTab != NULL);
        pTab->SetRange(10,24);		// wielko�� szyny adresowej
    }

    CPropertyPage::DoDataExchange(pDX);

    DDX_Radio(pDX, IDC_OPT_6502, m_nProc6502);
    DDX_Radio(pDX, IDC_OPT_HELP_CHM, m_nHelpFile);  //^^help
    DDX_Text(pDX, IDC_OPT_BUS_WIDTH, m_uBusWidth);
    if (m_nProc6502==2)
        DDV_MinMaxUInt(pDX, m_uBusWidth, 10, 24);
    else
        DDV_MinMaxUInt(pDX, m_uBusWidth, 10, 16);

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

/////////////////////////////////////////////////////////////////////////////
// COptionsMarksPage message handlers

void COptionsMarksPage::OnBrkpColButton()
{
    m_colorChanged |= SelectColor(this, &m_rgbBreakpoint, &m_ColorButtonBreakpoint);
}

void COptionsMarksPage::OnErrColButton()
{
    m_colorChanged |= SelectColor(this, &m_rgbError, &m_ColorButtonError);
}

void COptionsMarksPage::OnPtrColButton()
{
    m_colorChanged |= SelectColor(this, &m_rgbPointer, &m_ColorButtonPointer);
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

// Context Sensitive Help starts here
//% Bug fix 1.2.14.1 - convert to HTML help ------------------------------------------

#if REWRITE_TO_WX_WIDGET

bool COptionsSymPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

bool COptionsDeasmPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

bool COptionsEditPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0 )
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

bool COptionsMarksPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

bool COptionsAsmPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 0)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

bool COptions::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 1000 && pHelpInfo->iCtrlId < 2999)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

#endif

void COptionsSymPage::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    //HtmlHelpA(59991, HH_HELP_CONTEXT);
}

void COptions::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    //HtmlHelpA(NULL, HH_DISPLAY_TOPIC);
}

void COptionsAsmPage::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    //HtmlHelpA(59992, HH_HELP_CONTEXT);
}

void COptionsDeasmPage::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    //HtmlHelpA(59993, HH_HELP_CONTEXT);
}

void COptionsEditPage::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    //HtmlHelpA(59994, HH_HELP_CONTEXT);
}

void COptionsMarksPage::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    //HtmlHelpA(59995, HH_HELP_CONTEXT);
}

//--------------------------------------------------------------------------------------

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
