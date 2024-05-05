////////////////////////////////////////////////////////////////////////////
//	File:		CFindTextDlg.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CFindTextDlg dialog, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CFindTextDlg.h"
#include "CCrystalTextView.h"

/////////////////////////////////////////////////////////////////////////////
// CFindTextDlg dialog

CFindTextDlg::CFindTextDlg(CCrystalTextView *buddy)
    : wxDialog()
    , m_buddy(buddy)
{
    ASSERT(m_buddy != NULL);

    m_nDirection = 1;
    m_bMatchCase = FALSE;
    m_bWholeWord = FALSE;
    m_text = "";
    m_ptCurrentPos = wxPoint(0, 0);
}

#if REWRITE_TO_WX_WIDGET
void CFindTextDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFindTextDlg)
    DDX_Radio(pDX, IDC_EDIT_DIRECTION_UP, m_nDirection);
    DDX_Check(pDX, IDC_EDIT_MATCH_CASE, m_bMatchCase);
    DDX_Text(pDX, IDC_EDIT_TEXT, m_sText);
    DDX_Check(pDX, IDC_EDIT_WHOLE_WORD, m_bWholeWord);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFindTextDlg, CDialog)
    //{{AFX_MSG_MAP(CFindTextDlg)
    ON_EN_CHANGE(IDC_EDIT_TEXT, OnChangeEditText)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CFindTextDlg message handlers

void CFindTextDlg::OnOK()
{
#if REWRITE_TO_WX_WIDGET
    ASSERT(m_buddy != NULL);

    if (UpdateData())
    {
        uint32_t searchFlags = 0;

        if (m_bMatchCase)
            searchFlags |= FIND_MATCH_CASE;

        if (m_bWholeWord)
            searchFlags |= FIND_WHOLE_WORD;

        if (m_nDirection == 0)
            searchFlags |= FIND_DIRECTION_UP;

        wxPoint ptTextPos;
        if (! m_pBuddy->FindText(m_sText, m_ptCurrentPos, searchFlags, TRUE, &ptTextPos))
        {
            wxString prompt;
            prompt.Printf(IDS_EDIT_TEXT_NOT_FOUND, m_text);
            AfxMessageBox(prompt);
            m_ptCurrentPos = CPoint(0, 0);
            return;
        }

        m_buddy->HighlightText(ptTextPos, lstrlen(m_sText));

        CDialog::OnOK();
    }
#endif
}

void CFindTextDlg::OnChangeEditText()
{
#if REWRITE_TO_WX_WIDGET
    std::string text;
    GetDlgItem(IDC_EDIT_TEXT)->GetWindowText(text);
    GetDlgItem(IDOK)->EnableWindow(!text.empty());
#endif
}

bool CFindTextDlg::OnInitDialog()
{
#if REWRITE_TO_WX_WIDGET
    CDialog::OnInitDialog();

    GetDlgItem(IDOK)->EnableWindow(m_sText != "");

    return true;
#endif
    return false;
}

void CFindTextDlg::OnCancel()
{
#if REWRITE_TO_WX_WIDGET
    VERIFY(UpdateData());

    CDialog::OnCancel();
#endif
}
