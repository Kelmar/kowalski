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

#include <wx/colordlg.h>

#include "Options.h"
#include "options/OptionsDialog.h"

#include "ConfigSettings.h"

/*************************************************************************/

//int OptionsDialog::s_lastActivePage = 0;

/*************************************************************************/

OptionsDialog::OptionsDialog()
    : wxDialog()
    , wxExtra(this)
    , m_notebook(nullptr)
{
}

OptionsDialog::~OptionsDialog()
{
    //s_lastActivePage = m_notebook->GetSelection();
}

/*************************************************************************/

void OptionsDialog::Create(wxFrame *parent)
{
    if (!wxXmlResource::Get()->LoadDialog(this, parent, "OptionsDlg"))
        throw ResourceError();

    m_notebook = FindChild<wxNotebook>("m_notebook");

#if 0

    m_SymPage = new COptionsSymPage(m_notebook);
    m_AsmPage = new COptionsAsmPage(m_notebook);
    //m_EditPage = new COptionsEditPage(m_notebook);
    //m_DeasmPage = new COptionsDeasmPage(m_notebook);
    //m_MarksPage = new COptionsMarksPage(m_notebook);
    //m_ViewPage = new COptionsViewPage(m_notebook);

    m_notebook->AddPage(m_SymPage, _("Simulator"));
    m_notebook->AddPage(m_AsmPage, _("Assembler"));
    //m_notebook->AddPage(m_EditPage, _("Editor"));
    //m_notebook->AddPage(m_DeasmPage, _("Disassembler"));
    //m_notebook->AddPage(m_MarksPage, _("General"));
    //m_notebook->AddPage(m_ViewPage, _("Appearance"));

#endif

    // set up HH_POPUP defaults for all context sensitive help
    // Initialize structure to NULLs
#if REWRITE_TO_WX_WIDGET
    memset(&hPop, 0, sizeof(hPop));
    // Set size of structure
    hPop.cbStruct = sizeof(hPop);
    hPop.clrBackground = RGB(255, 255, 208); // Yellow background color
    hPop.clrForeground = -1; // Font color //  black font
    hPop.rcMargins.top = -1;
    hPop.rcMargins.left = -1;
    hPop.rcMargins.bottom = -1;
    hPop.rcMargins.right = -1;
    hPop.pszFont = NULL; // Font
#endif

    UpdateSize();
}

/*************************************************************************/

void OptionsDialog::AddPage(const OptionsPageFactory &factory, const wxString &text)
{
    auto page = factory(m_notebook);

    m_pages.push_back(page);
    m_notebook->AddPage(page, text);
}

/*************************************************************************/

void OptionsDialog::UpdateSize()
{
    auto sizer = this->GetSizer();
    sizer->Layout();

    Fit();
}

/*************************************************************************/
// OptionsDialog message handlers

/*************************************************************************/
// Context Sensitive Help

#if REWRITE_TO_WX_WIDGET

bool OptionsDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if (pHelpInfo->iCtrlId > 1000 && pHelpInfo->iCtrlId < 2999)
    {
        hPop.pt = pHelpInfo->MousePos;
        hPop.idString = pHelpInfo->iCtrlId;
        HtmlHelpA((uint32_t)(void*)&hPop, HH_DISPLAY_TEXT_POPUP);
    }

    return true;
}

#endif

void OptionsDialog::OnContextMenu(wxWindow* pWnd, wxPoint point)
{
    UNUSED(pWnd);
    UNUSED(point);

    //HtmlHelpA(NULL, HH_DISPLAY_TOPIC);
}

/*************************************************************************/
