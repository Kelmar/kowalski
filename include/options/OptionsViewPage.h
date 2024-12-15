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

#ifndef OPT_VIEW_PAGE_H__
#define OPT_VIEW_PAGE_H__

/*************************************************************************/

class COptionsViewPage : public wxPanel
{
private:
    wxColourPickerCtrl m_ColorButtonText;
    wxColourPickerCtrl m_ColorButtonBkgnd;
    bool m_bSubclassed;
    int m_nSelection;
    wxColour m_rgbBkgndCol;
    wxColour m_rgbTextCol;

    void repaint_example();
    
public:
    struct TextDef
    {
        wxColour text, bkgnd;
        wxFont font;
        int changed;
        int brush;
    };
    //static TextDef m_Text[];

public:
    /* constructor */ COptionsViewPage(wxBookCtrlBase *parent);
    virtual ~COptionsViewPage();

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    
    afx_msg int OnCtlColor(wxDC* pDC, wxWindow* pWnd, UINT nCtlColor);

    afx_msg void OnSelchangeViewWnd();
    afx_msg void OnViewTxtCol();
    afx_msg void OnViewBkgndCol();
    afx_msg void OnViewFontBtn();

};

/*************************************************************************/

#endif /* OPT_VIEW_PAGE_H__ */

/*************************************************************************/
