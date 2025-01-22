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

#ifndef DOC_6502_ASM_H__
#define DOC_6502_ASM_H__

/*=======================================================================*/

class CSrc6502Doc : public wxDocument
{
private:
    void BindViews();

    CSrc6502View *GetSourceView();

    void OnTextChange(wxCommandEvent& event);

protected:
    wxDECLARE_DYNAMIC_CLASS(CSrc6502Doc);
    wxDECLARE_NO_COPY_CLASS(CSrc6502Doc);

public:
    /* constructor */ CSrc6502Doc();
    virtual          ~CSrc6502Doc();

    bool OnCreate(const wxString& path, long flags) override;

    bool DoOpenDocument(const wxString &filename) override;
    bool DoSaveDocument(const wxString &filename) override;
};

/*=======================================================================*/

#endif /* DOC_6502_ASM_H__ */

/*=======================================================================*/

