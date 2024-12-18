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

#ifndef OPTIONS_DIALOG_6502_H__
#define OPTIONS_DIALOG_6502_H__

/*************************************************************************/

class OptionsDialog : public wxDialog, public wxExtra
{
private:
    int s_lastActivePage;

    wxNotebook *m_notebook;

    std::vector<OptionsPage *> m_pages;

public:
    /* constructor */ OptionsDialog();
    virtual          ~OptionsDialog();

    void Create(wxFrame *parent);

    void AddPage(const OptionsPageFactory &factory, const wxString &text);
    void UpdateSize();

    const std::vector<OptionsPage *> &GetPages() const { return m_pages; }

protected:
    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
};

/*************************************************************************/

#endif /* OPTIONS_6502_H__ */

/*************************************************************************/
