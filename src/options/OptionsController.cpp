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

#include "MainFrm.h"

#include "Options.h"
#include "options/OptionsDialog.h"

/*************************************************************************/

OptionsController::OptionsController()
    : m_mainFrame(nullptr)
    , m_pageFactories()
{
    BindEvents();
}

OptionsController::~OptionsController()
{
}

/*************************************************************************/

bool OptionsController::Init(CMainFrame *mainFrame)
{
    ASSERT(mainFrame);

    m_mainFrame = mainFrame;
    return true;
}

/*************************************************************************/

void OptionsController::RegisterPage(OptionsPageFactory factory, const wxString &text)
{
    m_pageFactories.push_back(PageInfo
    {
        .factory = factory,
        .text = text
    });
}

/*************************************************************************/

void OptionsController::BuildMenu(wxMenuBar *)
{

}

/*************************************************************************/

void OptionsController::BindEvents()
{
    Bind(wxEVT_MENU, &OptionsController::OnOptions, this, evID_OPTIONS);
}

/*************************************************************************/

void OptionsController::OnOptions(wxCommandEvent &)
{
    auto options = std::make_unique<OptionsDialog>();

    options->Create(m_mainFrame);

    for (auto info : m_pageFactories)
        options->AddPage(info.factory, info.text);

    options->UpdateSize();

    if (options->ShowModal() == wxID_OK)
    {
        for (auto page : options->GetPages())
            page->SaveChanges();
    }
    else
    {
        for (auto page : options->GetPages())
            page->AbortChanges();
    }
}

/*************************************************************************/
