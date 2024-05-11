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

#ifndef PROJECT_MANAGER_H__
#define PROJECT_MANAGER_H__

/*************************************************************************/

class ProjectManager : public wxEvtHandler
{
private:
    static ProjectManager *s_self;

public:
    /* constructor */ ProjectManager();
    virtual          ~ProjectManager();

    static ProjectManager &Get()
    {
        ASSERT(s_self);
        return *s_self;
    }

    static ProjectManager *Ptr() { return s_self; }

public:
    // Event handlers
    void OnLoadCode(wxCommandEvent &);

private:
    wxDECLARE_EVENT_TABLE();
};

/*************************************************************************/

#endif /* PROJECT_MANAGER_H__ */

/*************************************************************************/
