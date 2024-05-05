////////////////////////////////////////////////////////////////////////////
//	File:		CFindTextDlg.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Declaration of the CFindTextDlg dialog, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef FIND_TEXT_DLG_H__
#define FIND_TEXT_DLG_H__

#include "cedefs.h"
#include "editcmd.h"

class CCrystalTextView;

/////////////////////////////////////////////////////////////////////////////
// CFindTextDlg dialog

class CFindTextDlg : public wxDialog
{
private:
    CCrystalTextView *m_buddy;

// Construction
public:
    /* constructor */ CFindTextDlg(CCrystalTextView *pBuddy);

    wxPoint m_ptCurrentPos;

    int m_nDirection;
    bool m_bMatchCase;
    std::string m_text;
    bool m_bWholeWord;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual void OnOK();
    afx_msg void OnChangeEditText();
    virtual bool OnInitDialog();
    virtual void OnCancel();
};

#endif /* FIND_TEXT_DLG_H__ */
