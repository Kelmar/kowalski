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

#include "wx/fontenum.h"

#include "FontController.h"
#include "options/OptionsFontPage.h"

/*************************************************************************/

namespace
{
    class FontPageFactory : public OptionPageFactory
    {
    public:
        virtual POptionsPage Create(wxBookCtrlBase *parent) const override
        {
            return std::make_shared<OptionsFontPage>(parent);
        }
    };
}

/*************************************************************************/
/*************************************************************************/

FontController::FontController()
    : m_monoFont(nullptr)
    , m_cellSize()
{
    LoadFonts();
    CalcCellSizes();
}

FontController::~FontController()
{
    delete m_monoFont;
}

/*************************************************************************/

void FontController::LoadFonts()
{
    wxFontInfo info;
    info.Family(wxFONTFAMILY_TELETYPE);
    info.AntiAliased(true);

    delete m_monoFont;
    m_monoFont = new wxFont(info);
}

/*************************************************************************/

void FontController::CalcCellSizes()
{
    // Premeasure fonts.
    wxMemoryDC dc;
    dc.SetFont(*m_monoFont);

    m_cellSize = dc.GetTextExtent(" ");
}

/*************************************************************************/

void FontController::InitOptions()
{
    wxGetApp().optionsController().RegisterPage(
        std::make_shared<FontPageFactory>(),
        _("Fonts and Colors")
    );
}

/*************************************************************************/

std::vector<BasicFontInfo> FontController::GetInstalledFonts()
{
    wxFontEnumerator fontEnum;

    auto fontNames = fontEnum.GetFacenames();

    std::vector<BasicFontInfo> rval;

    for (wxString name : fontNames)
    {
        auto fnt = std::make_unique<wxFont>(wxFontInfo(10).FaceName(name));

        if (!fnt->IsOk())
            continue;

        rval.push_back(BasicFontInfo
            {
                .name = name,
                .isFixedWidth = fnt->IsFixedWidth()
            });
    }

    return rval;
}

/*************************************************************************/
