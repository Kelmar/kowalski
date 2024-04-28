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

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define WINVER 0xA00

#define REWRITE_TO_WX_WIDGET 0

#include <stdint.h>

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/docmdi.h>
#include <wx/docview.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/settings.h>

#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cstdio>

#define UCHAR unsigned char
#define USHORT unsigned short
#define UINT unsigned int
#define ULONG unsigned long

#include <cassert>

#ifdef _DEBUG
// Just mapping this to the C assert() function for now.
# define ASSERT(X_) assert(X_)
#else
# define ASSERT(X_)
#endif

#define VERIFY(X_) assert(X_)

#define __AFXWIN_H__

// Some Win32/MFC temp place holders

#define afx_msg

#define LRESULT int
#define WPARAM uint32_t
#define LPARAM uint32_t

// No idication as to what this POSITION type actually is, we're stubbing in an int for now. -- B.Simonds (April 25, 2024)
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

//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxtempl.h>
//#include <AFXCVIEW.H>
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>			// MFC support for Windows 95 Common Controls
//#endif // _AFX_NO_AFXCMN_SUPPORT

//#include <afxpriv.h>
//#include <afxole.h>

//#include <AFXMT.H>

//#pragma warning(disable : 4800)

//#include "Shlwapi.h"

#include "6502.h"

#endif /* STDAFX_H__ */
