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

#include "StdAfx.h"

#include "6502Doc.h"
#include "6502View.h"

/*************************************************************************/
// Construction/Destruction

CSrc6502Doc::CSrc6502Doc()
    : wxDocument()
{
}

CSrc6502Doc::~CSrc6502Doc()
{
}

wxIMPLEMENT_DYNAMIC_CLASS(CSrc6502Doc, wxDocument);

/*************************************************************************/

CSrc6502View *CSrc6502Doc::GetSourceView()
{
    return dynamic_cast<CSrc6502View *>(GetFirstView());
}

/*************************************************************************/
// Document Create/Load/Save

bool CSrc6502Doc::OnCreate(const wxString& path, long flags)
{
    if (!wxDocument::OnCreate(path, flags))
	return false;

    BindViews();

    return true;
}

bool CSrc6502Doc::DoOpenDocument(const wxString &filename)
{
    CSrc6502View *sourceView = GetSourceView();

    if (!sourceView->m_text->LoadFile(filename))
	return false;

    Modify(false);

    return true;
}

bool CSrc6502Doc::DoSaveDocument(const wxString &filename)
{
    CSrc6502View *sourceView = GetSourceView();

    if (!sourceView->m_text->SaveFile(filename))
	return false;

    Modify(false);

    wxString str;
    str.Printf("Saved: %s", filename);

    wxLogStatus(str.c_str());

    return true;
}

/*************************************************************************/

void CSrc6502Doc::BindViews()
{
    CSrc6502View *sourceView = GetSourceView();

    sourceView->m_text->Bind(wxEVT_STC_MODIFIED, &CSrc6502Doc::OnTextChange, this);
}

/*************************************************************************/

void CSrc6502Doc::OnTextChange(wxCommandEvent& event)
{
    Modify(true);
    event.Skip();
}

/*************************************************************************/
