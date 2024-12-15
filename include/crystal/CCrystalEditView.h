////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalEditView.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Interface of the CCrystalEditView class, a part of Crystal Edit - syntax
//	coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef CRYSTAL_EDIT_VIEW_H__
#define CRYSTAL_EDIT_VIEW_H__

#include "cedefs.h"
#include "CCrystalTextView.h"

/////////////////////////////////////////////////////////////////////////////
//	Forward class declarations

class CEditDropTargetImpl;

/////////////////////////////////////////////////////////////////////////////
//	CCrystalEditView view

class CRYSEDIT_CLASS_DECL CCrystalEditView : public CCrystalTextView
{
private:
    bool    m_bOvrMode;
    bool    m_bDropPosVisible;
    wxPoint m_ptSavedCaretPos;
    wxPoint m_ptDropPos;
    bool    m_bSelectionPushed;
    wxPoint m_ptSavedSelStart, m_ptSavedSelEnd;
    bool    m_bAutoIndent;

    // [JRT]
    bool m_bDisableBSAtSOL; // Disable BS At Start Of Line

    bool DeleteCurrentSelection();

protected:
    CEditDropTargetImpl *m_pDropTarget;

#if REWRITE_TO_WX_WIDGET
    virtual DROPEFFECT GetDropEffect();
    virtual void OnDropSource(DROPEFFECT de);
#endif

    void Paste();
    void Cut();
    virtual void ResetView();

// Attributes
public:
    /* constructor */ CCrystalEditView();
    virtual          ~CCrystalEditView();

    bool GetAutoIndent() const;
    void SetAutoIndent(bool bEnable)
    {
        m_bAutoIndent = bEnable;
    }

    // [JRT]
    void SetDisableBSAtSOL(bool bDisableBSAtSOL);
    bool GetDisableBSAtSOL() const;

    bool GetOverwriteMode() const;
    void SetOverwriteMode(bool bOvrMode = TRUE);

    void ShowDropIndicator(const wxPoint &point);
    void HideDropIndicator();

    //bool DoDropText(COleDataObject *pDataObject, const wxPoint &ptClient);
    void DoDragScroll(const wxPoint &point);

    virtual bool QueryEditable();
    virtual void UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext, uint32_t dwFlags, int nLineIndex = -1);

    bool ReplaceSelection(const char *newText);

    virtual void OnEditOperation(int nAction, const std::string &text) override;

    enum LineChange { NOTIF_NO_CHANGES, NOTIF_LINE_MODIFIED, NOTIF_LINE_ERROR };
    virtual LineChange NotifyEnterPressed(wxPoint ptCursor, std::string& strLine);

protected:
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
    afx_msg void OnEditCut();
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnEditDelete();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnEditDeleteBack();
    afx_msg void OnEditUntab();
    afx_msg void OnEditTab();
    afx_msg void OnEditSwitchOvrmode();
    afx_msg void OnUpdateEditSwitchOvrmode(CCmdUI* pCmdUI);
    //afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnEditReplace();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
    afx_msg void OnEditRedo();
    afx_msg void OnEditDeleteToEOL();
    afx_msg void OnUpdateEditDeleteToEol(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorCol(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorOvr(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorRead(CCmdUI* pCmdUI);
    
    bool NotifyEnterPressed();
};

/////////////////////////////////////////////////////////////////////////////

inline bool CCrystalEditView::GetOverwriteMode() const
{
    return m_bOvrMode;
}

inline void CCrystalEditView::SetOverwriteMode(bool bOvrMode /*= true */)
{
    m_bOvrMode = bOvrMode;
}

inline bool CCrystalEditView::GetDisableBSAtSOL() const
{
    return m_bDisableBSAtSOL;
}

inline bool CCrystalEditView::GetAutoIndent() const
{
    return m_bAutoIndent;
}

#endif /* CRYSTAL_EDIT_VIEW_H__ */
