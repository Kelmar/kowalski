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

#include "LoadCodeOptions.h"

#include "Events.h"
#include "ProjectManager.h"

/*************************************************************************/
/*************************************************************************/

bool CodeTemplate::supportsExt(const std::string &ext) const
{
    if (m_exprCache.empty())
    {
        for (auto pattern : getWildCards())
            m_exprCache.push_back(std::regex(pattern | str::filePatToRegex));
    }

    return std::ranges::any_of(m_exprCache, str::matches(ext));
}

/*************************************************************************/

std::vector<std::string> CodeTemplate::getWildCards() const
{
    // GCC 11 makes us assign to a useless temp variable first. >_<
    auto exts = getExtensions();

    auto r = exts
        | std::views::transform(str::toLower)
        | std::views::transform([](auto s) -> std::string { return "*." + s; })
        ;

    std::vector<std::string> rval(r.begin(), r.end());

    return rval;
}

/*************************************************************************/

std::string CodeTemplate::toString() const
{
    if (m_toStringCache.empty())
    {
        auto filters = getWildCards();

        m_toStringCache = fmt::format("{0} ({1})|{1}", getDescription(), str::join(";", filters));
    }

    return m_toStringCache;
}

/*************************************************************************/
/*************************************************************************/

bool BinaryCodeTemplate::read(const std::string &filename, LoadCodeState *state)
{
    BinaryArchive archive(filename, Archive::Mode::Read, BinaryArchive::Endianess::Little);

    return read(_In_ archive, state);
}

/*************************************************************************/

bool BinaryCodeTemplate::write(const std::string &filename, LoadCodeState *state)
{
    BinaryArchive archive(filename, Archive::Mode::Write, BinaryArchive::Endianess::Little);

    return write(_In_ archive, state);
}

/*************************************************************************/
/*************************************************************************/

ProjectManager::ProjectManager()
    : m_templates()
{
}

ProjectManager::~ProjectManager()
{
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
        | std::ranges::views::transform([](auto ptr) -> std::string { return ptr->toString(); })
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
    event.Skip();

    // TODO: Remove static qualifier from this, and save values to registry.
    /*static LoadCodeState options =
    {
        0,      // Start address
        false,  // clear memory
        0       // Fill byte
    };*/

    LoadCodeState state;

    wxString exts = GetSupportedFileTypes([](auto t) -> bool { return t->canRead(); });

    std::unique_ptr<wxFileDialog> fileDlg(new wxFileDialog(
        nullptr,
        _("Load Binary Code"),
        "",
        "",
        exts,
        wxFD_OPEN | wxFD_FILE_MUST_EXIST)
    );

    if (fileDlg->ShowModal() != wxID_OK)
        return;

    std::string path = fileDlg->GetPath().ToStdString();

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
        auto supportsExt = [&ext](std::shared_ptr<CodeTemplate> t) { return t->supportsExt(ext); };
        auto found = std::ranges::find_if(m_templates, supportsExt);

        codeTmp = found != m_templates.end() ? *found : nullptr;
    }

    if (!codeTmp)
    {
        // TODO: In the future we might be able to do this with some magic bytes.

        wxString err = _("Unable to find supported file type for filename: ") + path;

        wxLogError(err);

        return;
    }

    if (!codeTmp->read(path, &state))
    {
        wxLogDebug(_("User aborted load"));
        return;
    }

    wxGetApp().m_global.LoadCode(state);
}

/*************************************************************************/

void ProjectManager::OnSaveCode(wxCommandEvent &event)
{
    event.Skip();

    wxString exts = GetSupportedFileTypes([](auto t) -> bool { return t->canWrite(); });
}

/*************************************************************************/
