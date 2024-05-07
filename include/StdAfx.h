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

#ifndef STDAFX_H__
#define STDAFX_H__

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define REWRITE_TO_WX_WIDGET 0

#include <stdint.h>
#include <string.h>

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/config.h>
#include <wx/docmdi.h>
#include <wx/docview.h>
#include <wx/event.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/notebook.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/settings.h>
#include <wx/stc/stc.h>

#include <algorithm>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cstdio>
#include <cctype>

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
#include "Utils.h"

#ifdef _MSC_VER
# define strcasecmp stricmp
#endif

#define __AFXWIN_H__

// Some Win32/MFC temp place holders

#define afx_msg

#define LRESULT int
#define WPARAM uint32_t
#define LPARAM uint32_t

// No indication as to what this POSITION type actually is, we're stubbing in an int for now. -- B.Simonds (April 25, 2024)
typedef int POSITION;

// Couple of class stubs from MFC that we need to convert.

class CCmdUI;

// Stub for now, need full exception system; wxWidgets doesn't provide one.
class CFileException
{
public:
    CFileException(int err) { }
    virtual ~CFileException() { }
};

// Another stub
class CMemoryException
{
public:
    CMemoryException() {}
    virtual ~CMemoryException() {}
};

#include "6502.h"

#endif /* STDAFX_H__ */
