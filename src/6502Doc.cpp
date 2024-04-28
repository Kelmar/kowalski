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

// 6502Doc.cpp : implementation of the CSrc6502Doc class
//

#include "StdAfx.h"
#include "6502Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc

//IMPLEMENT_DYNCREATE(CSrc6502Doc, CDocument)

//BEGIN_MESSAGE_MAP(CSrc6502Doc, CDocument)
    //{{AFX_MSG_MAP(CSrc6502Doc)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc construction/destruction

CSrc6502Doc::CSrc6502Doc()
{
#ifdef USE_CRYSTAL_EDIT
    m_TextBuffer.m_pOwnerDoc = this;
#endif
}

CSrc6502Doc::~CSrc6502Doc()
{
#ifdef USE_CRYSTAL_EDIT
//	m_TextBuffer.FreeAll();
#endif
}

bool CSrc6502Doc::OnNewDocument()
{
    //if (!CDocument::OnNewDocument())
    //    return false;

#ifdef USE_CRYSTAL_EDIT
    m_TextBuffer.InitNew();
#endif

    static UINT no = 1;
    char name[32];
    snprintf(name, sizeof(name), "NewFile%u", no++);

    //SetPathName(name, false);

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc serialization

void CSrc6502Doc::Serialize(CArchive &ar)
{
#ifdef USE_CRYSTAL_EDIT
    //no-op
#else
    // CEditView contains an edit control which handles all serialization
    //((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc diagnostics

#ifdef _DEBUG
void CSrc6502Doc::AssertValid() const
{
    //CDocument::AssertValid();
}

void CSrc6502Doc::Dump(CDumpContext& dc) const
{
    //CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_CRYSTAL_EDIT

void CSrc6502Doc::DeleteContents()
{
    //CDocument::DeleteContents();
    m_TextBuffer.FreeAll();
}


bool CSrc6502Doc::OnOpenDocument(const char *pathName)
{
    //if (!CDocument::OnOpenDocument(pathName))
    //    return false;

    return m_TextBuffer.LoadFromFile(pathName);
}


bool CSrc6502Doc::OnSaveDocument(const char *pathName)
{
    m_TextBuffer.SaveToFile(pathName);
    return true;
}

#else

bool CSrc6502Doc::DeleteContents()
{
    //CDocument::DeleteContents();
    return false;
}

bool CSrc6502Doc::OnOpenDocument(const char *pathName)
{
    //return CDocument::OnOpenDocument(pathName);
    return false;
}

bool CSrc6502Doc::OnSaveDocument(const char *pathName)
{
    //return CDocument::OnSaveDocument(pathName);
    return false;
}

#endif
