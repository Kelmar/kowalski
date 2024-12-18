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

#ifndef OPTIONS_CONTROLLER_6502_H__
#define OPTIONS_CONTROLLER_6502_H__

/*************************************************************************/

#include "MainFrm.h"

/*************************************************************************/

class OptionsPage : public wxPanel
{
protected:
    OptionsPage() : wxPanel() { }

public:
    virtual ~OptionsPage() { }

    /**
     * @brief Called by OptionsDialog when any pending changes should be discarded.
     */
    virtual void AbortChanges() = 0;

    /**
     * @brief Called by OptionsDialog when any pending changes to should be commited.
     */
    virtual void SaveChanges() = 0;
};

/*************************************************************************/

typedef std::function<OptionsPage *(wxBookCtrlBase *)> OptionsPageFactory;

/*************************************************************************/

class OptionsController : public wxEvtHandler
{
private:
    struct PageInfo
    {
        OptionsPageFactory factory;
        wxString text;
    };

    CMainFrame *m_mainFrame;
    std::vector<PageInfo> m_pageFactories;

    void BindEvents();
    void OnOptions(wxCommandEvent &);

public:
    /* constructor */ OptionsController();
    virtual ~OptionsController();

    bool Init(CMainFrame *mainFrame);

public:
    void RegisterPage(OptionsPageFactory factory, const wxString &text);

    void BuildMenu(wxMenuBar *);
};

/*************************************************************************/

#endif /* OPTIONS_CONTROLLER_6502_H__ */

/*************************************************************************/