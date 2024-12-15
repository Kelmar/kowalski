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

#ifndef PROJECT_MANAGER_H__
#define PROJECT_MANAGER_H__

/*************************************************************************/

#include "StdAfx.h"
#include "Archive.h"
#include "LoadCodeOptions.h"

// Atari - archive, mem, area, prog_start
// Intel - archive, mem, area, prog_start
// Moto  - archive, mem, area, prog_start
// Bin   - archive, start, end (raw bin)
// 65p   - archive, from, to (some header)

/*************************************************************************/

/**
 * @brief Code file reader/writer
 */
class CodeTemplate
{
private:
    mutable std::string m_toStringCache;
    mutable std::vector<std::regex> m_exprCache;

    std::vector<std::string> getWildCards() const;

protected:
    /* constructor */ CodeTemplate() { }

public:
    virtual ~CodeTemplate() { }

    /// Check if we can read this format from disk
    virtual bool canRead() const = 0;

    /// Checks if we can write this format to disk
    
    virtual bool canWrite() const = 0;

    /// Returns if this format is in binary or not.
    virtual bool isBinary() const = 0;

    /// Returns a human readable description of the file type.
    virtual std::string getDescription() const = 0;

    /// Returns a list of file extensions supported by this template.
    virtual std::vector<std::string> getExtensions() const = 0;

    /// Checks to see if this code template supports the supplied file extension.
    virtual bool supportsExt(const std::string &ext) const;

    virtual bool read(const std::string &path, LoadCodeState *state) = 0;
    virtual bool write(const std::string &path, LoadCodeState *state) = 0;

    virtual std::string toString() const;
};

/*************************************************************************/

class BinaryCodeTemplate : public CodeTemplate
{
protected:
    /* constructor */ BinaryCodeTemplate() { }

    virtual bool read(BinaryArchive &ar, LoadCodeState *state) = 0;
    virtual bool write(BinaryArchive &ar, LoadCodeState *state) = 0;

public:
    virtual bool isBinary() const { return true; }

    virtual bool read(const std::string &path, LoadCodeState *state);
    virtual bool write(const std::string &path, LoadCodeState *state);
};

/*************************************************************************/

class ProjectManager : public wxEvtHandler
{
private:
    friend class C6502App;

    std::vector<std::shared_ptr<CodeTemplate>> m_templates;

    /* constructor */ ProjectManager();

    wxString GetSupportedFileTypes(std::function<bool (const std::shared_ptr<CodeTemplate> &)> predicate);

public:
    virtual ~ProjectManager();

    template <typename T>
    void AddTemplate()
        requires(std::is_base_of_v<CodeTemplate, T>)
    {
        AddTemplate(std::shared_ptr<CodeTemplate>(new T()));
    }

    void AddTemplate(const std::shared_ptr<CodeTemplate> &codeTemplate);

public:
    // Event handlers
    void OnLoadCode(wxCommandEvent &);
    void OnSaveCode(wxCommandEvent &);

private:
    wxDECLARE_EVENT_TABLE();
};

/*************************************************************************/

#endif /* PROJECT_MANAGER_H__ */

/*************************************************************************/
