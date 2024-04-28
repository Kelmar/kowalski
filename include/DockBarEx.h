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

#ifndef DOCBAR_EX_H__
#define DOCBAR_EX_H__

/////////////////////////////////////////////////////////////////////////////
// CDockBarEx window

class CDockBarEx : public wxFrame // public CDockBar
{
public:
    /* constructor */ CDockBarEx();
    virtual ~CDockBarEx();

    //virtual wxSize CalcFixedLayout(bool stretch, bool horz);
    //virtual bool PreCreateWindow(CREATESTRUCT& cs);

protected:
    afx_msg bool OnSetCursor(wxWindow *pWnd, UINT nHitTest, UINT message);
    afx_msg LRESULT OnNcHitTest(wxPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    //afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    
    afx_msg LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitSizeMove(WPARAM wParam, LPARAM lParam);

    wxRect ResizeArea();
    bool m_bResize;
    wxPoint m_ptStart;
    int m_nStartWidth;
    void ResizeBars(int nWidth);
    bool m_bResizing;
    int m_nDeltaWidth;
};

#endif /* DOCBAR_EX_H__ */
