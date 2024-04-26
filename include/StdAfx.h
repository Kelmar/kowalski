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

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define WINVER 0xA00

#include <stdint.h>

#include <wx/wx.h>

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

#ifdef _DEBUG
# include <cassert>
// Just mapping this to the C assert() function for now.
# define ASSERT(X_) std::assert(X_)
#else
# define ASSERT(X_)
#endif

#define __AFXWIN_H__

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
