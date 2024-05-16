/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*************************************************************************/

#include "StdAfx.h"

#include "LoadCodeOptions.h"

#include "Events.h"
#include "MainFrm.h"
#include "ProjectManager.h"

#include "MotorolaSRecord.h"

/*************************************************************************/
/*************************************************************************/

bool CodeTemplate::SupportsExt(const std::string &ext) const
{
    // GCC 11 makes us assign to a useless temp variable first. >_<
    std::vector<std::string> exts = GetExtensions();
    auto e = exts | std::views::transform(str::tolower);

    return std::ranges::find(e, ext | str::trim | str::tolower) != e.end();
}

/*************************************************************************/

std::string CodeTemplate::ToString() const
{
    // GCC 11 makes us assign to a useless temp variable first. >_<
    auto exts = GetExtensions();

    auto filters = exts
        | std::views::transform(str::tolower)
        | std::views::transform([](auto s) -> std::string { return "*." + s; });

    // Using wsString::Format() until we can get std::format() from GNU... >_<

    std::string desc = GetDescription();
    std::string allExts = str::join(";", filters);
        
    return wxString::Format("%s (%s)", desc.c_str(), allExts.c_str()).ToStdString();
    //return std::format("{} ({})", GetDescription(), str::join(";", exts));
}

/*************************************************************************/
/*************************************************************************/

ProjectManager *ProjectManager::s_self = nullptr;

/*************************************************************************/

ProjectManager::ProjectManager()
    : m_templates()
{
    ASSERT(s_self == nullptr);

    s_self = this;

    InitCodeTemplates();
}

ProjectManager::~ProjectManager()
{
    s_self = nullptr;
}

/*************************************************************************/

void ProjectManager::InitCodeTemplates()
{
    // Hard coded for now.

    //AddTemplate<CMotorolaSRecord>();
}

/*************************************************************************/

void ProjectManager::AddTemplate(const std::shared_ptr<CodeTemplate> &codeTemplate)
{
    ASSERT(codeTemplate);

    m_templates.push_back(codeTemplate);
}

/*************************************************************************/

wxString ProjectManager::GetSupportedFileTypes(
    std::function<bool(const std::shared_ptr<CodeTemplate> &)> predicate)
{
    return str::join("|", m_templates | std::views::filter(predicate));
}

/*************************************************************************/

wxBEGIN_EVENT_TABLE(ProjectManager, wxEvtHandler)
    EVT_MENU(evID_LOAD_CODE, ProjectManager::OnLoadCode)
    EVT_MENU(evID_SAVE_CODE, ProjectManager::OnSaveCode)
wxEND_EVENT_TABLE()

/*************************************************************************/

void ProjectManager::OnLoadCode(wxCommandEvent &event)
{
    event.Skip();

    wxString exts = GetSupportedFileTypes([](auto t) -> bool { return t->CanRead(); });

    auto optDlg = new CLoadCodeOptions();
    optDlg->ShowModal();

    //auto dlg = new wxFileDialog(this);

    //dlg->ShowModal();
}

/*************************************************************************/

void ProjectManager::OnSaveCode(wxCommandEvent &event)
{
    event.Skip();

    wxString exts = GetSupportedFileTypes([](auto t) -> bool { return t->CanWrite(); });
}

/*************************************************************************/
