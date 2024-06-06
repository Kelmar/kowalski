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

#include "AtariBin.h"
#include "MotorolaSRecord.h"

/*************************************************************************/
/*************************************************************************/

bool CodeTemplate::SupportsExt(const std::string &ext) const
{
    // GCC 11 makes us assign to a useless temp variable first. >_<
    auto exts = GetExtensions();
    auto e = exts | std::views::transform(str::toLower);

    return std::ranges::find(e, ext | str::trim | str::toLower) != e.end();
}

/*************************************************************************/

std::string CodeTemplate::ToString() const
{
    // GCC 11 makes us assign to a useless temp variable first. >_<
    auto exts = GetExtensions();

    auto filters = exts
        | std::views::transform(str::toLower)
        | std::views::transform([](auto s) -> std::string { return "*." + s; });

    return fmt::format("{} ({})", GetDescription(), str::join(";", filters));
}

/*************************************************************************/

void CodeTemplate::Read(const std::string &filename, LoadCodeState *state)
{
    UNUSED(filename);
    UNUSED(state);
}

/*************************************************************************/

void CodeTemplate::Write(const std::string &filename, LoadCodeState *state)
{
    UNUSED(filename);
    UNUSED(state);
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
    AddTemplate<CAtariBin>();
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
    return str::join("|",
        m_templates
        | std::views::filter(predicate)
        | std::ranges::views::transform([](auto ptr) -> std::string { return ptr->ToString(); })
    );
}

/*************************************************************************/

wxBEGIN_EVENT_TABLE(ProjectManager, wxEvtHandler)
    EVT_MENU(evID_LOAD_CODE, ProjectManager::OnLoadCode)
    EVT_MENU(evID_SAVE_CODE, ProjectManager::OnSaveCode)
wxEND_EVENT_TABLE()

/*************************************************************************/

void ProjectManager::OnLoadCode(wxCommandEvent &event)
{
    // TODO: Remove static qualifier from this, and save values to registry.
    /*static LoadCodeState options =
    {
        0, false, 0
    };*/

    LoadCodeState state;

    event.Skip();

    wxString exts = GetSupportedFileTypes([](auto t) -> bool { return t->CanRead(); });
    std::string foo = exts.ToStdString();

    std::unique_ptr<wxFileDialog> fileDlg(new wxFileDialog());

    if (fileDlg->ShowModal() != wxID_OK)
        return;

    std::string path = fileDlg->GetFilename().ToStdString();

    std::string ext = file::getExtension(path);
    std::shared_ptr<CodeTemplate> codeTmp;

    if (ext.empty())
    {
        // Try based on selected drop down.
        int filterIdx = fileDlg->GetFilterIndex();

        codeTmp = m_templates[filterIdx];
    }
    else
    {
        auto supportsExt = [&ext](std::shared_ptr<CodeTemplate> t) { return t->SupportsExt(ext); };
        codeTmp = *std::ranges::find_if(m_templates, supportsExt);
    }

    if (!codeTmp)
    {
        // TODO: In the future we might be able to do this with some magic bytes.

        wxString err = "Unable to find supported file type for filename: " + path;

        wxLogWarning(err);
        wxMessageBox(err);

        return;
    }

    codeTmp->Read(path, &state);
    
    std::unique_ptr<LoadCodeOptionsDlg> optDlg(new LoadCodeOptionsDlg(&state));

    int res = optDlg->ShowModal();

    if (res != wxID_OK)
        return;
}

/*************************************************************************/

void ProjectManager::OnSaveCode(wxCommandEvent &event)
{
    event.Skip();

    wxString exts = GetSupportedFileTypes([](auto t) -> bool { return t->CanWrite(); });
}

/*************************************************************************/
