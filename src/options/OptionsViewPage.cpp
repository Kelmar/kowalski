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

#include "OptionsViewPage.h"
#include "ConfigSettings.h"

/*************************************************************************/

COptionsViewPage::TextDef COptionsViewPage::m_Text[6];

/*************************************************************************/

COptionsViewPage::COptionsViewPage(wxBookCtrlBase *parent)
    : wxPanel(parent)
{
    m_bSubclassed = false;
    m_nSelection = 0;

#if REWRITE_TO_WX_WIDGETS
    for (int i= 0; i < sizeof(m_Text) / sizeof(m_Text[0]); i++)
    {
        m_Text[i].font.CreateFontIndirect(CConfigSettings::fonts[i]);
        m_Text[i].text = *CConfigSettings::text_color[i];
        m_Text[i].bkgnd = *CConfigSettings::bkgnd_color[i];
        m_Text[i].changed = 0;
    }
#endif
}

COptionsViewPage::~COptionsViewPage()
{
#if REWRITE_TO_WX_WIDGETS
    for (int i = 0; i < sizeof(m_Text) / sizeof(m_Text[0]); i++)
    {
        m_Text[i].font.DeleteObject();
        m_Text[i].brush.DeleteObject();
    }
#endif
}

/*************************************************************************/

#if REWRITE_TO_WX_WIDGETS

void COptionsViewPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COptionsViewPage)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsViewPage, CPropertyPage)
    //{{AFX_MSG_MAP(COptionsViewPage)
    ON_WM_CTLCOLOR()
    ON_LBN_SELCHANGE(IDC_OPT_VIEW_WND, OnSelchangeViewWnd)
    ON_BN_CLICKED(IDC_OPT_VIEW_TXT_COL, OnViewTxtCol)
    ON_BN_CLICKED(IDC_OPT_VIEW_BKGND_COL, OnViewBkgndCol)
    ON_BN_CLICKED(IDC_OPT_VIEW_FONT_BTN, OnViewFontBtn)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsViewPage message handlers

//#include "6502View.h"

int COptionsViewPage::OnCtlColor(wxDC* pDC, wxWindow* pWnd, UINT nCtlColor)
{
    UNUSED(pDC);
    UNUSED(pWnd);
    UNUSED(nCtlColor);

#if REWRITE_TO_WX_WIDGETS
    if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() == IDC_OPT_VIEW_EXAMPLE)
    {
        m_Text[m_nSelection].brush.DeleteObject();
        m_Text[m_nSelection].brush.CreateSolidBrush(m_Text[m_nSelection].bkgnd);
//		LOGBRUSH lb= {BS_SOLID, m_Text[m_nSelection].bkgnd, 0L};
//		HBRUSH hbr= CreateBrushIndirect(&lb);
        pDC->SelectObject(&m_Text[m_nSelection].font);
        pDC->SetTextColor(m_Text[m_nSelection].text);
        pDC->SetBkColor(m_Text[m_nSelection].bkgnd);
        return m_Text[m_nSelection].brush;
    }

    return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
#endif

    return 0;
}

bool COptionsViewPage::OnSetActive()
{
#if REWRITE_TO_WX_WIDGETS
    CListBox *pList = (CListBox *)GetDlgItem(IDC_OPT_VIEW_WND);

    if (!m_bSubclassed)
    {
        m_rgbTextCol = m_Text[m_nSelection].text;
        m_rgbBkgndCol = m_Text[m_nSelection].bkgnd;
        m_ColorButtonText.SubclassDlgItem(IDC_OPT_VIEW_TXT_COL,this);
        m_ColorButtonText.SetColorRef(&m_rgbTextCol);
        m_ColorButtonBkgnd.SubclassDlgItem(IDC_OPT_VIEW_BKGND_COL,this);
        m_ColorButtonBkgnd.SetColorRef(&m_rgbBkgndCol);
        m_bSubclassed = TRUE;
    }

    if (pList && pList->GetCount() <= 0)
    {
        CString str;
        str.LoadString(IDS_OPT_VIEW_LIST);
        for (TCHAR *start= str.GetBuffer(0); ; )
        {
            TCHAR *lim= _tcschr(start,_T(';'));
            if (lim == NULL)
                break;
            *lim = 0;
            pList->AddString(start);
            start = lim + 1;
        }
        str.ReleaseBuffer();
        pList->SetCurSel(0);
        OnSelchangeViewWnd();
    }

    return CPropertyPage::OnSetActive();
#endif

    return false;
}

void COptionsViewPage::OnSelchangeViewWnd()
{
#if REWRITE_TO_WX_WIDGETS
    CListBox *pList = (CListBox *)GetDlgItem(IDC_OPT_VIEW_WND);

    if (pList == NULL)
        return;

    int sel = pList->GetCurSel();

    if (sel < 0)
        return;

    m_nSelection = sel;
    CString text;
    text.LoadString(IDS_OPT_VIEW_EXAMPLE_1 + sel);
    SetDlgItemText(IDC_OPT_VIEW_EXAMPLE,text);
    m_rgbTextCol = m_Text[m_nSelection].text;
    m_rgbBkgndCol = m_Text[m_nSelection].bkgnd;
    m_ColorButtonText.Invalidate();
    m_ColorButtonBkgnd.Invalidate();
    LOGFONT lf;
    m_Text[m_nSelection].font.GetLogFont(&lf);
    SetDlgItemText(IDC_OPT_VIEW_FONT_NAME,lf.lfFaceName);
#endif
}

void COptionsViewPage::OnViewTxtCol()
{
#if REWRITE_TO_WX_WIDGETS
    CColorDialog dlg(m_rgbTextCol,CC_FULLOPEN);
    if (dlg.DoModal() == IDOK && m_rgbTextCol != dlg.GetColor())
    {
        m_rgbTextCol = dlg.GetColor();
        m_Text[m_nSelection].text = m_rgbTextCol;
        m_Text[m_nSelection].changed = TRUE;
        m_ColorButtonText.Invalidate();
        repaint_example();
    }
#endif
}

void COptionsViewPage::OnViewBkgndCol()
{
#if REWRITE_TO_WX_WIDGETS
    CColorDialog dlg(m_rgbBkgndCol,CC_FULLOPEN);
    if (dlg.DoModal() == IDOK && m_rgbBkgndCol != dlg.GetColor())
    {
        m_rgbBkgndCol = dlg.GetColor();
        m_Text[m_nSelection].bkgnd = m_rgbBkgndCol;
        m_Text[m_nSelection].changed |= 1;		// zmieniony kolor
        m_ColorButtonText.Invalidate();
        repaint_example();
    }
#endif
}

void COptionsViewPage::repaint_example()
{
#if REWRITE_TO_WX_WIDGETS
    HWND hWnd;
    GetDlgItem(IDC_OPT_VIEW_EXAMPLE, &hWnd);
    ::InvalidateRect(hWnd, NULL, TRUE);
#endif
}

void COptionsViewPage::OnViewFontBtn()
{
#if REWRITE_TO_WX_WIDGETS

    LOGFONT lf;
    m_Text[m_nSelection].font.GetLogFont(&lf);
    CFontDialog fnt(&lf, CF_SCREENFONTS | CF_FIXEDPITCHONLY |
                    CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST | CF_SCRIPTSONLY);

    if (fnt.DoModal() == IDOK)
    {
        m_Text[m_nSelection].font.DeleteObject();
        m_Text[m_nSelection].font.CreateFontIndirect(&fnt.m_lf);
        SetDlgItemText(IDC_OPT_VIEW_FONT_NAME,fnt.m_lf.lfFaceName);
        m_Text[m_nSelection].changed |= 2;		// zmieniony font
        repaint_example();
    }
#endif
}
