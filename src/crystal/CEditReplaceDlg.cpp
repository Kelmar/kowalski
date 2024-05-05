////////////////////////////////////////////////////////////////////////////
//	File:		CEditReplaceDlg.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CEditReplaceDlg dialog, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CEditReplaceDlg.h"
#include "CCrystalEditView.h"

/////////////////////////////////////////////////////////////////////////////
// CEditReplaceDlg dialog


CEditReplaceDlg::CEditReplaceDlg(CCrystalEditView *buddy)
    : wxDialog()
    , m_buddy(buddy)
{
    ASSERT(buddy != NULL);

    m_bMatchCase = FALSE;
    m_bWholeWord = FALSE;
    m_text = "";
    m_newText = "";
    m_nScope = -1;
    m_bEnableScopeSelection = TRUE;
}

#if REWRITE_TO_WX_WIDGET

void CEditReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CEditReplaceDlg)
    DDX_Check(pDX, IDC_EDIT_MATCH_CASE, m_bMatchCase);
    DDX_Check(pDX, IDC_EDIT_WHOLE_WORD, m_bWholeWord);
    DDX_Text(pDX, IDC_EDIT_TEXT, m_sText);
    DDX_Text(pDX, IDC_EDIT_REPLACE_WITH, m_sNewText);
    DDX_Radio(pDX, IDC_EDIT_SCOPE_SELECTION, m_nScope);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditReplaceDlg, CDialog)
    //{{AFX_MSG_MAP(CEditReplaceDlg)
    ON_EN_CHANGE(IDC_EDIT_TEXT, OnChangeEditText)
    ON_BN_CLICKED(IDC_EDIT_REPLACE, OnEditReplace)
    ON_BN_CLICKED(IDC_EDIT_REPLACE_ALL, OnEditReplaceAll)
    ON_BN_CLICKED(IDC_EDIT_SKIP, OnEditSkip)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CEditReplaceDlg message handlers

void CEditReplaceDlg::OnChangeEditText()
{
#if REWRITE_TO_WX_WIDGET
    std::string text;
    GetDlgItem(IDC_EDIT_TEXT)->GetWindowText(text);
    GetDlgItem(IDC_EDIT_SKIP)->EnableWindow(!text.empty());
#endif
}

void CEditReplaceDlg::OnCancel()
{
#if REWRITE_TO_WX_WIDGET
    VERIFY(UpdateData());

    CDialog::OnCancel();
#endif
}

bool CEditReplaceDlg::OnInitDialog()
{
#if REWRITE_TO_WX_WIDGET
    CDialog::OnInitDialog();

    GetDlgItem(IDC_EDIT_SKIP)->EnableWindow(m_sText != _T(""));
    GetDlgItem(IDC_EDIT_SCOPE_SELECTION)->EnableWindow(m_bEnableScopeSelection);
    m_bFound = FALSE;

    return TRUE;
#endif
    return false;
}

bool CEditReplaceDlg::DoHighlightText()
{
#if REWRITE_TO_WX_WIDGET
    ASSERT(m_buddy != NULL);
    uint32_t searchFlags = 0;

    if (m_bMatchCase)
        searchFlags |= FIND_MATCH_CASE;

    if (m_bWholeWord)
        searchFlags |= FIND_WHOLE_WORD;

    bool bFound;
    if (m_nScope == 0)
    {
        //	Searching selection only
        bFound = m_pBuddy->FindTextInBlock(m_sText, m_ptFoundAt, m_ptBlockBegin, m_ptBlockEnd,
                                           dwSearchFlags, FALSE, &m_ptFoundAt);
    }
    else
    {
        //	Searching whole text
        bFound = m_pBuddy->FindText(m_sText, m_ptFoundAt, dwSearchFlags, FALSE, &m_ptFoundAt);
    }

    if (!bFound)
    {
        std::string prompt;
        prompt.Printf(IDS_EDIT_TEXT_NOT_FOUND, m_sText);
        AfxMessageBox(prompt);
        m_ptCurrentPos = m_nScope == 0 ? m_ptBlockBegin : wxPoint(0, 0);
        return false;
    }

    m_buddy->HighlightText(m_ptFoundAt, lstrlen(m_sText));
    return true;
#endif

    return false;
}

void CEditReplaceDlg::OnEditSkip()
{
#if REWRITE_TO_WX_WIDGET
    if (!UpdateData())
        return;

    if (!m_bFound)
    {
        m_ptFoundAt = m_ptCurrentPos;
        m_bFound = DoHighlightText();
        return;
    }

    m_ptFoundAt.x += 1;
    m_bFound = DoHighlightText();
#endif
}

void CEditReplaceDlg::OnEditReplace()
{
#if REWRITE_TO_WX_WIDGET
    if (!UpdateData())
        return;

    if (!m_bFound)
    {
        m_ptFoundAt = m_ptCurrentPos;
        m_bFound = DoHighlightText();
        return;
    }

    //	We have highlighted text
    VERIFY(m_buddy->ReplaceSelection(m_newText));

    //	Manually recalculate points
    if (m_bEnableScopeSelection)
    {
        if (m_ptBlockBegin.y == m_ptFoundAt.y && m_ptBlockBegin.x > m_ptFoundAt.x)
        {
            m_ptBlockBegin.x -= m_text.size();lstrlen(m_sText);
            m_ptBlockBegin.x += m_newText.size();
        }
        if (m_ptBlockEnd.y == m_ptFoundAt.y && m_ptBlockEnd.x > m_ptFoundAt.x)
        {
            m_ptBlockEnd.x -= m_text.size();
            m_ptBlockEnd.x += m_newText.size();
        }
    }

    m_ptFoundAt.x += m_newText.size();
    m_bFound = DoHighlightText();
#endif
}

void CEditReplaceDlg::OnEditReplaceAll()
{
#if REWRITE_TO_WX_WIDGET
    if (!UpdateData())
        return;

    if (!m_bFound)
    {
        m_ptFoundAt = m_ptCurrentPos;
        m_bFound = DoHighlightText();
    }

    while (m_bFound)
    {
        //	We have highlighted text
        VERIFY(m_buddy->ReplaceSelection(m_newText));

        //	Manually recalculate points
        if (m_bEnableScopeSelection)
        {
            if (m_ptBlockBegin.y == m_ptFoundAt.y && m_ptBlockBegin.x > m_ptFoundAt.x)
            {
                m_ptBlockBegin.x -= m_text.size();
                m_ptBlockBegin.x += m_newText.size();
            }

            if (m_ptBlockEnd.y == m_ptFoundAt.y && m_ptBlockEnd.x > m_ptFoundAt.x)
            {
                m_ptBlockEnd.x -= m_text.size();
                m_ptBlockEnd.x += m_newText.size();
            }
        }

        m_ptFoundAt.x += m_newText.size();
        m_bFound = DoHighlightText();
    }
#endif
}
