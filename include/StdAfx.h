/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

/*************************************************************************/

#ifndef STDAFX_H__
#define STDAFX_H__

/*************************************************************************/

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define REWRITE_TO_WX_WIDGET 0

#include <stdint.h>
#include <string.h>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/config.h>
#include <wx/clipbrd.h>
#include <wx/docmdi.h>
#include <wx/docview.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/notebook.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/stc/stc.h>
#include <wx/utils.h>
#include <wx/validate.h>
#include <wx/valnum.h>
#include <wx/xrc/xmlres.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <ranges>
#include <regex>
#include <span>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cstdio>
#include <cctype>

#include <fmt/core.h>

#ifndef wxHAVE_DPI_INDEPENDENT_PIXELS
// Hack for wxWidgets 3.0 on Ubuntu
# define FromDIP(X_) (X_)
#endif

#define UCHAR unsigned char
#define USHORT unsigned short
#define UINT unsigned int
#define ULONG unsigned long

#if wxUSE_STD_IOSTREAM
typedef std::istream DocIStream;
typedef std::ostream DocOStream;
#else
typedef wxInputStream DocIStream;
typedef wxOutputStream DocOStream;
#endif

#include "debug.h"
#include "Except.h"
#include "Utils.h"
#include "StrUtils.h"
#include "Singleton.h"
#include "property.h"

#include "wxExtra.h"

#define UNUSED(X_) (void)(X_)

#ifdef _MSC_VER
# define strcasecmp _stricmp
# define chdir _chdir
#endif

#ifndef HIWORD
inline
constexpr uint16_t HIWORD(uint32_t v) { return static_cast<uint16_t>((v >> 16) & 0x0000'FFFF); }
#endif

#ifndef LOWORD
inline
constexpr uint16_t LOWORD(uint32_t v) { return static_cast<uint16_t>(v & 0x0000'FFFF); }
#endif

// Some Win32/MFC temp place holders

#define afx_msg

#define LRESULT int
#define WPARAM uint32_t
#define LPARAM uint32_t

// No indication as to what this POSITION type actually is, we're stubbing in an int for now. -- B.Simonds (April 25, 2024)
typedef int POSITION;

// Couple of class stubs from MFC that we need to convert.

class CCmdUI;

// Another stub
class CMemoryException
{
public:
    CMemoryException() {}
    virtual ~CMemoryException() {}
};

/*************************************************************************/

#endif /* STDAFX_H__ */

/*************************************************************************/
