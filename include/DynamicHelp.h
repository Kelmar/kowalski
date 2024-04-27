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

#ifndef DYNAMIC_HELP__
#define DYNAMIC_HELP__

struct TEXTBLOCK;

/**
 * @brief CDynamicHelp window
 * 
 * Provides the quick help tips on the right side of the window.
 */
class CDynamicHelp : public wxPanel //CControlBar
{
private:
    //typedef CControlBar inherited;

    //wxToolTip m_ToolTip;
    //CToolBarCtrl m_wndClose;
    wxRichTextCtrl m_wndHelp;    // What actually displays the formatted help.

    int m_nHeaderHeight;

    void SizeToolBar(int length, bool vert = false);

public:
    /* constructor */ CDynamicHelp();
    virtual          ~CDynamicHelp();

public:
    void SetContextHelp(const char *text, const char *header = 0);

    void DisplayHelp(const std::string &line, int wordStart, int wordEnd);

public:
    bool Create(wxWindow *parent, UINT id);

    virtual void DoPaint(wxDC* pDC);

    virtual wxSize CalcFixedLayout(bool stretch, bool horz);
    virtual wxSize CalcDynamicLayout(int length, uint32_t mode);

    //virtual void OnUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler);

    wxSize CalcLayout(uint32_t mode, int length);

    virtual void OnBarStyleChange(uint32_t dwOldStyle, uint32_t dwNewStyle);

    // Generated message map functions
protected:
    afx_msg bool OnEraseBkgnd(wxDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnNcPaint();
    afx_msg void OnDestroy();
    //afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnCloseWnd();
    //afx_msg bool OnToolTipGetText(UINT uId, NMHDR* pNmHdr, LRESULT* pResult);

    static std::string s_strWndClass;

    void RegisterWndClass();
    wxSize m_sizeDefault;
    void Resize();

    LRESULT OnDelayedResize(WPARAM, LPARAM);
};

#endif /* DYNAMIC_HELP__ */
