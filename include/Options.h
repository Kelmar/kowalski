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

#include "ColorButton.h"
#include "OptionsViewPage.h"
#include "SyntaxExample.h"


/////////////////////////////////////////////////////////////////////////////
// COptionsSymPage dialog

class COptionsSymPage : public wxPanel
{
// Construction
public:
    /* constructor */ COptionsSymPage();
    virtual ~COptionsSymPage();

    uint32_t m_nIOAddress;
    bool m_bIOEnable;
    int m_nFinish;
    int m_nWndWidth;
    int m_nWndHeight;
    bool m_bProtectMemory;
    uint32_t m_nProtFromAddr;
    uint32_t m_nProtToAddr;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
};

/////////////////////////////////////////////////////////////////////////////
// COptionsEditPage dialog

class COptionsEditPage : public wxPanel
{
public:
    /* constructor */ COptionsEditPage();
    virtual ~COptionsEditPage();

    enum { IDD = IDD_PROPPAGE_EDITOR };
    wxCheckBox m_btnBold;
    CColorButton m_btnColor;
    wxComboBox m_wndElement;
    CSyntaxExample m_wndExample;
    bool m_bAutoIndent;
    int	m_nTabStep;
    bool m_bAutoSyntax;
    bool m_bAutoUppercase;
    bool m_bFileNew;
    int  m_nElement;
    bool m_bColorChanged;

    wxColour* GetColorElement(int nIndex);
    bool* GetFontStyle(int nIndex);

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    virtual bool OnInitDialog();
    afx_msg void OnChangeTabStep();
    afx_msg void OnColorSyntax();
    afx_msg void OnSelChangeElement();
    afx_msg void OnEditColor();
    afx_msg void OnBoldFont();

    wxColour* GetColorElement();
    bool* GetFontStyle();
};

/////////////////////////////////////////////////////////////////////////////
// COptionsAsmPage dialog

class COptionsAsmPage : public wxPanel
{
public:
    /* constructor */ COptionsAsmPage();
    virtual ~COptionsAsmPage();

    int m_nCaseSensitive;
    bool m_nSwapBin;
    bool m_bGenerateListing;
    std::string m_strListingFile;
    bool m_bGenerateBRKExtraByte;
    UINT m_uBrkExtraByte;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    afx_msg void OnOptAsmChooseFile();
};

/////////////////////////////////////////////////////////////////////////////
// COptionsDeasmPage dialog

class COptionsDeasmPage : public wxPanel
{
private:
    CColorButton m_ColorButtonAddress;
    CColorButton m_ColorButtonCode;
//  CColorButton m_ColorButtonInstr;
    bool m_bSubclassed;

public:
    wxColour m_rgbAddress;
    wxColour m_rgbCode;
    //wxColour m_rgbInstr;
    bool m_bColorChanged;

    // Construction
public:
    /* constructor */ COptionsDeasmPage();
    virtual ~COptionsDeasmPage();

    bool m_ShowCode;

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    afx_msg void OnAddrColButton();
    afx_msg void OnCodeColButton();
    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
};


/////////////////////////////////////////////////////////////////////////////
// COptionsMarksPage dialog

class COptionsMarksPage : public wxPanel
{
private:
    CColorButton m_ColorButtonPointer;
    CColorButton m_ColorButtonBreakpoint;
    CColorButton m_ColorButtonError;

    bool m_bSubclassed;

public:
    wxColour m_rgbPointer;
    wxColour m_rgbBreakpoint;
    wxColour m_rgbError;
    bool m_colorChanged;
    bool m_bFontChanged;
    wxFontInfo m_LogFont;

    // Construction
public:
    /* constructor */ COptionsMarksPage();
    virtual ~COptionsMarksPage();

    int m_nProc6502;
    UINT m_uBusWidth;
    int m_nHelpFile; //^^help

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    afx_msg void OnBrkpColButton();
    afx_msg void OnErrColButton();
    afx_msg void OnPtrColButton();
    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    afx_msg void OnOptFontBtn();
};

/////////////////////////////////////////////////////////////////////////////
// COptions

class COptions : public wxNotebook
{
    int m_nLastActivePageIndex;

    //static int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);

public:
    /* constructor */ COptions(UINT iSelectPage = 0);
    virtual          ~COptions();

    // Attributes
public:
    static uint32_t m_arrIds[];
    COptionsAsmPage m_AsmPage;
    COptionsEditPage m_EditPage;
    COptionsSymPage m_SymPage;
    COptionsDeasmPage m_DeasmPage;
    COptionsMarksPage m_MarksPage;
    COptionsViewPage m_ViewPage;

    // Operations
public:
    int GetLastActivePage();

protected:
    virtual bool OnCommand(WPARAM wParam, LPARAM lParam);

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
};

/////////////////////////////////////////////////////////////////////////////
