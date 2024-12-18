/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
 /*************************************************************************/

#include "StdAfx.h"
#include "6502.h"

#include "Options.h"
#include "options/OptionsFontPage.h"
#include "FontController.h"

/*************************************************************************/

OptionsFontPage::OptionsFontPage(wxBookCtrlBase *parent)
    : OptionsPage()
    , wxExtra(this)
{
    if (!wxXmlResource::Get()->LoadPanel(this, parent, "OptionsFontPage"))
        throw ResourceError();

    m_categoryChoice = FindChild<wxChoice>("m_categoryChoice");
    m_fontChoice = FindChild<wxChoice>("m_fontCombo");

    InitCategories();
    InitFontList();
}

OptionsFontPage::~OptionsFontPage()
{
}

/*************************************************************************/

void OptionsFontPage::InitCategories()
{
    m_categoryChoice->Append(_("Editor"));
    m_categoryChoice->Append(_("Memory View"));

    m_categoryChoice->SetSelection(0);
}

/*************************************************************************/

void OptionsFontPage::InitFontList()
{
    for (auto font : wxGetApp().fontController().GetInstalledFonts())
    {
        wxString name = font.name;

        if (font.isFixedWidth)
            name += " *";

        m_fontChoice->Append(name);
    }
}
/*************************************************************************/

void OptionsFontPage::AbortChanges()
{
}

/*************************************************************************/

void OptionsFontPage::SaveChanges()
{
}

/*************************************************************************/
