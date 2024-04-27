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

#ifndef DIAL_BAR_H__
#define DIAL_BAR_H__

class CDialBar : public wxFrame // CDialogBar
{
public:
    /* construtor */ CDialBar();
    virtual         ~CDialBar();

    bool Create(wxWindow* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID);

    virtual wxSize CalcFixedLayout(bool stretch, bool horz);
    virtual wxSize CalcDynamicLayout(int length, uint32_t mode);
    
    //afx_msg void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
    afx_msg void OnLButtonDown(UINT nFlags, wxPoint point);
    
private:
    wxSize m_lastSize;
    wxSize CalcLayout(int length, uint32_t mode);
};

#endif /* DIAL_BAR_H__ */
