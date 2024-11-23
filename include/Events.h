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

#ifndef EVENT_6502_H__
#define EVENT_6502_H__

/*************************************************************************/

#include "StdAfx.h"

/*************************************************************************/

class MessageEvent : public wxThreadEvent
{
private:
    wxString m_target;

public:
    /* constructor */ MessageEvent(const MessageEvent &other)
        : wxThreadEvent(other)
        , m_target(other.m_target)
    { }

    /* constructor */ MessageEvent(wxEventType eventType, int id, const char *target)
        : wxThreadEvent(eventType, id)
        , m_target(target)
    { }

    wxString GetTarget() const { return m_target; }

    virtual wxEvent *Clone() const { return new MessageEvent(*this); }
};

/*************************************************************************/

// File menu events
wxDECLARE_EVENT(evID_LOAD_CODE, wxCommandEvent);
wxDECLARE_EVENT(evID_SAVE_CODE, wxCommandEvent);

// View menu events
wxDECLARE_EVENT(evID_SHOW_LOG, wxCommandEvent);
wxDECLARE_EVENT(evID_SHOW_DISASM, wxCommandEvent);
wxDECLARE_EVENT(evID_SHOW_REGS, wxCommandEvent);
wxDECLARE_EVENT(evID_SHOW_MEMORY, wxCommandEvent);
wxDECLARE_EVENT(evID_SHOW_OUTPUT, wxCommandEvent);
wxDECLARE_EVENT(evID_SHOW_IOWINDOW, wxCommandEvent);
wxDECLARE_EVENT(evID_SHOW_TEST, wxCommandEvent);

// Simulator menu events
wxDECLARE_EVENT(evID_ASSEMBLE, wxCommandEvent);
wxDECLARE_EVENT(evID_RUN, wxCommandEvent);
wxDECLARE_EVENT(evID_STOP, wxCommandEvent);
wxDECLARE_EVENT(evID_RESET, wxCommandEvent);
wxDECLARE_EVENT(evID_BREAK, wxCommandEvent);
wxDECLARE_EVENT(evID_STEP_INTO, wxCommandEvent);
wxDECLARE_EVENT(evID_STEP_OVER, wxCommandEvent);
wxDECLARE_EVENT(evID_STEP_OUT, wxCommandEvent);
wxDECLARE_EVENT(evID_RUN_TO, wxCommandEvent);

wxDECLARE_EVENT(evID_OPTIONS, wxCommandEvent);

static const int evTHD_ASM_COMPLETE = wxID_HIGHEST + 1;
static const int evTHD_OUTPUT_ID = wxID_HIGHEST + 2;

wxDECLARE_EVENT(evTHD_OUTPUT, MessageEvent);

/*************************************************************************/

#endif /* EVENT_6502_H__ */

/*************************************************************************/
