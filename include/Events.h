/*************************************************************************/

#ifndef EVENT_6502_H__
#define EVENT_6502_H__

/*************************************************************************/

#include "StdAfx.h"

// List of application defined events.

wxDEFINE_EVENT(evID_SHOW_DISASM, wxCommandEvent);
wxDEFINE_EVENT(evID_SHOW_REGS, wxCommandEvent);

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

#endif /* EVENT_6502_H__ */

/*************************************************************************/
