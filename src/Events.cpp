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

#include "Events.h"

/*************************************************************************/

// File menu events
wxDEFINE_EVENT(evID_LOAD_CODE, wxCommandEvent);
wxDEFINE_EVENT(evID_SAVE_CODE, wxCommandEvent);

// View menu events
wxDEFINE_EVENT(evID_SHOW_OUTPUT, wxCommandEvent);
wxDEFINE_EVENT(evID_SHOW_LOG, wxCommandEvent);
wxDEFINE_EVENT(evID_SHOW_DISASM, wxCommandEvent);
wxDEFINE_EVENT(evID_SHOW_REGS, wxCommandEvent);
wxDEFINE_EVENT(evID_SHOW_MEMORY, wxCommandEvent);
wxDEFINE_EVENT(evID_SHOW_TEST, wxCommandEvent);

// Simulator menu events
wxDEFINE_EVENT(evID_ASSEMBLE, wxCommandEvent);
wxDEFINE_EVENT(evID_DEBUG, wxCommandEvent);
wxDEFINE_EVENT(evID_RUN, wxCommandEvent);
wxDEFINE_EVENT(evID_RESET, wxCommandEvent);
wxDEFINE_EVENT(evID_BREAK, wxCommandEvent);
wxDEFINE_EVENT(evID_STEP_INTO, wxCommandEvent);
wxDEFINE_EVENT(evID_STEP_OVER, wxCommandEvent);
wxDEFINE_EVENT(evID_STEP_OUT, wxCommandEvent);
wxDEFINE_EVENT(evID_RUN_TO, wxCommandEvent);

/*************************************************************************/
