////////////////////////////////////////////////////////////////////////////
//	File:		CEditReplaceDlg.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Declaration of the CEditReplaceDlg dialog, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef EDIT_REPLACE_DLG_H__
#define EDIT_REPLACE_DLG_H__

#include "cedefs.h"
#include "editcmd.h"

class CCrystalEditView;

/////////////////////////////////////////////////////////////////////////////
// CEditReplaceDlg dialog

class CEditReplaceDlg : public wxDialog
{
private:
    CCrystalEditView *m_buddy;
    bool m_bFound;
    wxPoint m_ptFoundAt;
    bool DoHighlightText();

public:
    /* constructor */ CEditReplaceDlg(CCrystalEditView *buddy);

    bool m_bEnableScopeSelection;
    wxPoint m_ptCurrentPos;
    wxPoint m_ptBlockBegin, m_ptBlockEnd;

    enum { IDD = IDD_EDIT_REPLACE };
    bool m_bMatchCase;
    bool m_bWholeWord;
    std::string m_text;
    std::string m_newText;
    int m_nScope;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    afx_msg void OnChangeEditText();
    virtual void OnCancel();
    virtual bool OnInitDialog();
    afx_msg void OnEditReplace();
    afx_msg void OnEditReplaceAll();
    afx_msg void OnEditSkip();

};

#endif /* EDIT_REPLACE_DLG_H__ */
