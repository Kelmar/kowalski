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

// LoadCode.cpp : implementation file
//

#include "StdAfx.h"
#include "LoadCode.h"
#include "LoadCodeOptions.h"
//#include <Dlgs.h>
#include "IntelHex.h"
#include "Code65p.h"

int CLoadCode::m_nInitPos = 0;

/////////////////////////////////////////////////////////////////////////////
// CLoadCode

CLoadCode::CLoadCode(const char *fileName, const char *filter)
#if 0
    : CFileDialog(TRUE, "", fileName,
                OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                filter)
#endif
{
#if 0
    m_strTitle.LoadString(IDS_LOAD_CODE_DLG);

    m_ofn.lpstrTitle = m_strTitle;
    m_ofn.nFilterIndex = m_nInitPos + 1;
#endif
}

//-----------------------------------------------------------------------------

void CLoadCode::LoadCode() // Load code 6502 into the simulator
{
#if 0
    m_nInitPos = m_ofn.nFilterIndex - 1;

    std::string fileName = GetPathName();

    if (fileName.GetLength() == 0)
        return;

    CFileException exception;
    CFile file;

    if (!file.Open(fileName, CFile::modeRead | CFile::shareDenyWrite, &exception))
    {
        std::string msg;
        char buf[256];
        exception.GetErrorMessage(buf, 255);
        AfxFormatString2(msg, IDS_LOAD_CODE_ERR_1, fileName, buf);
        AfxMessageBox(msg);
        return;
    }

    try
    {
        std::string ext = GetFileExt();
        std::string extensions;
        extensions.LoadString(IDS_CODE_EXTENSIONS);
        ext.MakeLower();
        int nPos = extensions.Find(ext);

        switch (nPos >= 0 ? nPos / 4 : -1)
        {
        // Determining the type based on the extension of the file to be written
        case 0: // 65h
        case 1: // hex
            m_nPos = 0;
            break;

        case 2: // 65m
        case 3: // s9
            m_nPos = 1;
            break;

        case 4: // bin
        case 5: // 65b
            m_nPos = 2;
            break;

        case 6: // 65p
            m_nPos = 3;
            break;

        case 7: // com
            m_nPos = 4;
            break;

        default:
            m_nPos = m_ofn.nFilterIndex - 1;
            if (m_ofn.nFilterIndex == 6) // All files (*.*) ?
                m_nPos = 2;
            break;
        } // If unrecognized extension, we use the type selected in the dialog box

        CLoadCodeOptions dlg;
        
        if (m_nPos == 2 && dlg.DoModal() != IDOK) // Providing the address if the type is 'binary file'
            return;

        ASSERT(dlg.m_uStart <= 0xFFFF);

        CArchive archive(&file, CArchive::load, 1024 * 8);

        if (wxGetApp().m_global.m_bProc6502 == 2) // 65816
            wxGetApp().m_global.LoadCode(archive, dlg.m_uStart, 0xFFFFFF, m_nPos, dlg.m_bClearMem ? dlg.m_uFill : -1);
        else
            wxGetApp().m_global.LoadCode(archive, dlg.m_uStart, 0xFFFF, m_nPos, dlg.m_bClearMem ? dlg.m_uFill : -1);

        archive.Close();

        if (wxGetApp().m_global.m_bProc6502 != 2) // 1.3.3  65816 - deassembler not ready yet
            wxGetApp().m_global.CreateDeasm();
    }
    catch (CException *exception)
    {
        std::string msg;
        char buf[256];
        exception->GetErrorMessage(buf, 255);
        AfxFormatString2(msg, IDS_LOAD_CODE_ERR_2, fileName, buf);
        AfxMessageBox(msg);
        exception->Delete();
        return;
    }
    catch (CIntelHex::CIntelHexException exception)
    {
        std::string msg;
        char buf[256];
        exception.GetErrorMessage(buf, sizeof(buf));
        //    AfxFormatString2(msg, IDS_SAVE_CODE_ERR_2, fileName, buf);
        AfxMessageBox(buf);
        return;
    }
#endif
}

/*
void CLoadCode::OnTypeChange()
{
  CWnd *pWnd= GetParent();
  if (pWnd == NULL)
    return;
  m_nInitPos = m_nPos = pWnd->SendDlgItemMessage(cmb1,CB_GETCURSEL);
}
*/
