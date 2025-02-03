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

#include "stdafx.h"
#include "resource.h"
#include "Options.h"
#include "ConfigSettings.h"

//static const char *HELP_FILE_6502= "6502.txt";

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord= true);

static HH_POPUP hPop;

/////////////////////////////////////////////////////////////////////////////
// COptions

IMPLEMENT_DYNAMIC(COptions, CPropertySheet)

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

COptions::COptions(CWnd* pParentWnd, UINT iSelectPage)
  : CPropertySheet(ID, pParentWnd, iSelectPage)
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_HASHELP | PSH_USECALLBACK;
	m_psh.pfnCallback = &PropSheetProc;
	AddPage(&m_SymPage);
	AddPage(&m_AsmPage);
	AddPage(&m_EditPage);
	AddPage(&m_DeasmPage);
	AddPage(&m_MarksPage);
	AddPage(&m_ViewPage);
	m_nLastActivePageIndex = iSelectPage;


        // set up HH_POPUP defaults for all context sensitive help
	// Initialize structure to NULLs    
  	memset(&hPop, 0, sizeof(hPop)); 
  	// Set size of structure
  	hPop.cbStruct         = sizeof(hPop);        
  	hPop.clrBackground    = RGB(255, 255, 208);    // Yellow background color
  	hPop.clrForeground    = -1; // Font color             //  black font	
  	hPop.rcMargins.top   = -1;             
  	hPop.rcMargins.left   = -1;             
  	hPop.rcMargins.bottom = -1;
  	hPop.rcMargins.right  = -1;
  	hPop.pszFont          = NULL; 		           // Font

}

COptions::~COptions()
{
}


int CALLBACK COptions::PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
	if (uMsg == PSCB_INITIALIZED && ::IsWindow(hwndDlg))
		CWnd::FromHandle(hwndDlg)->ModifyStyleEx(0, WS_EX_CONTEXTHELP);  // w³¹czenie pomocy kontekstowej

	return 0;
}


int COptions::GetLastActivePage()
{
	return m_nLastActivePageIndex;
}

BEGIN_MESSAGE_MAP(COptions, CPropertySheet)
	
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

BOOL COptions::OnCommand(WPARAM wParam, LPARAM lParam)
{
	m_nLastActivePageIndex = GetActiveIndex();
	return CPropertySheet::OnCommand(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// COptionsSymPage property page

IMPLEMENT_DYNCREATE(COptionsSymPage, CPropertyPage)

COptionsSymPage::COptionsSymPage() : CPropertyPage(COptionsSymPage::IDD)
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
/*
static void AFX_CDECL DDX_Hex(CDataExchange* pDX, int nIDC, unsigned int &hex)
{
  HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
  TCHAR szT[32];
  if (pDX->m_bSaveAndValidate)
  {
    ::GetWindowText(hWndCtrl, szT, sizeof(szT)/sizeof(szT[0]));
    if (sscanf(szT, _T("%X"),&hex) <= 0)
    {
      AfxMessageBox(IDS_MSG_HEX_STR);
      pDX->Fail();        // throws exception
    }
  }
  else
  {
    wsprintf(szT,_T("%04X"),hex);
    ::SetWindowText(hWndCtrl, szT);
  }
}
*/

void COptionsSymPage::DoDataExchange(CDataExchange* pDX)
{
	if (!pDX->m_bSaveAndValidate)
	{
		CSpinButtonCtrl *pCols;
		pCols = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_SYM_W_SPIN);
		ASSERT(pCols != NULL);
		pCols->SetRange(1, 255);		// iloœæ kolumn terminala

		CSpinButtonCtrl *pRows;
		pRows = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_SYM_H_SPIN);
		ASSERT(pRows != NULL);
		pRows->SetRange(1, 255);		// iloœæ wierszy terminala
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
	
	if (theApp.m_global.GetProcType() == ProcessorType::WDC65816)
		DDV_MinMaxUInt(pDX, m_nProtFromAddr, 0, 0xffffff);
	else
		DDV_MinMaxUInt(pDX, m_nProtFromAddr, 0, 0xffff);

	DDX_HexDec(pDX, IDC_OPT_SYM_PROT_TO, m_nProtToAddr);
	if (theApp.m_global.GetProcType() == ProcessorType::WDC65816)
		DDV_MinMaxUInt(pDX, m_nProtToAddr, 0, 0xffffff);
	else
		DDV_MinMaxUInt(pDX, m_nProtToAddr, 0, 0xffff);
}


BEGIN_MESSAGE_MAP(COptionsSymPage, CPropertyPage)
	
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSymPage message handlers



/////////////////////////////////////////////////////////////////////////////
// COptionsEditPage property page

IMPLEMENT_DYNCREATE(COptionsEditPage, CPropertyPage)

COptionsEditPage::COptionsEditPage() : CPropertyPage(COptionsEditPage::IDD)
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

/////////////////////////////////////////////////////////////////////////////
// COptionsEditPage message handlers

BOOL COptionsEditPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// copy color settings
	m_wndExample.m_rgbInstruction	= *CConfigSettings::color_syntax[0];
	m_wndExample.m_rgbDirective		= *CConfigSettings::color_syntax[1];
	m_wndExample.m_rgbComment		= *CConfigSettings::color_syntax[2];
	m_wndExample.m_rgbNumber		= *CConfigSettings::color_syntax[3];
	m_wndExample.m_rgbString		= *CConfigSettings::color_syntax[4];
	m_wndExample.m_rgbSelection		= *CConfigSettings::color_syntax[5];

	m_wndExample.m_vbBold[0]		= *CConfigSettings::syntax_font_style[0];
	m_wndExample.m_vbBold[1]		= *CConfigSettings::syntax_font_style[1];
	m_wndExample.m_vbBold[2]		= *CConfigSettings::syntax_font_style[2];
	m_wndExample.m_vbBold[3]		= *CConfigSettings::syntax_font_style[3];
	m_wndExample.m_vbBold[4]		= *CConfigSettings::syntax_font_style[4];

	OnSelChangeElement();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL COptionsEditPage::OnSetActive()
{
	// copy font settings
	m_wndExample.m_hEditorFont		= COptionsViewPage::m_Text[0].font;
	m_wndExample.m_rgbBackground	= COptionsViewPage::m_Text[0].bkgnd;
	m_wndExample.m_rgbText			= COptionsViewPage::m_Text[0].text;

	return CPropertyPage::OnSetActive();
}


void COptionsEditPage::OnChangeTabStep()
{
}

void COptionsEditPage::OnColorSyntax()
{
}

void COptionsEditPage::OnBoldFont()
{
	if (bool* pBold= GetFontStyle())
	{
		*pBold = m_btnBold.GetCheck() > 0;
		m_wndExample.Invalidate();
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
	return GetFontStyle(m_wndElement.GetCurSel());
}


void COptionsEditPage::OnEditColor()
{
	if (COLORREF* pColor= COptionsEditPage::GetColorElement())
	{
		CColorDialog dlg(*pColor, CC_FULLOPEN);

		if (dlg.DoModal() == IDOK && *pColor != dlg.GetColor())
		{
			*pColor = dlg.GetColor();
			m_btnColor.Invalidate();
			m_wndExample.Invalidate();
			m_bColorChanged = true;
		}
	}
}


void COptionsEditPage::OnSelChangeElement()
{
	if (COLORREF* p= GetColorElement())
		m_btnColor.SetColorRef(p);
	if (bool* pBold= GetFontStyle())
	{
		m_btnBold.EnableWindow();
		m_btnBold.SetCheck(*pBold ? 1 : 0);
	}
	else
	{
		m_btnBold.EnableWindow(false);
		m_btnBold.SetCheck(0);
	}
}


COLORREF* COptionsEditPage::GetColorElement(int nIndex)
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

COLORREF* COptionsEditPage::GetColorElement()
{
	return GetColorElement(m_wndElement.GetCurSel());
}


/////////////////////////////////////////////////////////////////////////////
// COptionsAsmPage property page

IMPLEMENT_DYNCREATE(COptionsAsmPage, CPropertyPage)

COptionsAsmPage::COptionsAsmPage() : CPropertyPage(COptionsAsmPage::IDD)
{
  
  m_nCaseSensitive = -1;
  m_nSwapBin = -1;
  m_bGenerateListing = FALSE;
  m_strListingFile = _T("");
  m_bGenerateBRKExtraByte = FALSE;
  m_uBrkExtraByte = 0;
  
}

COptionsAsmPage::~COptionsAsmPage()
{
}

void COptionsAsmPage::DoDataExchange(CDataExchange* pDX)
{ 
  CPropertyPage::DoDataExchange(pDX);
  
  DDX_Radio(pDX, IDC_OPT_ASM_CASE_Y, m_nCaseSensitive);
  DDX_Radio(pDX, IDC_OPT_ASM_NO_SWAPBIN, m_nSwapBin);
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

/////////////////////////////////////////////////////////////////////////////
// COptionsAsmPage message handlers


/////////////////////////////////////////////////////////////////////////////
// COptionsDeasmPage property page

IMPLEMENT_DYNCREATE(COptionsDeasmPage, CPropertyPage)

COptionsDeasmPage::COptionsDeasmPage() : CPropertyPage(COptionsDeasmPage::IDD)
{
  m_ShowCode = FALSE;
  m_bSubclassed = FALSE;
  m_bColorChanged = FALSE;
}

COptionsDeasmPage::~COptionsDeasmPage()
{
}

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

/////////////////////////////////////////////////////////////////////////////
// COptionsDeasmPage message handlers

void COptionsDeasmPage::OnAddrColButton()
{
  CColorDialog dlg(m_rgbAddress,CC_FULLOPEN);
  if (dlg.DoModal() == IDOK && m_rgbAddress != dlg.GetColor())
  {
    m_rgbAddress = dlg.GetColor();
    m_bColorChanged = TRUE;
    m_ColorButtonAddress.InvalidateRect(NULL);
  }
}


void COptionsDeasmPage::OnCodeColButton()
{
  CColorDialog dlg(m_rgbCode,CC_FULLOPEN);
  if (dlg.DoModal() == IDOK && m_rgbCode != dlg.GetColor())
  {
    m_rgbCode = dlg.GetColor();
    m_bColorChanged = TRUE;
    m_ColorButtonCode.InvalidateRect(NULL);
  }
}


BOOL COptionsDeasmPage::OnSetActive()
{
  if (!m_bSubclassed)
  {
    m_ColorButtonAddress.SubclassDlgItem(IDC_OPT_DA_ADDR_COL,this);
    m_ColorButtonAddress.SetColorRef(&m_rgbAddress);
    m_ColorButtonCode.SubclassDlgItem(IDC_OPT_DA_CODE_COL,this);
    m_ColorButtonCode.SetColorRef(&m_rgbCode);
    m_bSubclassed = TRUE;
  }
  return CPropertyPage::OnSetActive();
}

/////////////////////////////////////////////////////////////////////////////
// COptionsMarksPage property page

IMPLEMENT_DYNCREATE(COptionsMarksPage, CPropertyPage)

COptionsMarksPage::COptionsMarksPage() : CPropertyPage(COptionsMarksPage::IDD)
{
  m_nProc6502 = -1;
  m_uBusWidth = 16;
  m_nHelpFile = 0;  //^^help
  m_bSubclassed = FALSE;
  m_bColorChanged = FALSE;
  m_bFontChanged = FALSE;
}

COptionsMarksPage::~COptionsMarksPage()
{
}

void COptionsMarksPage::DoDataExchange(CDataExchange* pDX)
{
  if (!pDX->m_bSaveAndValidate)
  {
//    CSpinButtonCtrl *pTab;
//    pTab = (CSpinButtonCtrl *) GetDlgItem(IDC_OPT_BUS_SPIN);
//    ASSERT(pTab != NULL);
//	pTab->SetRange(10,24);		// wielkoœæ szyny adresowej
  }

  CPropertyPage::DoDataExchange(pDX);
  
  DDX_Radio(pDX, IDC_OPT_6502, m_nProc6502);
  DDX_Radio(pDX, IDC_OPT_HELP_CHM, m_nHelpFile);  //^^help
//  DDX_Text(pDX, IDC_OPT_BUS_WIDTH, m_uBusWidth);
//  if (m_nProc6502==2)
//	DDV_MinMaxUInt(pDX, m_uBusWidth, 10, 24);
//  else
//	DDV_MinMaxUInt(pDX, m_uBusWidth, 10, 16);
  
}


BEGIN_MESSAGE_MAP(COptionsMarksPage, CPropertyPage)
  
  ON_BN_CLICKED(IDC_OPT_MARK_BRKP_COL, OnBrkpColButton)
  ON_BN_CLICKED(IDC_OPT_MARK_ERR_COL, OnErrColButton)
  ON_BN_CLICKED(IDC_OPT_MARK_PTR_COL, OnPtrColButton)
  ON_WM_HELPINFO()
  ON_WM_CONTEXTMENU()
  ON_BN_CLICKED(IDC_OPT_FONT_BTN, OnOptFontBtn)
  
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsMarksPage message handlers

void COptionsMarksPage::OnBrkpColButton()
{
  CColorDialog dlg(m_rgbBreakpoint,CC_FULLOPEN);
  if (dlg.DoModal() == IDOK && m_rgbBreakpoint != dlg.GetColor())
  {
    m_rgbBreakpoint = dlg.GetColor();
    m_bColorChanged = TRUE;
    m_ColorButtonBreakpoint.InvalidateRect(NULL);
  }
}

void COptionsMarksPage::OnErrColButton()
{
  CColorDialog dlg(m_rgbError,CC_FULLOPEN);
  if (dlg.DoModal() == IDOK && m_rgbError != dlg.GetColor())
  {
    m_rgbError = dlg.GetColor();
    m_bColorChanged = TRUE;
    m_ColorButtonBreakpoint.InvalidateRect(NULL);
  }
}

void COptionsMarksPage::OnPtrColButton()
{
  CColorDialog dlg(m_rgbPointer,CC_FULLOPEN);
  if (dlg.DoModal() == IDOK && m_rgbPointer != dlg.GetColor())
  {
    m_rgbPointer = dlg.GetColor();
    m_bColorChanged = TRUE;
    m_ColorButtonBreakpoint.InvalidateRect(NULL);
  }
}

BOOL COptionsMarksPage::OnSetActive()
{
  if (!m_bSubclassed)
  {
    m_ColorButtonPointer.SubclassDlgItem(IDC_OPT_MARK_PTR_COL,this);
    m_ColorButtonPointer.SetColorRef(&m_rgbPointer);
    m_ColorButtonBreakpoint.SubclassDlgItem(IDC_OPT_MARK_BRKP_COL,this);
    m_ColorButtonBreakpoint.SetColorRef(&m_rgbBreakpoint);
    m_ColorButtonError.SubclassDlgItem(IDC_OPT_MARK_ERR_COL,this);
    m_ColorButtonError.SetColorRef(&m_rgbError);
    m_bSubclassed = TRUE;
  }
  return CPropertyPage::OnSetActive();
}

void COptionsMarksPage::OnOptFontBtn()
{
  CFontDialog fnt(&m_LogFont,CF_SCREENFONTS | CF_FIXEDPITCHONLY |
    CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST | CF_SCRIPTSONLY);

  if (fnt.DoModal() == IDOK)
  {
    m_bFontChanged = TRUE;
    m_LogFont = fnt.m_lf;
    SetDlgItemText(IDC_OPT_FONT_NAME,m_LogFont.lfFaceName);
  }
}

// Context Sensitive Help starts here
//% Bug fix 1.2.14.1 - convert to HTML help ------------------------------------------
BOOL COptionsSymPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if (pHelpInfo->iCtrlId > 0 )
	{
		hPop.pt               = pHelpInfo->MousePos;    
		hPop.idString         = pHelpInfo->iCtrlId;
		HtmlHelpA((DWORD)(void*)&hPop, HH_DISPLAY_TEXT_POPUP) ;
	}
	return TRUE;
}

BOOL COptionsDeasmPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iCtrlId > 0 )
	{
		hPop.pt               = pHelpInfo->MousePos;    
		hPop.idString         = pHelpInfo->iCtrlId;
		HtmlHelpA((DWORD)(void*)&hPop, HH_DISPLAY_TEXT_POPUP) ;
	}
	return TRUE;
}

BOOL COptionsEditPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iCtrlId > 0 )
	{
		hPop.pt               = pHelpInfo->MousePos;    
		hPop.idString         = pHelpInfo->iCtrlId;
		HtmlHelpA((DWORD)(void*)&hPop, HH_DISPLAY_TEXT_POPUP) ;
	}
	return TRUE;
}

BOOL COptionsMarksPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iCtrlId > 0 )
	{
		hPop.pt               = pHelpInfo->MousePos;    
		hPop.idString         = pHelpInfo->iCtrlId;
		HtmlHelpA((DWORD)(void*)&hPop, HH_DISPLAY_TEXT_POPUP) ;
	}
	return TRUE;
}

BOOL COptionsAsmPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iCtrlId > 0 )
	{
		hPop.pt               = pHelpInfo->MousePos;    
		hPop.idString         = pHelpInfo->iCtrlId;
		HtmlHelpA((DWORD)(void*)&hPop, HH_DISPLAY_TEXT_POPUP) ;
	}

	return TRUE;
}

BOOL COptions::OnHelpInfo(HELPINFO* pHelpInfo)
{	
	if ( pHelpInfo->iCtrlId > 1000 && pHelpInfo->iCtrlId < 2999 )
	{
		hPop.pt               = pHelpInfo->MousePos;    
		hPop.idString         = pHelpInfo->iCtrlId;
		HtmlHelpA((DWORD)(void*)&hPop, HH_DISPLAY_TEXT_POPUP) ;
	}
    return TRUE;
}

void COptionsSymPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HtmlHelpA(59991, HH_HELP_CONTEXT);
}

void COptions::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HtmlHelpA(NULL, HH_DISPLAY_TOPIC);
}

void COptionsAsmPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HtmlHelpA(59992, HH_HELP_CONTEXT);

}

void COptionsDeasmPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HtmlHelpA(59993, HH_HELP_CONTEXT);
}

void COptionsEditPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HtmlHelpA(59994, HH_HELP_CONTEXT);
}

void COptionsMarksPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HtmlHelpA(59995, HH_HELP_CONTEXT);
}
//--------------------------------------------------------------------------------------


void COptionsAsmPage::OnOptAsmChooseFile()
{
  CString filter;
  filter.LoadString(IDS_ASM_OPT_LIST_FILES);
  CFileDialog dlg(TRUE,_T("lst"),NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,filter);
  if (dlg.DoModal() != IDOK)
    return;
  SetDlgItemText(IDC_OPT_ASM_FILE_LISTING,dlg.GetPathName());
}
