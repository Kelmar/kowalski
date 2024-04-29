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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "StdAfx.h"

#include "MainFrm.h"
#include "DialAsmStat.h"
#include "Options.h"
#include "DialEditBrkp.h"
#include "SaveCode.h"
#include "LoadCode.h"
#include "Deasm6502View.h"
#include "Splash.h"
#include "6502View.h"
#include "6502Doc.h"
#include "IntRequestGeneratorDlg.h"
#include "editcmd.h"
#include "DockBarEx.h"

#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
// Windows MainFrame, RegisterBar, IOWindow, MemoryView, ZeroPageView, IdentInfo
//const HWND * /*const*/ CMainFrame::m_hWindows[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

WNDPROC CMainFrame::m_pfnOldProc;

//wxBitmap CMainFrame::m_bmpCode; // pictures in StatusBar
//wxBitmap CMainFrame::m_bmpDebug;

//-----------------------------------------------------------------------------

const char CMainFrame::REG_ENTRY_LAYOUT[] = "Bars\\Layout";
const char CMainFrame::REG_ENTRY_MAINFRM[] = "MainFrame";
const char CMainFrame::REG_POSX[] = "XPos";
const char CMainFrame::REG_POSY[] = "YPos";
const char CMainFrame::REG_SIZX[] = "Width";
const char CMainFrame::REG_SIZY[] =  "Height";
const char CMainFrame::REG_STATE[] = "Maximize";

std::string CMainFrame::ProjName = "";

void CMainFrame::ConfigSettings(bool load)
{
#if 0
    static const LOGFONT LogFont =
    {
        -12,// LONG lfHeight;
        0,	// LONG lfWidth;
        0,	// LONG lfEscapement;
        0,	// LONG lfOrientation;
        0,	// LONG lfWeight;
        0,	// BYTE lfItalic;
        0,	// BYTE lfUnderline;
        0,	// BYTE lfStrikeOut;
        0,	// BYTE lfCharSet;
        0,	// BYTE lfOutPrecision;
        0,	// BYTE lfClipPrecision;
        0,	// BYTE lfQuality;
        FIXED_PITCH,	// BYTE lfPitchAndFamily;
        "Fixedsys"	// CHAR lfFaceName[LF_FACESIZE];
    };
#endif

    static const char ENTRY_SYM[] = "Simulator";
    static const char SYM_FIN[] = "Finish";
    static const char SYM_IO_ADDR[] = "IOAddress";
    static const char SYM_IO_ENABLED[] = "IOEnabled";
    static const char SYM_PROT_MEM[] = "ProtectMem";
    static const char SYM_PROT_MEM_FROM[] = "ProtectMemFrom";
    static const char SYM_PROT_MEM_TO[] = "ProtectMemTo";
    static const char SYM_WND_X[] = "TerminalXPos";
    static const char SYM_WND_Y[] = "TerminalYPos";
    static const char SYM_WND_W[] = "TerminalWidth";
    static const char SYM_WND_H[] = "TerminalHeight";

    static const char ENTRY_ASM[] = "Assembler";
    static const char ASM_CASE[] = "CaseSensitive";
    static const char ASM_GEN_LST[] = "GenerateListing";
    static const char ASM_LST_FILE[] = "ListingFile";
    static const char ASM_GEN_BYTE[] = "GenerateBRKExtraByte";
    static const char ASM_BRK_BYTE[] = "BRKExtraByte";

    static const char ENTRY_EDIT[] = "Editor";
    static const char EDIT_FONT[] = "Font";
    static const char EDIT_TAB_STEP[] = "TabStep";
    static const char EDIT_AUTO_INDENT[] = "AutoIndent";
    static const char EDIT_SYNTAX_CHECK[] = "SyntaxChecking";
    static const char EDIT_CAPITALS[] = "AutoUppercase";
    static const char EDIT_FILENEW[] = "FileNew";

    static const char ENTRY_DEASM[] = "Deassembler";
    static const char DEASM_ADDR_COLOR[] = "AddrColor";
    static const char DEASM_CODE_COLOR[] = "CodeColor";
    static const char DEASM_SHOW_CODE[] = "ShowCode";

    static const char ENTRY_GEN[] = "General";
    static const char GEN_PROC[] = "ProcType";
    static const char GEN_HELP[] = "HelpType";       //^^ Help
    static const char GEN_BUS_WIDTH[] = "BusWidth";
    static const char GEN_PTR[] = "PointerColor";
    static const char GEN_BRKP[] = "BreakpointColor";
    static const char GEN_ERR[] = "ErrorColor";

    static const char ENTRY_VIEW[] = "View";
    static const char VIEW_IDENTS_X[] = "IdentsXPos";
    static const char VIEW_IDENTS_Y[] = "IdentsYPos";
    static const char VIEW_IDENTS_W[] = "IdentsW";
    static const char VIEW_IDENTS_H[] = "IdentsH";
    static const char VIEW_MEMO_X[] = "MemoryXPos";
    static const char VIEW_MEMO_Y[] = "MemoryYPos";
    static const char VIEW_MEMO_W[] = "MemoryW";
    static const char VIEW_MEMO_H[] = "MemoryH";
    static const char VIEW_MEMO_HID[] = "MemoryWndHidden";
    static const char VIEW_ZMEM_X[] = "ZeroPageXPos";
    static const char VIEW_ZMEM_Y[] = "ZeroPageYPos";
    static const char VIEW_ZMEM_W[] = "ZeroPageW";
    static const char VIEW_ZMEM_H[] = "ZeroPageH";
    static const char VIEW_ZMEM_HID[] = "ZeroPageWndHidden";
    static const char VIEW_STACK_X[] = "StackXPos";
    static const char VIEW_STACK_Y[] = "StackYPos";
    static const char VIEW_STACK_W[] = "StackW";
    static const char VIEW_STACK_H[] = "StackH";
    static const char VIEW_STACK_HID[] = "StackWndHidden";
    static const char VIEW_LOG_X[] = "LogWndXPos";
    static const char VIEW_LOG_Y[] = "LogWndYPos";
    static const char VIEW_LOG_W[] = "LogWndW";
    static const char VIEW_LOG_H[] = "LogWndH";
    static const char VIEW_LOG_HID[] = "LogWndHidden";
    static const char VIEW_IO_HID[] = "IOWndHidden";
    static const char VIEW_REGS_HID[] = "RegsWndHidden";
    static const char VIEW_FONT_ED[] = "FontEditor";
    static const char VIEW_ED_TCOL[] = "EditorTextColor";
    static const char VIEW_ED_BCOL[] = "EditorBkgndColor";
    static const char VIEW_FONT_SYM[] = "FontIOSymWnd";
    static const char VIEW_SYM_TCOL[] = "IOSymWndTextColor";
    static const char VIEW_SYM_BCOL[] = "IOSymWndBkgndColor";
    static const char VIEW_FONT_DEASM[] = "FontDeasm";
    static const char VIEW_DEASM_TCOL[] = "DeasmInstrColor";
    static const char VIEW_DEASM_BCOL[] = "DeasmBkgndColor";
    static const char VIEW_FONT_MEMO[] = "FontMemory";
    static const char VIEW_MEMO_TCOL[] = "MemoryTextColor";
    static const char VIEW_MEMO_BCOL[] = "MemoryBkgndColor";
    static const char VIEW_FONT_ZERO[] = "FontZeroPage";
    static const char VIEW_ZERO_TCOL[] = "ZeroPageTextColor";
    static const char VIEW_ZERO_BCOL[] = "ZeroPageBkgndColor";
    static const char VIEW_FONT_STACK[] = "FontStack";
    static const char VIEW_STACK_TCOL[] = "StackTextColor";
    static const char VIEW_STACK_BCOL[] = "StackBkgndColor";

    static const char * const idents[] =
    { VIEW_FONT_ED, VIEW_FONT_SYM, VIEW_FONT_DEASM, VIEW_FONT_MEMO, VIEW_FONT_ZERO, VIEW_FONT_STACK };
    static const char * const tcolors[] =
    { VIEW_ED_TCOL, VIEW_SYM_TCOL, VIEW_DEASM_TCOL, VIEW_MEMO_TCOL, VIEW_ZERO_TCOL, VIEW_STACK_TCOL };
    static const char * const bcolors[] =
    { VIEW_ED_BCOL, VIEW_SYM_BCOL, VIEW_DEASM_BCOL, VIEW_MEMO_BCOL, VIEW_ZERO_BCOL, VIEW_STACK_BCOL };
    static const char * const syntax_colors[] =
    { "ColorInstruction", "ColorDirective", "ColorComment", "ColorNumber", "ColorString", "ColorSelection", 0 };
    static const char * const syntax_font[] =
    { "FontInstruction", "FontDirective", "FontComment", "FontNumber", "FontString", 0 };

    CWinApp *pApp = AfxGetApp();

    if (load)		// reading?
    {
        theApp.m_global.SetSymFinish( (CAsm::Finish)(pApp->GetProfileInt(ENTRY_SYM, SYM_FIN, 0)) );
        CSym6502::io_addr    = pApp->GetProfileInt(ENTRY_SYM, SYM_IO_ADDR, 0xE000);
        CSym6502::io_enabled = pApp->GetProfileInt(ENTRY_SYM, SYM_IO_ENABLED, 1);
        CSym6502::s_bWriteProtectArea = !!pApp->GetProfileInt(ENTRY_SYM, SYM_PROT_MEM, 0);
        CSym6502::s_uProtectFromAddr = pApp->GetProfileInt(ENTRY_SYM, SYM_PROT_MEM_FROM, 0xC000);
        CSym6502::s_uProtectToAddr = pApp->GetProfileInt(ENTRY_SYM, SYM_PROT_MEM_TO, 0xCFFF);
        wxPoint pos;
        pos.x = pApp->GetProfileInt(ENTRY_SYM, SYM_WND_X, 200);
        pos.y = pApp->GetProfileInt(ENTRY_SYM, SYM_WND_Y, 200);
        m_IOWindow.SetWndPos(pos);
        m_IOWindow.SetSize( pApp->GetProfileInt(ENTRY_SYM, SYM_WND_W, 40),
                            pApp->GetProfileInt(ENTRY_SYM, SYM_WND_H, 12) );
//    m_IOWindow.SetColors( (COLORREF)pApp->GetProfileInt(ENTRY_SYM, SYM_WND_TEXT_COL, int(RGB(0, 0, 0))),
//      pApp->GetProfileInt(ENTRY_SYM, SYM_WND_BK_COL, int(RGB(255,255,255))) );

        CDeasm6502View::m_rgbAddress   = COLORREF(pApp->GetProfileInt(ENTRY_DEASM, DEASM_ADDR_COLOR, (int)RGB(127,127,127)));
        CDeasm6502View::m_rgbCode      = COLORREF(pApp->GetProfileInt(ENTRY_DEASM, DEASM_CODE_COLOR, (int)RGB(191,191,191)));
        //    CDeasm6502View::m_rgbInstr   = COLORREF(pApp->GetProfileInt(ENTRY_DEASM, DEASM_INSTR_COLOR, (int)RGB(0,0,0)));
        CDeasm6502View::m_bDrawCode    = pApp->GetProfileInt(ENTRY_DEASM, DEASM_SHOW_CODE, 1);

        CMarks::m_rgbPointer           = COLORREF(pApp->GetProfileInt(ENTRY_GEN, GEN_PTR, (int)RGB(255,255,0)));
        CMarks::m_rgbBreakpoint        = COLORREF(pApp->GetProfileInt(ENTRY_GEN, GEN_BRKP, (int)RGB(0,0,160)));
        CMarks::m_rgbError             = COLORREF(pApp->GetProfileInt(ENTRY_GEN, GEN_ERR, (int)RGB(255,0,0)));
        theApp.m_global.SetProcType( pApp->GetProfileInt(ENTRY_GEN, GEN_PROC, 1));
        theApp.m_global.SetHelpType( pApp->GetProfileInt(ENTRY_GEN, GEN_HELP, 1));    //^^ Help

        CSym6502::bus_width            = pApp->GetProfileInt(ENTRY_GEN, GEN_BUS_WIDTH, 16);
        theApp.m_global.m_bGenerateListing = (bool) pApp->GetProfileInt(ENTRY_ASM, ASM_GEN_LST, false);
        theApp.m_global.m_strListingFile   = pApp->GetProfileString(ENTRY_ASM, ASM_LST_FILE, NULL);
        CAsm6502::generateBRKExtraByte = (bool) pApp->GetProfileInt(ENTRY_ASM, ASM_GEN_BYTE, 1);
        CAsm6502::BRKExtraByte         = (UINT8)pApp->GetProfileInt(ENTRY_ASM, ASM_BRK_BYTE, 0);

        CSrc6502View::m_nTabStep       = pApp->GetProfileInt(ENTRY_EDIT, EDIT_TAB_STEP, 8);
        CSrc6502View::m_bAutoIndent    = pApp->GetProfileInt(ENTRY_EDIT, EDIT_AUTO_INDENT, 1);
        CSrc6502View::m_bAutoSyntax    = pApp->GetProfileInt(ENTRY_EDIT, EDIT_SYNTAX_CHECK, 1);
        CSrc6502View::m_bAutoUppercase = pApp->GetProfileInt(ENTRY_EDIT, EDIT_CAPITALS, 1);
        C6502App::m_bFileNew           = pApp->GetProfileInt(ENTRY_EDIT, EDIT_FILENEW, 0);

        CIdentInfo::m_WndRect.left     = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_X, 200);
        CIdentInfo::m_WndRect.top      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_Y, 200);
        CIdentInfo::m_WndRect.right    = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_W, 400);
        CIdentInfo::m_WndRect.bottom   = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_H, 400);
        CIdentInfo::m_WndRect.bottom  += CIdentInfo::m_WndRect.top;
        CIdentInfo::m_WndRect.right   += CIdentInfo::m_WndRect.left;

        m_Memory.m_WndRect.left        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_X, 220);
        m_Memory.m_WndRect.top         = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_Y, 220);
        m_Memory.m_WndRect.right       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_W, 400);
        m_Memory.m_WndRect.bottom      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_H, 400);
        m_Memory.m_WndRect.bottom     += m_Memory.m_WndRect.top;
        m_Memory.m_WndRect.right      += m_Memory.m_WndRect.left;

        m_ZeroPage.m_WndRect.left      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_X, 240);
        m_ZeroPage.m_WndRect.top       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_Y, 240);
        m_ZeroPage.m_WndRect.right     = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_W, 400);
        m_ZeroPage.m_WndRect.bottom    = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_H, 400);
        m_ZeroPage.m_WndRect.bottom   += m_ZeroPage.m_WndRect.top;
        m_ZeroPage.m_WndRect.right    += m_ZeroPage.m_WndRect.left;

        m_Stack.m_WndRect.left         = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_X, 260);
        m_Stack.m_WndRect.top          = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_Y, 260);
        m_Stack.m_WndRect.right        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_W, 300);
        m_Stack.m_WndRect.bottom       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_H, 400);
        m_Stack.m_WndRect.bottom      += m_Stack.m_WndRect.top;
        m_Stack.m_WndRect.right       += m_Stack.m_WndRect.left;

        m_wndLog.m_WndRect.left        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_X, 280);
        m_wndLog.m_WndRect.top         = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_Y, 280);
        m_wndLog.m_WndRect.right       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_W, 300);
        m_wndLog.m_WndRect.bottom      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_H, 400);
        m_wndLog.m_WndRect.right       += m_wndLog.m_WndRect.left;
        m_wndLog.m_WndRect.bottom      += m_wndLog.m_WndRect.top;

        CIOWindow::m_bHidden           = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IO_HID, 0);
        CRegisterBar::m_bHidden        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_REGS_HID, 0);

        m_Memory.m_bHidden             = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_HID, false) != 0;
        m_ZeroPage.m_bHidden           = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_HID, false) != 0;
        m_Stack.m_bHidden              = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_HID, false) != 0;
        m_wndLog.m_bHidden             = !!pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_HID, false) != 0;

//    CMemoryInfo::m_bHidden = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_HID, false) != 0;

        for (int i=0; fonts[i]; i++)	// reading info about fonts in the program
        {
            *fonts[i] = LogFont;	// default font
            LPBYTE ptr= NULL;
            UINT bytes= sizeof *fonts[i];
            pApp->GetProfileBinary(ENTRY_VIEW, idents[i], &ptr, &bytes);
            if (ptr)
            {
                if (bytes == sizeof *fonts[i])
                    memcpy(fonts[i], ptr, sizeof *fonts[i]);
                delete [] ptr;
            }
            static const COLORREF defaults[]=	      // default background colors for windows
            {
                // VIEW_ED_BCOL, VIEW_SYM_BCOL, VIEW_DEASM_BCOL, VIEW_MEMO_BCOL, VIEW_ZERO_BCOL
                RGB(255,255,255), RGB(255,255,224), RGB(255,255,255),
                RGB(240,255,240), RGB(255,240,240), RGB(255,255,255), RGB(240,240,240)
            };
            *text_color[i] = COLORREF(pApp->GetProfileInt(ENTRY_VIEW, tcolors[i], RGB(0,0,0)));
            *bkgnd_color[i] = COLORREF(pApp->GetProfileInt(ENTRY_VIEW, bcolors[i], defaults[i]));
        }

        for (int clr= 0; syntax_colors[clr]; ++clr)
            *color_syntax[clr] = pApp->GetProfileInt(ENTRY_VIEW, syntax_colors[clr], *color_syntax[clr]);

        for (int style= 0; syntax_font[style]; ++style)
            *syntax_font_style[style] = pApp->GetProfileInt(ENTRY_VIEW, syntax_font[style], *syntax_font_style[style]);

        //    pApp->GetProfileInt(ENTRY_ASM,ASM_CASE,1);

    }
    else			// record
    {
        pApp->WriteProfileInt(ENTRY_SYM, SYM_FIN, theApp.m_global.GetSymFinish());
        pApp->WriteProfileInt(ENTRY_SYM, SYM_IO_ADDR, CSym6502::io_addr);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_IO_ENABLED, CSym6502::io_enabled);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_PROT_MEM, CSym6502::s_bWriteProtectArea);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_PROT_MEM_FROM, CSym6502::s_uProtectFromAddr);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_PROT_MEM_TO, CSym6502::s_uProtectToAddr);
        wxPoint pos= m_IOWindow.GetWndPos();
        pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_X, pos.x);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_Y, pos.y);
        int w, h;
        m_IOWindow.GetSize(w, h);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_W, w);
        pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_H, h);
        COLORREF txt, bk;
        m_IOWindow.GetColors(txt, bk);
//    pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_TEXT_COL, (int)txt);
//    pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_BK_COL, (int)bk);
//    pApp->WriteProfileBinary(ENTRY_SYM, SYM_WND_FONT,  LPBYTE(&m_IOWindow.m_LogFont), UINT(sizeof m_IOWindow.m_LogFont));

        pApp->WriteProfileInt(ENTRY_DEASM, DEASM_ADDR_COLOR, (int)CDeasm6502View::m_rgbAddress);
        pApp->WriteProfileInt(ENTRY_DEASM, DEASM_CODE_COLOR, (int)CDeasm6502View::m_rgbCode);
        //    pApp->WriteProfileInt(ENTRY_DEASM, DEASM_INSTR_COLOR, (int)CDeasm6502View::m_rgbInstr);
        pApp->WriteProfileInt(ENTRY_DEASM, DEASM_SHOW_CODE, (int)CDeasm6502View::m_bDrawCode);

        pApp->WriteProfileInt(ENTRY_GEN, GEN_PROC, (int)theApp.m_global.GetProcType());
        pApp->WriteProfileInt(ENTRY_GEN, GEN_HELP, (int)theApp.m_global.GetHelpType());    //^^ Help
        pApp->WriteProfileInt(ENTRY_GEN, GEN_BUS_WIDTH, CSym6502::bus_width);
        pApp->WriteProfileInt(ENTRY_GEN, GEN_PTR, (int)CMarks::m_rgbPointer);
        pApp->WriteProfileInt(ENTRY_GEN, GEN_BRKP, (int)CMarks::m_rgbBreakpoint);
        pApp->WriteProfileInt(ENTRY_GEN, GEN_ERR, (int)CMarks::m_rgbError);

//    pApp->WriteProfileBinary(ENTRY_EDIT, EDIT_FONT,  LPBYTE(&CSrc6502View::m_LogFont), UINT(sizeof CSrc6502View::m_LogFont));
        pApp->WriteProfileInt(ENTRY_EDIT, EDIT_TAB_STEP, CSrc6502View::m_nTabStep);
        pApp->WriteProfileInt(ENTRY_EDIT, EDIT_AUTO_INDENT, CSrc6502View::m_bAutoIndent);
        pApp->WriteProfileInt(ENTRY_EDIT, EDIT_SYNTAX_CHECK, CSrc6502View::m_bAutoSyntax);
        pApp->WriteProfileInt(ENTRY_EDIT, EDIT_CAPITALS, CSrc6502View::m_bAutoUppercase);
        pApp->WriteProfileInt(ENTRY_EDIT, EDIT_FILENEW, C6502App::m_bFileNew);

        pApp->WriteProfileInt(ENTRY_ASM, ASM_CASE, 1);
        pApp->WriteProfileInt(ENTRY_ASM, ASM_GEN_LST, theApp.m_global.m_bGenerateListing);
        pApp->WriteProfileString(ENTRY_ASM, ASM_LST_FILE, theApp.m_global.m_strListingFile);
        pApp->WriteProfileInt(ENTRY_ASM, ASM_GEN_BYTE, CAsm6502::generateBRKExtraByte);
        pApp->WriteProfileInt(ENTRY_ASM, ASM_BRK_BYTE, CAsm6502::BRKExtraByte);

        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_X, CIdentInfo::m_WndRect.left);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_Y, CIdentInfo::m_WndRect.top);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_W, CIdentInfo::m_WndRect.Width());
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_H, CIdentInfo::m_WndRect.Height());

        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_X, m_Memory.m_WndRect.left);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_Y, m_Memory.m_WndRect.top);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_W, m_Memory.m_WndRect.Width());
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_H, m_Memory.m_WndRect.Height());

        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_X, m_ZeroPage.m_WndRect.left);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_Y, m_ZeroPage.m_WndRect.top);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_W, m_ZeroPage.m_WndRect.Width());
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_H, m_ZeroPage.m_WndRect.Height());

        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_X, m_Stack.m_WndRect.left);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_Y, m_Stack.m_WndRect.top);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_W, m_Stack.m_WndRect.Width());
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_H, m_Stack.m_WndRect.Height());

        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_X, m_wndLog.m_WndRect.left);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_Y, m_wndLog.m_WndRect.top);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_W, m_wndLog.m_WndRect.Width());
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_H, m_wndLog.m_WndRect.Height());

        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IO_HID,    CIOWindow::m_bHidden);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_REGS_HID,  CRegisterBar::m_bHidden);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_HID,  m_Memory.m_bHidden);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_HID,  m_ZeroPage.m_bHidden);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_HID, m_Stack.m_bHidden);
        pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_HID,   m_wndLog.m_bHidden);

        for (int i= 0; fonts[i]; i++)
        {
            pApp->WriteProfileBinary(ENTRY_VIEW, idents[i], LPBYTE(fonts[i]), UINT(sizeof *fonts[i]));
            pApp->WriteProfileInt(ENTRY_VIEW, tcolors[i], int(*text_color[i]));
            pApp->WriteProfileInt(ENTRY_VIEW, bcolors[i], int(*bkgnd_color[i]));
        }

        for (int clr= 0; syntax_colors[clr]; ++clr)
            pApp->WriteProfileInt(ENTRY_VIEW, syntax_colors[clr], int(*color_syntax[clr]));

        for (int style= 0; syntax_font[style]; ++style)
            pApp->WriteProfileInt(ENTRY_VIEW, syntax_font[style], *syntax_font_style[style]);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

#if 0
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_COMMAND(ID_SYM_ASSEMBLE, OnAssemble)
    ON_UPDATE_COMMAND_UI(ID_SYM_ASSEMBLE, OnUpdateAssemble)
    ON_UPDATE_COMMAND_UI(ID_SYM_DEBUG, OnUpdateSymDebug)
    ON_COMMAND(ID_SYM_DEBUG, OnSymDebug)
    ON_COMMAND(ID_SYM_STEP_INTO, OnSymStepInto)
    ON_UPDATE_COMMAND_UI(ID_SYM_STEP_INTO, OnUpdateSymStepInto)
    ON_COMMAND(ID_SYM_SKIP_INSTR, OnSymSkipInstr)
    ON_UPDATE_COMMAND_UI(ID_SYM_SKIP_INSTR, OnUpdateSymSkipInstr)
    ON_COMMAND(ID_SYM_BREAKPOINT, OnSymBreakpoint)
    ON_UPDATE_COMMAND_UI(ID_SYM_BREAKPOINT, OnUpdateSymBreakpoint)
    ON_COMMAND(ID_SYM_BREAK, OnSymBreak)
    ON_UPDATE_COMMAND_UI(ID_SYM_BREAK, OnUpdateSymBreak)
    ON_COMMAND(ID_SYM_GO, OnSymGo)
    ON_UPDATE_COMMAND_UI(ID_SYM_GO, OnUpdateSymGo)
    ON_COMMAND(ID_SYM_OPTIONS, OnOptions)
    ON_UPDATE_COMMAND_UI(ID_SYM_OPTIONS, OnUpdateOptions)
    ON_COMMAND(ID_SYM_GO_LINE, OnSymGoToLine)
    ON_UPDATE_COMMAND_UI(ID_SYM_GO_LINE, OnUpdateSymGoToLine)
    ON_COMMAND(ID_SYM_SKIP_TO_LINE, OnSymSkipToLine)
    ON_UPDATE_COMMAND_UI(ID_SYM_SKIP_TO_LINE, OnUpdateSymSkipToLine)
    ON_COMMAND(ID_SYM_GO_RTS, OnSymGoToRts)
    ON_UPDATE_COMMAND_UI(ID_SYM_GO_RTS, OnUpdateSymGoToRts)
    ON_COMMAND(ID_SYM_STEP_OVER, OnSymStepOver)
    ON_UPDATE_COMMAND_UI(ID_SYM_STEP_OVER, OnUpdateSymStepOver)
    ON_COMMAND(ID_SYM_EDIT_BREAKPOINT, OnSymEditBreakpoint)
    ON_UPDATE_COMMAND_UI(ID_SYM_EDIT_BREAKPOINT, OnUpdateSymEditBreakpoint)
    ON_COMMAND(ID_SYM_RESTART, OnSymRestart)
    ON_UPDATE_COMMAND_UI(ID_SYM_RESTART, OnUpdateSymRestart)
    ON_COMMAND(ID_SYM_ANIMATE, OnSymAnimate)
    ON_UPDATE_COMMAND_UI(ID_SYM_ANIMATE, OnUpdateSymAnimate)
    ON_UPDATE_COMMAND_UI(ID_VIEW_REGISTERBAR, OnUpdateIdViewRegisterbar)
    ON_COMMAND(ID_FILE_SAVE_CODE, OnFileSaveCode)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_CODE, OnUpdateFileSaveCode)
    ON_COMMAND(ID_VIEW_DEASM, OnViewDeasm)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DEASM, OnUpdateViewDeasm)
    ON_COMMAND(ID_VIEW_IDENT, OnViewIdents)
    ON_UPDATE_COMMAND_UI(ID_VIEW_IDENT, OnUpdateViewIdents)
    ON_COMMAND(ID_VIEW_MEMORY, OnViewMemory)
    ON_UPDATE_COMMAND_UI(ID_VIEW_MEMORY, OnUpdateViewMemory)
    ON_COMMAND(ID_EDITOR_OPT, OnEditorOpt)
    ON_UPDATE_COMMAND_UI(ID_EDITOR_OPT, OnUpdateEditorOpt)
    ON_COMMAND(ID_VIEW_IO_WINDOW, OnViewIOWindow)
    ON_UPDATE_COMMAND_UI(ID_VIEW_IO_WINDOW, OnUpdateViewIOWindow)
    ON_WM_DESTROY()
    ON_COMMAND(ID_FILE_OPEN_CODE, OnFileLoadCode)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_CODE, OnUpdateFileLoadCode)
    ON_COMMAND(ID_DEASM_OPTIONS, OnDeasmOptions)
    ON_UPDATE_COMMAND_UI(ID_DEASM_OPTIONS, OnUpdateDeasmOptions)
    ON_COMMAND(ID_VIEW_REGISTERBAR, OnViewRegisterWnd)
    ON_WM_SYSCOLORCHANGE()
    ON_UPDATE_COMMAND_UI(ID_VIEW_ZEROPAGEBAR, OnUpdateViewZeropage)
    ON_COMMAND(ID_VIEW_ZEROPAGEBAR, OnViewZeropage)
    ON_UPDATE_COMMAND_UI(ID_MEMORY_OPTIONS, OnUpdateMemoryOptions)
    ON_COMMAND(ID_MEMORY_OPTIONS, OnMemoryOptions)
    ON_WM_TIMER()
    ON_COMMAND(ID_VIEW_STACK, OnViewStack)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STACK, OnUpdateViewStack)
    ON_COMMAND(ID_SYM_GEN_IRQ, OnSymGenIRQ)
    ON_UPDATE_COMMAND_UI(ID_SYM_GEN_IRQ, OnUpdateSymGenIRG)
    ON_COMMAND(ID_SYM_GEN_NMI, OnSymGenNMI)
    ON_UPDATE_COMMAND_UI(ID_SYM_GEN_NMI, OnUpdateSymGenNMI)
    ON_COMMAND(ID_SYM_GEN_RST, OnSymGenReset)
    ON_UPDATE_COMMAND_UI(ID_SYM_GEN_RST, OnUpdateSymGenReset)
    ON_COMMAND(ID_SYM_GEN_INT, OnSymGenIntDlg)
    ON_UPDATE_COMMAND_UI(ID_SYM_GEN_INT, OnUpdateSymGenIntDlg)
    ON_COMMAND(ID_VIEW_LOG, OnViewLog)
    ON_UPDATE_COMMAND_UI(ID_VIEW_LOG, OnUpdateViewLog)
    ON_COMMAND(ID_HELP_DYNAMIC, OnHelpDynamic)
    ON_UPDATE_COMMAND_UI(ID_HELP_DYNAMIC, OnUpdateHelpDynamic)
    ON_COMMAND(ID_HELP_FINDER, OnHtmlHelp)   //% Bug fix 1.2.14.1 - convert to HTML help
    ON_COMMAND(ID_HELP, OnHtmlHelp)          //% Bug fix 1.2.14.1 - convert to HTML help
    ON_COMMAND(ID_DEFAULT_HELP, OnHtmlHelp)  //% Bug fix 1.2.14.1 - convert to HTML help
    ON_MESSAGE(WM_USER+9998, OnUpdateState)
    ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebugger)
    ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebugger)
    ON_MESSAGE(CBroadcast::WM_USER_PROG_MEM_CHANGED, OnChangeCode)
END_MESSAGE_MAP()

#endif

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    0,
    0,
#ifdef USE_CRYSTAL_EDIT
    ID_EDIT_INDICATOR_POSITION,
    ID_INDICATOR_OVR,
#endif
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    // TODO: add member initialization code here
    m_nLastPage = 0;	// the last page called up (tab) in the options box

#if 0
    int i = 0;
    m_hWindows[i++] = &m_hWnd;
    m_hWindows[i++] = &m_wndRegisterBar.m_hWnd;
    m_hWindows[i++] = &m_IOWindow.m_hWnd;
    m_hWindows[i++] = &m_Memory.m_hWnd;
    m_hWindows[i++] = &m_ZeroPage.m_hWnd;
    m_hWindows[i++] = &m_Idents.m_hWnd;
    m_hWindows[i++] = &m_Stack.m_hWnd;
    m_hWindows[i++] = &m_wndLog.m_hWnd;
    m_hWindows[i++] = NULL;
#endif

    m_uTimer = 0;
}

CMainFrame::~CMainFrame()
{
//  if (m_Idents)
//    delete m_Idents;
}

//-----------------------------------------------------------------------------

const uint32_t CMainFrame::dwDockBarMapEx[4][2] =
{
    { AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
    { AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
    { AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
    { AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};

void CMainFrame::EnableDockingEx(DWORD dwDockStyle)
{
    // must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
    ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

    m_pFloatingFrameClass = RUNTIME_CLASS(CMiniDockFrameWnd);
    for (int i = 0; i < 4; i++)
    {
        if (dwDockBarMapEx[i][1] & dwDockStyle & CBRS_ALIGN_ANY)
        {
            CDockBar* pDock = (CDockBar*)GetControlBar(dwDockBarMapEx[i][0]);
            if (pDock == NULL)
            {
                pDock = new CDockBarEx;
                if (!pDock->Create(this,
                                   WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE |
                                   dwDockBarMapEx[i][1], dwDockBarMapEx[i][0]))
                {
                    AfxThrowResourceException();
                }
            }
        }
    }
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndToolBar.Create(this) ||
            !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1; // fail to create
    }

    std::string strName;
    strName.LoadString(IDS_TOOLBAR);
    m_wndToolBar.SetWindowText(strName);

    if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1; // fail to create
    }

    {
        // adding a field to display the current row and column
        UINT uID;
        UINT uStyle;
        int nWidth;
        wxRect rectArea(0,0,0,0);

        if (m_strFormat.LoadString(IDS_ROW_COLUMN))
        {
            std::string str;
            str.Format(m_strFormat,99999,999);
            m_wndStatusBar.GetPaneInfo(1, uID, uStyle, nWidth);
#ifdef USE_CRYSTAL_EDIT
            m_wndStatusBar.SetPaneInfo(1, uID, SBPS_NOBORDERS | SBPS_DISABLED, 1);
#else
            CClientDC dc(&m_wndStatusBar);
            dc.SelectObject(m_wndStatusBar.GetFont());
            dc.DrawText(str, -1, rectArea, DT_SINGLELINE | DT_CALCRECT);
            m_wndStatusBar.SetPaneInfo(1, uID, uStyle, rectArea.Width());
#endif
        }

        m_wndStatusBar.GetPaneInfo(2, uID, uStyle, nWidth);
        m_wndStatusBar.SetPaneInfo(2, uID, uStyle, 16);	// szeroko�� obrazka

        m_bmpCode.LoadMappedBitmap(IDB_CODE);
        m_bmpDebug.LoadMappedBitmap(IDB_DEBUG);

        m_pfnOldProc = (WNDPROC)::SetWindowLong(m_wndStatusBar.m_hWnd,GWL_WNDPROC,(LONG)StatusBarWndProc);
    }

    // TODO: Remove this if you don't want tool tips or a resizeable toolbar
    m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    EnableDockingEx(CBRS_ALIGN_ANY); //CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

    // TODO: Delete these three lines if you don't want the toolbar to be dockable
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);


    // TODO: Change the value of ID_VIEW_REGISTERBAR to an appropriate value:
    //   1. Open the file resource.h
    //   2. Find the definition for the symbol ID_VIEW_REGISTERBAR
    //   3. Change the value of the symbol. Use a value in the range
    //      0xE804 to 0xE81A that is not already used by another symbol

    // CG: The following block was inserted by the 'Dialog Bar' component
    {
        // Initialize dialog bar m_wndRegisterBar
        if (!m_wndRegisterBar.Create(this,
                                     CBRS_RIGHT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE | CBRS_SIZE_FIXED, ID_VIEW_REGISTERBAR))
        {
            TRACE0("Failed to create dialog bar m_wndRegisterBar\n");
            return -1;		// fail to create
        }
        // m_wndRegisterBar.SetBarStyle(CBRS_BORDER_TOP|CBRS_BORDER_BOTTOM|CBRS_BORDER_LEFT|CBRS_BORDER_RIGHT);
        m_wndRegisterBar.EnableDocking(0); //CBRS_ALIGN_ANY);
        DockControlBar(&m_wndRegisterBar);
        FloatControlBar(&m_wndRegisterBar, wxPoint(100, 100));
//		EnableDocking(0); //CBRS_ALIGN_ANY);
        //    DockControlBar(&m_wndRegisterBar);
        //    FloatControlBar(&m_wndRegisterBar,wxPoint(10,10));
    }

    if (!m_wndHelpBar.Create(this, AFX_IDW_CONTROLBAR_LAST))
    {
        TRACE0("Failed to create help bar\n");
        return -1;		// fail to create
    }
    m_wndHelpBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
    m_wndHelpBar.SetWindowText("Dynamic Help");
    DockControlBar(&m_wndHelpBar);


//	LoadBarState(REG_ENTRY_LAYOUT);
    bool bEmptyInfo= true;
    {
        CDockState state;
        state.LoadState(REG_ENTRY_LAYOUT);

        for (int i=0; i < state.m_arrBarInfo.GetSize(); i++)
        {
            CControlBarInfo* pInfo= (CControlBarInfo*)(state.m_arrBarInfo[i]);
            if (pInfo->m_nBarID == ID_VIEW_REGISTERBAR)
                pInfo->m_bVisible = false;	// registerBar always hidden after application start
        }
        bEmptyInfo = state.m_arrBarInfo.GetSize() == 0;
        SetDockState(state);
    }
    if (bEmptyInfo)		// first launch of the application in the system?
    {
        wxPoint point(32,32);	// original position
        CMiniDockFrameWnd* pDockFrame = CreateFloatingFrame(CBRS_ALIGN_TOP);
        ASSERT(pDockFrame != NULL);
        pDockFrame->SetWindowPos(NULL, point.x, point.y, 0, 0,
                                 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        if (pDockFrame->m_hWndOwner == NULL)
            pDockFrame->m_hWndOwner = m_wndRegisterBar.m_hWnd;

        CDockBar* pDockBar = (CDockBar*)pDockFrame->GetDlgItem(AFX_IDW_DOCKBAR_FLOAT);
        ASSERT(pDockBar != NULL);
        ASSERT_KINDOF(CDockBar, pDockBar);

        ASSERT(m_wndRegisterBar.m_pDockSite == this);
        // if this assertion occurred it is because the parent of pBar was not
        //  initially this CFrameWnd when pBar's OnCreate was called
        // (this control bar should have been created with a different
        //  parent initially)

        pDockBar->DockControlBar(&m_wndRegisterBar);
        pDockFrame->RecalcLayout(true);
        pDockFrame->ShowWindow(SW_HIDE);
        //    pDockFrame->UpdateWindow();
        m_wndRegisterBar.ModifyStyle(WS_VISIBLE,0);
    }

    for (int i =0; cfonts[i]; i++)	// creating fonts
    {
        //    cfonts[i]->DeleteObject();
        cfonts[i]->CreateFontIndirect(fonts[i]);
    }

    m_Memory.Create(theApp.m_global.GetMem(), theApp.m_global.GetStartAddr(), CMemoryInfo::VIEW_MEMORY);
    m_ZeroPage.Create(theApp.m_global.GetMem(), 0x00, CMemoryInfo::VIEW_ZEROPAGE);
    m_Stack.Create(theApp.m_global.GetMem(), 0x00, CMemoryInfo::VIEW_STACK);
    m_wndLog.Create();

    return 0;
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK CMainFrame::StatusBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWnd *pWnd= FromHandlePermanent(hWnd);
//  ASSERT (pWnd->IsKindOf(RUNTIME_CLASS(CStatusBar)));

    switch (msg)
    {
    case WM_PAINT:
    {
        LRESULT ret= (*CMainFrame::m_pfnOldProc)(hWnd,msg,wParam,lParam);
        if (ret == 0)
        {
            bool bCode;
            if (theApp.m_global.IsDebugger())		// jest dzia�aj�cy debugger ?
                bCode = false;
            else if (theApp.m_global.IsCodePresent())	// jest kod programu?
                bCode = true;
            else
                return ret;				// ani kodu programu, ani debuggera
            wxRect rect;
            (*m_pfnOldProc)(hWnd,SB_GETRECT,2,(LPARAM)(RECT *)rect);	// miejsce na obrazek - wymiary
            int borders[3];
            (*m_pfnOldProc)(hWnd,SB_GETBORDERS,0,(LPARAM)borders);		// grubo�� obw�dki
            rect.DeflateRect(borders[0]+1,borders[1]-1);
            CClientDC dc(pWnd);
            if (dc)
            {
                CDC memDC;
                memDC.CreateCompatibleDC(&dc);
                if (memDC)
                {
                    CBitmap *pOldBmp= memDC.SelectObject(bCode ? &m_bmpCode : &m_bmpDebug);
                    dc.BitBlt(rect.left+2, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
                    memDC.SelectObject(pOldBmp);
                }
            }
        }
        return ret;
    }

    default:
        return (*CMainFrame::m_pfnOldProc)(hWnd,msg,wParam,lParam);
    }
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CMainFrame::OnStartDebugger(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    RECT rect;
    m_wndStatusBar.SendMessage(SB_GETRECT,2,(LPARAM)&rect);	// miejsce na obrazek - wymiary
    m_wndStatusBar.InvalidateRect(&rect);		// obrazek pch�y do przerysowania
    return 0;
}


afx_msg LRESULT CMainFrame::OnExitDebugger(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    RECT rect;
    m_wndStatusBar.SendMessage(SB_GETRECT,2,(LPARAM)&rect);	// miejsce na obrazek - wymiary
    m_wndStatusBar.InvalidateRect(&rect);		// obrazek pch�y do przerysowania
    return 0;
}


afx_msg LRESULT CMainFrame::OnChangeCode(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    RECT rect;
    m_wndStatusBar.SendMessage(SB_GETRECT,2,(LPARAM)&rect);	// miejsce na obrazek - wymiary
    m_wndStatusBar.InvalidateRect(&rect);		// obrazek pch�y do przerysowania
    return 0;
}

//-----------------------------------------------------------------------------

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    CWinApp *pApp = AfxGetApp();

    wxRect desk;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, desk, 0);

    cs.x = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_POSX, 50);
    cs.y = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_POSY, 50);
    cs.cx = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_SIZX, desk.Width() - 100);
    cs.cy = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_SIZY, desk.Height() - 100);

    // prevent from appearing outside desk area
    if (cs.x < desk.left)
        cs.x = desk.left;
    if (cs.y < desk.top)
        cs.y = desk.top;
    if (cs.x + cs.cx > desk.right)
        cs.x = desk.right - min(cs.cx, desk.Width());
    if (cs.y + cs.cy > desk.bottom)
        cs.y = desk.bottom - min(cs.cy, desk.Height());

    if (pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_STATE, 0))	// maximize?
        C6502App::m_bMaximize = true;
    //  C6502App::m_bFileNew = pApp->GetProfileInt(REG_ENTRY_MAINFRM,REG_FILENEW,1);	// new file
    ConfigSettings(true);		// odczyt ustawie�

    return CMDIFrameWnd::PreCreateWindow(cs);
}

//-----------------------------------------------------------------------------
/*
HMENU CMainFrame::GetWindowMenuPopup(HMENU hMenuBar)
  // find which popup is the "Window" menu
{
  if (hMenuBar == NULL)
    return NULL;

  ASSERT(::IsMenu(hMenuBar));

  int iItem = ::GetMenuItemCount(hMenuBar);
  while (iItem--)
  {
    HMENU hMenuPop = ::GetSubMenu(hMenuBar, iItem);
    if (hMenuPop != NULL)
    {
      int iItemMax = ::GetMenuItemCount(hMenuPop);
      for (int iItemPop = 0; iItemPop < iItemMax; iItemPop++)
      {
	UINT nID = GetMenuItemID(hMenuPop, iItemPop);
	if (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST)
	  return hMenuPop;
        HMENU hMenuSubPop = ::GetSubMenu(hMenuPop, iItemPop);
        if (hMenuSubPop != NULL)
	{
	  int iItemSubMax = ::GetMenuItemCount(hMenuSubPop);
	  for (int iItemSubPop = 0; iItemSubPop < iItemSubMax; iItemSubPop++)
	  {
	    UINT nID = GetMenuItemID(hMenuSubPop, iItemSubPop);
	    if (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST)
	      return hMenuSubPop;
	  }
	}
      }
    }
  }

  // no default menu found
  TRACE0("Warning: GetWindowMenuPopup failed!\n");
  return NULL;
}
*/

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose()
{
    if (theApp.m_global.IsDebugger() && theApp.m_global.IsProgramRunning())
    {
        if (m_IOWindow.IsWaiting())
        {
            m_IOWindow.ExitModalLoop();
            return;
        }
        if (AfxMessageBox(IDS_MAINFRM_PROG_RUNNING,MB_YESNO) == IDYES)
        {
            theApp.m_global.AbortProg();
            if (m_IOWindow.m_hWnd != 0)
                m_IOWindow.SendMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
            if (m_wndRegisterBar.m_hWnd != 0)
                m_wndRegisterBar.SendMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
            if (m_Idents.m_hWnd != 0)
                m_Idents.SendMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
        }
        else
            return;
    }

    CWinApp* pApp = AfxGetApp();

    SaveBarState(REG_ENTRY_LAYOUT);		   // zapisanie po�o�enia pask�w narz�dzi

    WINDOWPLACEMENT wp;
    if (GetWindowPlacement(&wp))
    {
        // zapisanie po�o�enia okna g��wnego
        wxRect wnd(wp.rcNormalPosition);
        pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_POSX,wnd.left);
        pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_POSY,wnd.top);
        pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_SIZX,wnd.Width());
        pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_SIZY,wnd.Height());
        pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_STATE,wp.showCmd==SW_SHOWMAXIMIZED ? 1 : 0);
    }

    CMDIFrameWnd::OnClose();
}


void CMainFrame::OnDestroy()
{
    if (m_uTimer)
        KillTimer(m_uTimer);
    ConfigSettings(false);		// zapis ustawie�
    CMDIFrameWnd::OnDestroy();
}

//-----------------------------------------------------------------------------

void CMainFrame::OnAssemble()
{
    CSrc6502View *pView= GetCurrentView();
    if (pView==NULL)
        return;

    if (m_IOWindow.IsWaiting())
    {
        m_IOWindow.SetFocus();
        return;
    }

    SendMessageToViews(WM_USER_REMOVE_ERR_MARK);

    if (theApp.m_global.IsDebugger())	// dzia�a debugger?
    {
        if (AfxMessageBox(IDS_STOP_DEBUG,MB_OKCANCEL) != IDOK)
            return;
        ExitDebugMode();			// wyj�cie z trybu debuggera
    }

    if (CDocument* pDocument= pView->GetDocument())
    {
        // before assembly start set current dir to the document directory,
        // so include directive will find included files
        //
        const std::string& strPath= pDocument->GetPathName();
        if (!strPath.IsEmpty())
        {
            std::string strDir= strPath.Left(strPath.ReverseFind('\\'));
            ::SetCurrentDirectory(strDir);
        }
    }

    CDialAsmStat dial(pView);

    dial.DoModal();

//  dial.Create();

//  for (int i=0; i<1000; i++)
//    dial.SetValues(i,1);

}


void CMainFrame::OnUpdateAssemble(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetCurrentDocument() != NULL &&	// jest aktywny dokument?
                   !m_IOWindow.IsWaiting());
}


CSrc6502Doc *CMainFrame::GetCurrentDocument()
{
    CMDIChildWnd *pWnd= MDIGetActive();
    if (pWnd==NULL)
        return NULL;

    CDocument *pDoc= pWnd->GetActiveDocument();	// aktywny dokument
    if (pDoc==NULL || !pDoc->IsKindOf( RUNTIME_CLASS(CSrc6502Doc) ))
        return NULL;

    return static_cast<CSrc6502Doc*>(pDoc);
}


CSrc6502View *CMainFrame::GetCurrentView()
{
    CMDIChildWnd *pWnd= MDIGetActive();
    if (pWnd==NULL)
        return NULL;

    CView *pView= pWnd->GetActiveView();	// aktywne okno 'view'
    if (pView==NULL || !pView->IsKindOf( RUNTIME_CLASS(CSrc6502View) ))
        return NULL;

    return static_cast<CSrc6502View*>(pView);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateSymDebug(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(theApp.m_global.IsCodePresent());	// jest zasemblowany program?
    pCmdUI->SetCheck(theApp.m_global.IsDebugger());	// debugger uruchomiony
}

void CMainFrame::OnSymDebug()		// uruchomienie debuggera
{
    if (theApp.m_global.IsDebugger())   // ju� uruchomiony?
    {
        OnSymDebugStop();
    }
    else
    {
        if (!theApp.m_global.IsCodePresent())
            return;

        if (theApp.m_global.m_bProc6502==2) //1.3 65816 mode - disable simulator mode
        {
            std::string cs;
            cs.Format("The debugger does not currently support the 65816 processor.  Please go to the program options and selected a different processor to use the debugger.");
            MessageBoxA( cs, "Reminder", MB_OK );
            return;
        }
        theApp.m_global.StartDebug();
//		m_wndToolBar.OnInitialUpdate();
        DelayedUpdateAll();
        StartIntGenerator();
    }
    /*
      if (theApp.m_global.GetSimulator() == NULL)
        return;
      m_wndRegisterBar.Update( theApp.m_global.GetSimulator()->GetContext(), theApp.m_global.GetStatMsg() );

      if (m_IOWindow.m_hWnd != 0)
      {
        m_IOWindow.SendMessage(CBroadcast::WM_USER_START_DEBUGGER,0,0);
        m_IOWindow.SendMessage(CIOWindow::CMD_CLS);
      }
      if (m_wndRegisterBar.m_hWnd != 0)
        m_wndRegisterBar.PostMessage(CBroadcast::WM_USER_START_DEBUGGER,0,0);
      if (m_Idents.m_hWnd != 0)
        m_Idents.PostMessage(CBroadcast::WM_USER_START_DEBUGGER,0,0);
    */
}

//-----------------------------------------------------------------------------
/*
void CMainFrame::ClearPositionText()
{
  m_wndStatusBar.SetPaneText(1,NULL);
}
*/

void CMainFrame::SetPositionText(int row, int col)
{
    std::string strPosition;
    strPosition.Format(m_strFormat,row,col);
    m_wndStatusBar.SetPaneText(1,strPosition);
    m_wndStatusBar.UpdateWindow();
}

/*
void CMainFrame::SetRowColumn(CEdit &edit)
{
  int idx= edit.LineIndex();
  if (idx == -1)
    return;
  int row= edit.LineFromChar(idx);

  SetPositionText(row+1,0);
}
*/

//-----------------------------------------------------------------------------

void CMainFrame::OnSymBreakpoint()
{
    CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
//  ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

    if (!theApp.m_global.IsDebugger() || pView==NULL)
        return;

    if (pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)))
    {
        int line= pView->GetCurrLineNo();	// bie��cy wiersz
        // ustawienie miejsca przerwania w kodzie wynikowym odpowiadaj�cym bie��cemu wierszowi
        CAsm::Breakpoint bp= theApp.m_global.SetBreakpoint( line, pView->GetDocument()->GetPathName() );
        if (bp != CAsm::BPT_NO_CODE)
        {
            if (bp != CAsm::BPT_NONE)
                AddBreakpoint(pView, line, bp);
            else
                RemoveBreakpoint(pView, line);
        }
        else
            AfxMessageBox(IDS_SRC_NO_CODE);
    }
    else if (pView->IsKindOf(RUNTIME_CLASS(CDeasm6502View)))
    {
        ;
    }
    else
    {
        ASSERT(false);	// aktywne okno nierozpoznane
        return;
    }
}

void CMainFrame::OnUpdateSymBreakpoint(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
                    GetActiveFrame()->GetActiveView() );		// i aktywny dokument?
}


void CMainFrame::AddBreakpoint(CSrc6502View* pView, int nLine, CAsm::Breakpoint bp)
{
    if (pView == 0)
        return;

    CDocument* pDoc= pView->GetDocument();
    POSITION pos= pDoc->GetFirstViewPosition();
    while (pos != NULL)
        if (CSrc6502View* pSrcView= dynamic_cast<CSrc6502View*>(pDoc->GetNextView(pos)))
            pSrcView->AddBreakpoint(nLine, bp);
}

void CMainFrame::RemoveBreakpoint(CSrc6502View* pView, int nLine)
{
    if (pView == 0)
        return;

    CDocument* pDoc= pView->GetDocument();
    POSITION pos= pDoc->GetFirstViewPosition();
    while (pos != NULL)
        if (CSrc6502View* pSrcView= dynamic_cast<CSrc6502View*>(pDoc->GetNextView(pos)))
            pSrcView->RemoveBreakpoint(nLine);
}


void CMainFrame::OnSymEditBreakpoint()
{
    CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
    ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

    if (!theApp.m_global.IsDebugger() || pView==NULL)
        return;
    int line= pView->GetCurrLineNo();	// bie��cy wiersz

    // pobranie parametr�w przerwania w kodzie wynikowym odpowiadaj�cym bie��cemu wierszowi
    CAsm::Breakpoint bp= theApp.m_global.GetBreakpoint( line, pView->GetDocument()->GetPathName() );
    if (bp != CAsm::BPT_NO_CODE)
    {
        CDialEditBreakpoint edit_bp(bp,this);
        if (edit_bp.DoModal() != IDOK)
            return;
        bp = edit_bp.GetBreakpoint();
        if ((bp & CAsm::BPT_MASK) == CAsm::BPT_NONE)
        {
            theApp.m_global.ClrBreakpoint( line, pView->GetDocument()->GetPathName() );
            pView->RemoveBreakpoint(line);
        }
        else
        {
            theApp.m_global.ModifyBreakpoint( line, pView->GetDocument()->GetPathName(), bp );
            AddBreakpoint(pView, line,bp);
        }
    }
    else
        AfxMessageBox(IDS_SRC_NO_CODE);
}

void CMainFrame::OnUpdateSymEditBreakpoint(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
                    GetActiveFrame()->GetActiveView() );		// i aktywny dokument?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymBreak()
{
    if (!theApp.m_global.IsProgramRunning())
        return;
    theApp.m_global.GetSimulator()->Break();		// przerwanie dzia�aj�cego programu
    DelayedUpdateAll();

    AfxGetMainWnd()->SetFocus();		// restore focus (so it's not in i/o window)
}

void CMainFrame::OnUpdateSymBreak(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(theApp.m_global.IsProgramRunning());	// jest dzia�aj�cy program i debugger?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymSkipInstr()	// omini�cie bie��cej instrukcji
{
    if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
            theApp.m_global.IsProgramFinished() )
        return;
    theApp.m_global.GetSimulator()->SkipInstr();
}

void CMainFrame::OnUpdateSymSkipInstr(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
                    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
                    !theApp.m_global.IsProgramFinished() );		// i niezako�czony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymGo()		// uruchomienie programu
{
    if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
            theApp.m_global.IsProgramFinished() )
        return;
    theApp.m_global.GetSimulator()->Run();
}

void CMainFrame::OnUpdateSymGo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
                    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
                    !theApp.m_global.IsProgramFinished() );		// i niezako�czony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymGoToLine()	// uruchomienie do wiersza
{
    CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
    ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

    if (pView==NULL || !theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
            theApp.m_global.IsProgramFinished() )
        return;

    int line= pView->GetCurrLineNo();
    CAsm::DbgFlag flg= theApp.m_global.GetLineDebugFlags(line,pView->GetDocument()->GetPathName());
    if (flg == CAsm::DBG_EMPTY || (flg & CAsm::DBG_MACRO))	// wiersz bez kodu wynikowego?
    {
        AfxMessageBox(IDS_SRC_NO_CODE2);
        return;
    }
    else if (flg & CAsm::DBG_DATA)			// wiersz z danymi zamiast rozkaz�w?
    {
        if (AfxMessageBox(IDS_SRC_DATA,MB_YESNO) != IDYES)
            return;
    }

    theApp.m_global.SetTempExecBreakpoint(line,pView->GetDocument()->GetPathName());
    theApp.m_global.GetSimulator()->Run();	// uruchomienie po ustawieniu tymczasowego przerwania
}

void CMainFrame::OnUpdateSymGoToLine(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
                    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
                    !theApp.m_global.IsProgramFinished() &&		// niezako�czony program
                    GetActiveFrame()->GetActiveView() &&		// i aktywny dokument?
                    GetActiveFrame()->GetActiveView()->IsKindOf(RUNTIME_CLASS(CSrc6502View)) );
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymSkipToLine()	// przestawienie PC na bie��cy wiersz
{
    CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
    ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

    if (pView==NULL || !theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
            theApp.m_global.IsProgramFinished() )
        return;
    int line= pView->GetCurrLineNo();
    CAsm::DbgFlag flg= theApp.m_global.GetLineDebugFlags(line,pView->GetDocument()->GetPathName());
    if (flg == CAsm::DBG_EMPTY || (flg & CAsm::DBG_MACRO))	// wiersz bez kodu wynikowego?
    {
        AfxMessageBox(IDS_SRC_NO_CODE3);
        return;
    }
    else if (flg & CAsm::DBG_DATA)			// wiersz z danymi zamiast rozkaz�w?
    {
        if (AfxMessageBox(IDS_SRC_DATA,MB_YESNO) != IDYES)
            return;
    }
    UINT32 addr= theApp.m_global.GetLineCodeAddr(line,pView->GetDocument()->GetPathName());
    theApp.m_global.GetSimulator()->SkipToAddr(addr);
}

void CMainFrame::OnUpdateSymSkipToLine(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
                    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
                    !theApp.m_global.IsProgramFinished() &&		// niezako�czony program
                    GetActiveFrame()->GetActiveView() &&		// i aktywny dokument?
                    GetActiveFrame()->GetActiveView()->IsKindOf(RUNTIME_CLASS(CSrc6502View)) );
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymGoToRts() // Run to return from the subroutine
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramRunning() ||
        wxGetApp().m_global.IsProgramFinished() )
    {
        return;
    }

    theApp.m_global.GetSimulator()->RunTillRet();
}

void CMainFrame::OnUpdateSymGoToRts(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger() && // there is a working debugger
                   !wxGetApp().m_global.IsProgramRunning() && // and stopped
                   !wxGetApp().m_global.IsProgramFinished() ); // and unfinished program?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymStepInto()	// wykonanie bie��cej instrukcji
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramRunning() ||
        wxGetApp().m_global.IsProgramFinished() )
    {
        return;
    }

    wxGetApp().m_global.GetSimulator()->StepInto();
    DelayedUpdateAll();
}

void CMainFrame::OnUpdateSymStepInto(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger() && // there is a working debugger
                   !wxGetApp().m_global.IsProgramRunning() && // and stopped
                   !wxGetApp().m_global.IsProgramFinished() ); // and unfinished program?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymStepOver()
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramRunning() ||
        wxGetApp().m_global.IsProgramFinished() )
    {
        return;
    }

    wxGetApp().m_global.GetSimulator()->StepOver();
}

void CMainFrame::OnUpdateSymStepOver(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger() && // there is a working debugger
                   !wxGetApp().m_global.IsProgramRunning() && // and stopped
                   !wxGetApp().m_global.IsProgramFinished() ); // and unfinished program?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymRestart()
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramRunning())
        return;

    wxGetApp.m_global.RestartProgram();
    DelayedUpdateAll();
}

void CMainFrame::OnUpdateSymRestart(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger() && // there is a working debugger
                   !wxGetApp().m_global.IsProgramRunning() ); // and stopped program?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymAnimate()
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramRunning() ||
        wxGetApp().m_global.IsProgramFinished() )
    {
        return;
    }

    wxGetApp().m_global.GetSimulator()->Animate();
}

void CMainFrame::OnUpdateSymAnimate(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger() && // there is a working debugger
                   !wxGetApp().m_global.IsProgramRunning() && // and stopped
                   !wxGetApp().m_global.IsProgramFinished()); // and unfinished program?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymDebugStop()
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramRunning())
    {
        MessageBeep(-2);
        return;
    }
    wxGetApp().m_global.ExitDebugger();
    StopIntGenerator();
    /*
      if (m_IOWindow.m_hWnd != 0)
        m_IOWindow.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
      if (m_wndRegisterBar.m_hWnd != 0)
        m_wndRegisterBar.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
      if (m_Idents.m_hWnd != 0)
        m_Idents.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
    */
}

//void CMainFrame::OnUpdateSymDebugStop(CCmdUI* pCmdUI)
//{
//  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia�aj�cy debugger
//    !theApp.m_global.IsProgramRunning() );		// oraz zatrzymany program?
//}

//=============================================================================

afx_msg LRESULT CMainFrame::OnUpdateState(WPARAM wParam, LPARAM lParam)
{
    if (wxGetApp().m_global.IsDebugger())
    {
        wxGetApp().m_global.GetSimulator()->Update((CAsm::SymStat)wParam, lParam != 0);
        DelayedUpdateAll();
    }

    return 0;
}

//---------------------------- Opcje programu ---------------------------------

int CMainFrame::Options(int page)
{
    COptions dial(this, page);

    dial.m_EditPage.m_bAutoIndent		= CSrc6502View::m_bAutoIndent;
    dial.m_EditPage.m_nTabStep			= CSrc6502View::m_nTabStep;
    dial.m_EditPage.m_bAutoSyntax		= CSrc6502View::m_bAutoSyntax;
    dial.m_EditPage.m_bAutoUppercase	= CSrc6502View::m_bAutoUppercase;
    dial.m_EditPage.m_bFileNew			= C6502App::m_bFileNew;

    dial.m_SymPage.m_nIOAddress			= CSym6502::io_addr;
    dial.m_SymPage.m_bIOEnable			= CSym6502::io_enabled;
    dial.m_SymPage.m_nFinish			= wxGetApp().m_global.GetSymFinish();

    m_IOWindow.GetSize(dial.m_SymPage.m_nWndWidth, dial.m_SymPage.m_nWndHeight);

    dial.m_SymPage.m_bProtectMemory		= CSym6502::s_bWriteProtectArea;
    dial.m_SymPage.m_nProtFromAddr		= CSym6502::s_uProtectFromAddr;
    dial.m_SymPage.m_nProtToAddr		= CSym6502::s_uProtectToAddr;
    //  m_IOWindow.GetColors(dial.m_SymPage.m_rgbTextColor,dial.m_SymPage.m_rgbBackgndColor);

    dial.m_DeasmPage.m_rgbAddress = CDeasm6502View::m_rgbAddress;
    dial.m_DeasmPage.m_rgbCode = CDeasm6502View::m_rgbCode;
    //  dial.m_DeasmPage.m_rgbInstr = CDeasm6502View::m_rgbInstr;
    dial.m_DeasmPage.m_ShowCode = CDeasm6502View::m_bDrawCode;

    //dial.m_MarksPage.m_nProc6502 = !wxGetApp().m_global.GetProcType();
    dial.m_MarksPage.m_nProc6502 = wxGetApp().m_global.GetProcType();
    dial.m_MarksPage.m_nHelpFile = wxGetApp().m_global.GetHelpType();

    dial.m_MarksPage.m_uBusWidth = CSym6502::bus_width;
    dial.m_MarksPage.m_rgbPointer = CMarks::m_rgbPointer;
    dial.m_MarksPage.m_rgbBreakpoint = CMarks::m_rgbBreakpoint;
    dial.m_MarksPage.m_rgbError = CMarks::m_rgbError;

    dial.m_AsmPage.m_nCaseSensitive = CAsm6502::case_insensitive;
    dial.m_AsmPage.m_nAsmInstrWithDot = 0;
    dial.m_AsmPage.m_bGenerateListing = wxGetApp().m_global.m_bGenerateListing;
    dial.m_AsmPage.m_strListingFile = wxGetApp().m_global.m_strListingFile;
    dial.m_AsmPage.m_bGenerateBRKExtraByte = CAsm6502::generateBRKExtraByte;
    dial.m_AsmPage.m_uBrkExtraByte = CAsm6502::BRKExtraByte;

    int i;
    for (i = 0; text_color[i]; i++) // Read colors
    {
        dial.m_ViewPage.m_Text[i].text = *text_color[i];
        dial.m_ViewPage.m_Text[i].bkgnd = *bkgnd_color[i];
    }

    if (dial.DoModal() != IDOK)
        return dial.GetLastActivePage();

    C6502App::m_bFileNew = dial.m_EditPage.m_bFileNew;
    //CSrc6502View::m_bAutoIndent = dial.m_EditPage.m_bAutoIndent;
    CSrc6502View::m_bAutoSyntax = dial.m_EditPage.m_bAutoSyntax;
    CSrc6502View::m_bAutoUppercase = dial.m_EditPage.m_bAutoUppercase;

    if (dial.m_EditPage.m_bColorChanged)
    {
        for (int nColor = 0; nColor <= 5; ++nColor)
            CSrc6502View::m_vrgbColorSyntax[nColor] = *dial.m_EditPage.GetColorElement(nColor);

        for (int nStyle = 0; nStyle <= 4; ++nStyle)
            CSrc6502View::m_vbyFontStyle[nStyle] = *dial.m_EditPage.GetFontStyle(nStyle);
    }

    CSym6502::io_addr    = dial.m_SymPage.m_nIOAddress;
    CSym6502::io_enabled = dial.m_SymPage.m_bIOEnable;

    wxGetApp().m_global.SetSymFinish((CAsm::Finish)dial.m_SymPage.m_nFinish);
    m_IOWindow.SetSize(dial.m_SymPage.m_nWndWidth, dial.m_SymPage.m_nWndHeight, -1);

    CSym6502::s_bWriteProtectArea	= !!dial.m_SymPage.m_bProtectMemory;
    CSym6502::s_uProtectFromAddr	= dial.m_SymPage.m_nProtFromAddr;
    CSym6502::s_uProtectToAddr		= dial.m_SymPage.m_nProtToAddr;

    if (CSym6502::s_uProtectFromAddr > CSym6502::s_uProtectToAddr)
        std::swap(CSym6502::s_uProtectToAddr, CSym6502::s_uProtectFromAddr);

    //_IOWindow.SetColors(dial.m_SymPage.m_rgbTextColor, dial.m_SymPage.m_rgbBackgndColor);

    CDeasm6502View::m_rgbAddress = dial.m_DeasmPage.m_rgbAddress;
    CDeasm6502View::m_rgbCode    = dial.m_DeasmPage.m_rgbCode;
    //CDeasm6502View::m_rgbInstr = dial.m_DeasmPage.m_rgbInstr;
    CDeasm6502View::m_bDrawCode  = dial.m_DeasmPage.m_ShowCode;

    //wxGetApp().m_global.SetProcType(!dial.m_MarksPage.m_nProc6502);
    wxGetApp().m_global.SetProcType(dial.m_MarksPage.m_nProc6502);
    wxGetApp().m_global.SetHelpType(dial.m_MarksPage.m_nHelpFile);

    CSym6502::bus_width     = dial.m_MarksPage.m_uBusWidth;
    CMarks::m_rgbPointer    = dial.m_MarksPage.m_rgbPointer;
    CMarks::m_rgbBreakpoint = dial.m_MarksPage.m_rgbBreakpoint;
    CMarks::m_rgbError      = dial.m_MarksPage.m_rgbError;

    wxGetApp().m_global.m_bGenerateListing = dial.m_AsmPage.m_bGenerateListing;
    wxGetApp().m_global.m_strListingFile   = dial.m_AsmPage.m_strListingFile;

    CAsm6502::generateBRKExtraByte     = dial.m_AsmPage.m_bGenerateBRKExtraByte;
    CAsm6502::BRKExtraByte             = dial.m_AsmPage.m_uBrkExtraByte;
    CAsm6502::case_insensitive         = dial.m_AsmPage.m_nCaseSensitive;

    if (dial.m_EditPage.m_nTabStep != CSrc6502View::m_nTabStep ||
        dial.m_EditPage.m_bColorChanged ||
        !!dial.m_EditPage.m_bAutoIndent != CSrc6502View::m_bAutoIndent)
    {
        CSrc6502View::m_nTabStep = dial.m_EditPage.m_nTabStep;
        CSrc6502View::m_bAutoIndent = dial.m_EditPage.m_bAutoIndent;
        RedrawAllViews();
    }
    
    /*
      if (dial.m_EditPage.m_bFontChanged || dial.m_EditPage.m_nTabStep != CSrc6502View::m_nTabStep ||
        dial.m_DeasmPage.m_bColorChanged || dial.m_MarksPage.m_bColorChanged)
      {
        CSrc6502View::m_nTabStep = dial.m_EditPage.m_nTabStep;
        RedrawAllViews(dial.m_EditPage.m_bFontChanged);
      }
    */
    
    for (i = 0; ConfigSettings::text_color[i]; i++) // Adjust/Save colors
    {
#if REWRITE_TO_WX_WIDGET
        *ConfigSettings::text_color[i] = dial.m_ViewPage.m_Text[i].text;
        *ConfigSettings::bkgnd_color[i] = dial.m_ViewPage.m_Text[i].bkgnd;
#endif

        if (dial.m_ViewPage.m_Text[i].changed & 2) // Font changed?
        {
#if REWRITE_TO_WX_WIDGET
            dial.m_ViewPage.m_Text[i].font.GetLogFont(fonts[i]);

            delete ConfigSettings::cfonts[i];
            cfonts[i]->CreateFontIndirect(&ConfigSettings::cfonts[i]);
#endif
        }

        if (dial.m_ViewPage.m_Text[i].changed) // Changed font or colors?
        {
            switch (i)
            {
            case 0: // Editor
                RedrawAllViews(dial.m_ViewPage.m_Text[i].changed & 2);
                break;
                
            case 1: // Simulator
                m_IOWindow.Resize();
                break;

            case 2: // debugger
                RedrawAllViews(dial.m_ViewPage.m_Text[i].changed & 2);
                break;

            case 3: // 6502 Memory
                m_Memory.Refresh();
                break;

            case 4: // Zero page
                m_ZeroPage.Refresh();
                break;

            case 5: // stack
                m_Stack.Refresh();
                break;

            default:
                ASSERT(false);
                break;
            }
        }
    }

    // Redraw disassembler windows
#if REWRITE_TO_WX_WIDGET
    POSITION posDoc = wxGetApp().m_pDocDeasmTemplate->GetFirstDocPosition();
    while (posDoc != NULL) // Are the windows from the disassembler?
    {
        CDocument* pDoc = wxGetApp().m_pDocDeasmTemplate->GetNextDoc(posDoc);
        pDoc->UpdateAllViews(NULL);
    }
#endif

    return dial.GetLastActivePage();
}

void CMainFrame::OnOptions()
{
    m_nLastPage = Options(m_nLastPage);
}

void CMainFrame::OnUpdateOptions(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

int CMainFrame::RedrawAllViews(int chgHint)	// 'Invalidate' wszystkich okien
{
#if REWRITE_TO_WX_WIDGET
    CWinApp *pApp = AfxGetApp();
    POSITION posTempl = pApp->GetFirstDocTemplatePosition();
    while (posTempl != NULL)
    {
        CDocTemplate *pTempl = pApp->GetNextDocTemplate(posTempl);
        POSITION posDoc = pTempl->GetFirstDocPosition();
        while (posDoc != NULL)
        {
            CDocument *pDoc = pTempl->GetNextDoc(posDoc);
            POSITION posView = pDoc->GetFirstViewPosition();
            while (posView != NULL)
            {
                CView* pView = pDoc->GetNextView(posView);
                if (pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)))
                {
                    CSrc6502View* pSrcView = static_cast<CSrc6502View*>(pView);
#ifdef USE_CRYSTAL_EDIT
                    pSrcView->SetTabSize(CSrc6502View::m_nTabStep);
                    pSrcView->SetAutoIndent(CSrc6502View::m_bAutoIndent);
#else
                    int nTabStep = CSrc6502View::m_nTabStep * 4;
                    pSrcView->GetEditCtrl().SetTabStops(nTabStep);
#endif
                    pSrcView->SelectEditFont();
                }
                pView->Invalidate();
                //	pView->UpdateWindow();
            }
        }
        //GetNextDoc(posDoc)->UpdateAllViews(NULL);
    }
#endif

    return 0;
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewRegisterWnd()
{
#if REWRITE_TO_WX_WIDGET
    ShowControlBar(&m_wndRegisterBar, !m_wndRegisterBar.IsShown(), false);
#endif
    //m_wndToolBar.OnInitialUpdate();
    /*
      if (wxGetApp().m_global.IsDebugger()) // jest program?
        m_wndRegisterBar.Show(!m_wndRegisterBar.IsShown());
      else // nie ma programu
        m_wndRegisterBar.Hide();
    */
}

void CMainFrame::OnUpdateIdViewRegisterbar(CCmdUI* pCmdUI)
{
    //OnUpdateControlBarMenu(pCmdUI);

#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger()); // Is the debugger running?
    CControlBar *pBar = GetControlBar(pCmdUI->m_nID);
    pCmdUI->SetCheck(pBar ? pBar->IsShown() : false);
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnFileSaveCode() //@@
{
#if REWRITE_TO_WX_WIDGET
    std::string filter;
    std::string filename = "";

    if (CMainFrame::ProjName == "")
        filename = "BinaryCode";
    else
        filename = CMainFrame::ProjName;

    filter.LoadString(IDS_SAVE_CODE);
    CSaveCode dlg(filename, filter, this);

    if (dlg.ShowModal() == wxID_OK)
        dlg.SaveCode();
#endif
}

void CMainFrame::OnUpdateFileSaveCode(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsCodePresent()); // Is there a loaded program?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewDeasm()
{
    if (!wxGetApp().m_global.IsDebugger())
        return; // No debugger running.

    wxGetApp().m_global.CreateDeasm();
}

void CMainFrame::OnUpdateViewDeasm(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger());//is there a working debugger?
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewIdents()
{
    if (wxGetApp().m_global.IsDebugInfoPresent()) // Is the program assembled?
        m_Idents.Show(!m_Idents.IsShown());
    else // There is no assembled program
        m_Idents.Hide();
}

void CMainFrame::OnUpdateViewIdents(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugInfoPresent()); // jest zasemblowany program?
    pCmdUI->SetCheck(m_Idents.IsShown());
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewMemory()
{
    if (wxGetApp().m_global.IsCodePresent()) // is there a program?
        m_Memory.Show(!m_Memory.IsShown());
    else		// nie ma programu
        m_Memory.Hide();
}

void CMainFrame::OnUpdateViewMemory(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsCodePresent()); // Is there a program?
    pCmdUI->SetCheck(m_Memory.IsShown());
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnEditorOpt()
{
    m_nLastPage = Options(2); // Editor options
}

void CMainFrame::OnUpdateEditorOpt(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewIOWindow()
{
    if (!wxGetApp().m_global.IsDebugger()) // No working debugger?
        return;

    m_IOWindow.Show(!m_IOWindow.IsShown());
}

void CMainFrame::OnUpdateViewIOWindow(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger()); // Is the debugger running?
    pCmdUI->SetCheck(m_IOWindow.IsShown());
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnFileLoadCode()
{
#if REWRITE_TO_WX_WIDGET
    std::string filter;
    filter.LoadString(IDS_LOAD_CODE);

    CLoadCode dlg("",filter,this);

    if (dlg.ShowModal() == wxID_OK)
        dlg.LoadCode();
#endif
}

void CMainFrame::OnUpdateFileLoadCode(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable( true );
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::ExitDebugMode()
{
    if (wxGetApp().m_global.IsProgramRunning())
    {
        if (m_IOWindow.IsWaiting())
            m_IOWindow.ExitModalLoop();
            
        wxGetApp().m_global.GetSimulator()->AbortProg(); // Interrupt the running program
    }

    OnSymDebugStop();
}

//-----------------------------------------------------------------------------

void CMainFrame::OnDeasmOptions()
{
    m_nLastPage = Options(3); // Disassembler options
}

void CMainFrame::OnUpdateDeasmOptions(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSysColorChange()
{
#if REWRITE_TO_WX_WIDGET
    CMDIFrameWnd::OnSysColorChange();

    m_bmpCode.DeleteObject();
    m_bmpCode.LoadMappedBitmap(IDB_CODE);
    m_bmpDebug.DeleteObject();
    m_bmpDebug.LoadMappedBitmap(IDB_DEBUG);
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateViewZeropage(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsCodePresent()); // Is there a program?
    pCmdUI->SetCheck(m_ZeroPage.IsShown());
#endif
}

void CMainFrame::OnViewZeropage()
{
    if (wxGetApp().m_global.IsCodePresent()) // is simulator present?
        m_ZeroPage.Show(!m_ZeroPage.IsShown());
    else // There is no program
        m_ZeroPage.Hide();
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewLog()
{
    if (wxGetApp().m_global.IsDebugger()) // is simulator present?
        m_wndLog.Show(!m_wndLog.IsShown());
    else // There is no program
        m_wndLog.Hide();
}

void CMainFrame::OnUpdateViewLog(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger()); // is simulator present?
    pCmdUI->SetCheck(m_wndLog.IsShown());
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewStack()
{
    if (wxGetApp().m_global.IsCodePresent()) // Is there a program?
        m_Stack.Show(!m_Stack.IsShown());
    else // There is no program
        m_Stack.Hide();
}

void CMainFrame::OnUpdateViewStack(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsCodePresent()); // Is there a program?
    pCmdUI->SetCheck(m_Stack.IsShown());
#endif
}

//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateMemoryOptions(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(true);
#endif
}

void CMainFrame::OnMemoryOptions()
{
    m_nLastPage = Options(5); // Memory window appearance options
}

//-----------------------------------------------------------------------------

#if REWRITE_TO_WX_WIDGET
bool CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (m_Stack.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return true;

    if (m_ZeroPage.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return true;

    if (m_Memory.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return true;

    if (m_IOWindow.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return true;

    // If the object(s) in the extended command route don't handle
    // the command, then let the base class OnCmdMsg handle it.
    return CMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
#endif

//-----------------------------------------------------------------------------

void CMainFrame::UpdateAll()
{
    m_IOWindow.Refresh();
    m_Memory.Refresh();
    m_ZeroPage.Refresh();
    m_Stack.Refresh();
    m_wndLog.Refresh();
}

void CMainFrame::DelayedUpdateAll()
{
//	if (m_uTimer == 0)
#if REWRITE_TO_WX_WIDGET
    m_uTimer = SetTimer(100, 200, NULL);

    if (m_uTimer == 0)
        UpdateAll();
#endif
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
#if REWRITE_TO_WX_WIDGET
    KillTimer(m_uTimer);
    m_uTimer = 0;
    UpdateAll();
#endif

//  CMDIFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::SymGenInterrupt(CSym6502::IntType eInt)
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramFinished())
        return;

    wxGetApp().m_global.GetSimulator()->Interrupt(eInt);
}

void CMainFrame::UpdateSymGenInterrupt(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable(wxGetApp().m_global.IsDebugger() && !wxGetApp().m_global.IsProgramFinished());
#endif
}

void CMainFrame::OnSymGenIRQ()
{
    SymGenInterrupt(CSym6502::IRQ);
}

void CMainFrame::OnUpdateSymGenIRG(CCmdUI* pCmdUI)
{
    UpdateSymGenInterrupt(pCmdUI);
}

void CMainFrame::OnSymGenNMI()
{
    SymGenInterrupt(CSym6502::NMI);
}

void CMainFrame::OnUpdateSymGenNMI(CCmdUI* pCmdUI)
{
    UpdateSymGenInterrupt(pCmdUI);
}

void CMainFrame::OnSymGenReset()
{
    SymGenInterrupt(CSym6502::RST);
}

void CMainFrame::OnUpdateSymGenReset(CCmdUI* pCmdUI)
{
    UpdateSymGenInterrupt(pCmdUI);
}

//% Bug fix 1.2.14.1 - convert to HTML help ------------------------------------------

#if REWRITE_TO_WX_WIDGET
void CMainFrame::OnHtmlHelp()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);

    std::string buff = buffer;
    buff = "start " + buff.substr(0, buff.size() - 4) + ".htm";

    if (wxGetApp().m_global.GetHelpType()) // m_nHelpFile
    {
        int _flushall(void);
        system(buff); //^^ help
    }
    else
        HtmlHelpA(NULL, HH_DISPLAY_TOPIC);
}
#endif

//------------------------------------------------------------------------------------

#if REWRITE_TO_WX_WIDGET
void CALLBACK EXPORT TimerProc(
    HWND hWnd,		// handle of CWnd that called SetTimer
    UINT nMsg,		// WM_TIMER
    UINT nIDEvent,	// timer identification
    DWORD dwTime)	// system time
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramFinished())
        return;

    if (!wxGetApp().m_global.IsProgramRunning())
        return;		// if program is not running do not send int request

    switch (nIDEvent)
    {
    case CSym6502::IRQ:
        wxGetApp().m_global.GetSimulator()->Interrupt(CSym6502::IRQ);
        break;

    case CSym6502::NMI:
        wxGetApp().m_global.GetSimulator()->Interrupt(CSym6502::NMI);
        break;
    }
}
#endif

// Interrupt Request Generator dialog window
//
void CMainFrame::OnSymGenIntDlg()
{
    CIntRequestGeneratorDlg dlg;

    dlg.m_bGenerateIRQ = wxGetApp().m_global.m_IntGenerator.m_bGenerateIRQ;
    dlg.m_uIRQTimeLapse = wxGetApp().m_global.m_IntGenerator.m_nIRQTimeLapse;

    dlg.m_bGenerateNMI = wxGetApp().m_global.m_IntGenerator.m_bGenerateNMI;
    dlg.m_uNMITimeLapse = wxGetApp().m_global.m_IntGenerator.m_nNMITimeLapse;

    if (dlg.ShowModal() != wxID_OK)
        return;

    StopIntGenerator();

    wxGetApp().m_global.m_IntGenerator.m_bGenerateIRQ = dlg.m_bGenerateIRQ;
    wxGetApp().m_global.m_IntGenerator.m_nIRQTimeLapse = dlg.m_uIRQTimeLapse;

    wxGetApp().m_global.m_IntGenerator.m_bGenerateNMI = dlg.m_bGenerateNMI;
    wxGetApp().m_global.m_IntGenerator.m_nNMITimeLapse = dlg.m_uNMITimeLapse;

    StartIntGenerator();
}

void CMainFrame::OnUpdateSymGenIntDlg(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->Enable();
#endif
}

void CMainFrame::StopIntGenerator()
{
#if REWRITE_TO_WX_WIDGET
    KillTimer(CSym6502::IRQ);
    KillTimer(CSym6502::NMI);
#endif
}

void CMainFrame::StartIntGenerator()
{
    if (!wxGetApp().m_global.IsDebugger() || wxGetApp().m_global.IsProgramFinished())
        return;

#if REWRITE_TO_WX_WIDGET
    if (wxGetApp().m_global.m_IntGenerator.m_bGenerateIRQ)
    {
        int nId = SetTimer(CSym6502::IRQ, wxGetApp().m_global.m_IntGenerator.m_nIRQTimeLapse, &TimerProc);
        ASSERT(nId);
    }

    if (wxGetApp().m_global.m_IntGenerator.m_bGenerateNMI)
    {
        int nId = SetTimer(CSym6502::NMI, wxGetApp().m_global.m_IntGenerator.m_nNMITimeLapse, &TimerProc);
        ASSERT(nId);
    }
#endif
}

void CMainFrame::ShowDynamicHelp(const std::string& strLine, int nWordStart, int nWordEnd)
{
    m_wndHelpBar.DisplayHelp(strLine, nWordStart, nWordEnd);
}

void CMainFrame::OnHelpDynamic()
{
#if REWRITE_TO_WX_WIDGET
    ShowControlBar(&m_wndHelpBar, !m_wndHelpBar.IsShown(), false);
#endif
}

void CMainFrame::OnUpdateHelpDynamic(CCmdUI* pCmdUI)
{
#if REWRITE_TO_WX_WIDGET
    pCmdUI->SetCheck(m_wndHelpBar.IsShown() ? 1 : 0);
#endif
}
