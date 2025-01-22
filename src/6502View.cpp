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

// 6502View.cpp : implementation of the CSrc6502View class
//

#include "StdAfx.h"
#include "6502.h"

#include "sim.h"

#include "6502Doc.h"
#include "6502View.h"
#include "M6502.h"

#if 0

bool CSrc6502View::m_bAutoIndent = true; // static component -automatic indentation
int CSrc6502View::m_nTabStep = 8;
bool CSrc6502View::m_bAutoSyntax = true;
bool CSrc6502View::m_bAutoUppercase = true;

wxFont CSrc6502View::s_font;
wxFontInfo CSrc6502View::s_logFont;

wxColour CSrc6502View::s_rgbTextColor;
wxColour CSrc6502View::s_rgbBkgndColor;

wxColour CSrc6502View::s_vrgbColorSyntax[] =
{
    wxColour(0, 0, 160),        // instructions
    wxColour(128, 0, 128),      // directives
    wxColour(128, 128, 128),    // comments
    wxColour(0, 0, 255),        // number
    wxColour(0, 128, 128),      // string
    wxColour(192, 192, 224),    // selection
    wxColour(128, 0, 0)         // operator
};

uint8_t CSrc6502View::m_vbyFontStyle[6] =
{
    0, 0, 0, 0, 0, 0
};
#endif

/////////////////////////////////////////////////////////////////////////////
// CSrc6502View construction/destruction

CSrc6502View::CSrc6502View()
    : wxView()
    , m_frame(nullptr)
    , m_text(nullptr)
    , m_status(nullptr)
{
    m_nActualPointerLine = -1;
    m_nActualErrMarkLine = -1;
}

CSrc6502View::~CSrc6502View()
{
}

wxIMPLEMENT_DYNAMIC_CLASS(CSrc6502View, wxView);

/*=======================================================================*/

bool CSrc6502View::OnCreate(wxDocument *doc, long flags)
{
    if (!wxView::OnCreate(doc, flags))
        return false;

    m_frame = wxGetApp().CreateChildFrame(this);

    ASSERT(m_frame == GetFrame());

    m_text = new wxStyledTextCtrl(m_frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    // The underlying Scintilla control uses actual font names.

#ifdef _WIN32
    m_text->StyleSetFaceName(0, "Fixedsys");
#endif

    m_status = m_frame->CreateStatusBar(2);

    // TODO: Fall back to main window if we can't have an independent status bar.

    if (m_status)
    {
        int statusStyles[] = { wxSB_NORMAL, wxSB_SUNKEN };
        int statusWidths[] = { -1, 160 }; // 160 was picked arbitrarily, should be a better way to do this.

        int cnt = sizeof(statusStyles) / sizeof(int);

        m_status->SetFieldsCount(cnt, statusWidths);
        m_status->SetStatusStyles(cnt, statusStyles);

        UpdatePositionInfo();
    }

    m_text->Bind(wxEVT_STC_UPDATEUI, &CSrc6502View::OnTextUpdate, this);

    m_frame->Show();
    m_frame->Layout();

    return true;
}

void CSrc6502View::OnDraw(wxDC *)
{
    // Dummy method, the sub controls handle all the actual painting.
}

/*=======================================================================*/

void CSrc6502View::UpdatePositionInfo()
{
    if (!m_status)
        return;

    wxString str;

    int line = m_text->GetCurrentLine() + 1;
    int pos = m_text->GetCurrentPos() + 1;

    str.Printf("Ln: %d    Ch: %d", line, pos);

    m_status->SetStatusText(str, 1);
}

/*=======================================================================*/
// Event processing

void CSrc6502View::OnTextUpdate(wxCommandEvent &)
{
    UpdatePositionInfo();
}

/*=======================================================================*/

void CSrc6502View::check_line(const char *buf, CAsm::Stat &stat, int &start, int &fin, std::string &msg)
{
    UNUSED(buf);
    UNUSED(stat);
    UNUSED(start);
    UNUSED(fin);
    UNUSED(msg);

    // TODO: Need to rethink how this should work; want to split the assembler from view.

#if 0
    CAsm6502 xasm;

    xasm.m_procType = wxGetApp().m_global.GetProcType();
    stat = xasm.CheckLine(buf, start, fin);

    if (stat)
        msg = xasm.GetErrMsg(stat);
    else
        msg.clear();
#endif
}

void CSrc6502View::disp_warning(int line, const std::string &msg) // debugging message use?
{
    SetErrMark(line); // Select the line containing the error
    wxLogWarning(msg.c_str());
}

//-----------------------------------------------------------------------------

#if REWRITE_TO_WX_WIDGET
LRESULT CALLBACK CSrc6502View::EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWnd *pWnd = FromHandlePermanent(hWnd);
    ASSERT(pWnd->IsKindOf(RUNTIME_CLASS(CSrc6502View)));
    CSrc6502View *pView = (CSrc6502View *)pWnd;
    bool cr = false;

    switch (msg)
    {
    case WM_CHAR:
        cr = (wParam == 0xD);    // CR?
    case WM_KEYUP:
    case WM_PASTE:
    case WM_COPY:
    case WM_CUT:
    case WM_UNDO:
    {
        LRESULT ret;
        if (cr && m_bAutoIndent)
        {
            ret = (*m_pfnOldProc)(hWnd, msg, wParam, lParam);
            pView->set_position_info(hWnd);
            int line_idx = (*m_pfnOldProc)(hWnd, EM_LINEINDEX, WPARAM(-1), 0);
            int line = (*m_pfnOldProc)(hWnd, EM_LINEFROMCHAR, line_idx, 0) - 1;	// nr aktualnego wiersza - 1
            line_idx = (*m_pfnOldProc)(hWnd, EM_LINEINDEX, WPARAM(line), 0);
            int line_len = (*m_pfnOldProc)(hWnd, EM_LINELENGTH, line_idx, 0);
            line = (*m_pfnOldProc)(hWnd, EM_LINEFROMCHAR, line_idx, 0);
            TCHAR buf[260];
            const int size = sizeof(buf) / sizeof(TCHAR) - 2;
            *(WORD *)(buf + 2) = (WORD)size;
            (*m_pfnOldProc)(hWnd, EM_GETLINE, line, (LPARAM)(buf + 2));
            buf[2 + min(size - 1, line_len)] = 0;

            /*
                int line_idx = (*m_pfnOldProc)(hWnd, EM_LINEINDEX, WPARAM(-1), 0);
                int line_len = (*m_pfnOldProc)(hWnd, EM_LINELENGTH, line_idx, 0);
                int line = (*m_pfnOldProc)(hWnd, EM_LINEFROMCHAR, line_idx, 0);	// nr aktualnego wiersza
                TCHAR buf[260];
                const int size = sizeof(buf) / sizeof(TCHAR) - 2;
                *(WORD *)(buf + 2) = (WORD)size;
                (*m_pfnOldProc)(hWnd, EM_GETLINE, line, (LPARAM)(buf + 2));
                buf[2 + min(size - 1, line_len)] = 0;
            */
            int start, fin;
            CAsm::Stat stat = CAsm::OK;
            std::string strmsg;

            if (m_bAutoSyntax || m_bAutoUppercase)
                check_line(buf + 2, stat, start, fin, strmsg);

            if (m_bAutoUppercase && start > 0 && fin > 0)	// jest instrukcja do zamiany na du�e litery?
            {
                TCHAR instr[32];
                ASSERT(fin - start < 32);
                _tcsncpy(instr, buf + 2 + start, fin - start);
                instr[fin - start] = 0;
                _tcsupr(instr);
                int c_start, c_end;
                (*m_pfnOldProc)(hWnd, EM_GETSEL, WPARAM(&c_start), LPARAM(&c_end));
                (*m_pfnOldProc)(hWnd, EM_SETSEL, line_idx + start, line_idx + fin);
                (*m_pfnOldProc)(hWnd, EM_REPLACESEL, 0, (LPARAM)instr);
                (*m_pfnOldProc)(hWnd, EM_SETSEL, c_start, c_start);
                //	  (*m_pfnOldProc)(hWnd, EM_SETSEL, line_idx + line_len, line_idx + line_len);
            }
            int len = _tcsspn(buf + 2, _T(" \t"));	// ilo�� spacji i tabulator�w na pocz�tku wiersza
            if (!(m_bAutoSyntax && stat))		// je�li nie ma b��du (je�li spr. b��d�w), to  wci�cie
            {
                if (len)		// je�li jest wci�cie w wierszu powy�ej, to kopiujemy je
                {
                    buf[len + 2] = 0;
                    buf[0] = 0xD;
                    buf[1] = 0xA;
                    ret = (*m_pfnOldProc)(hWnd, EM_REPLACESEL, TRUE, (LPARAM)(buf + 2));	  // wci�cie tekstu
                }
                else
                    ;
                //	    ret = (*m_pfnOldProc)(hWnd, msg, wParam, lParam);	  // CR i rozsuni�cie wierszy
            }
            if (m_bAutoSyntax && stat)
                pView->disp_warning(line, strmsg);
        }
        else
        {
            ret = (*m_pfnOldProc)(hWnd, msg, wParam, lParam);
            pView->set_position_info(hWnd);
        }

        pView->RedrawMarks();
        /*
              pView->draw_breakpoints();
              if (pView->m_nActualPointerLine != -1)
                pView->DrawMark(pView->m_nActualPointerLine,MT_POINTER);
              if (pView->m_nActualErrMarkLine != -1)
                pView->DrawMark(pView->m_nActualErrMarkLine,MT_ERROR);
        */
        return ret;
    }

    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    {
        LRESULT ret = (*m_pfnOldProc)(hWnd, msg, wParam, lParam);
        pView->set_position_info(hWnd);
        return ret;
    }

    case WM_PAINT:
    {
        LRESULT ret = (*m_pfnOldProc)(hWnd, msg, wParam, lParam);
        if (ret == 0)
            pView->RedrawMarks();
        /*      {
            pView->draw_breakpoints((HDC)wParam);
            if (pView->m_nActualPointerLine != -1)
                  pView->DrawMark(pView->m_nActualPointerLine,MT_POINTER,FALSE,(HDC)wParam);
            if (pView->m_nActualErrMarkLine != -1)
                  pView->DrawMark(pView->m_nActualErrMarkLine,MT_ERROR,FALSE,(HDC)wParam);
              } */
        return ret;
    }

    default:
        return (*m_pfnOldProc)(hWnd, msg, wParam, lParam);
    }

}
#endif 

//=============================================================================
/*
void CSrc6502View::drawMark(CDC &dc, int line, MarkType type, bool scroll)
{
  int h, y= ScrollToLine(line,h,scroll);
  if (y < 0)
    return;
  y++;

  switch (type)
  {
    case MT_ERASE:	// zmazanie znacznika
    {
      dc.GetWindow()->RedrawWindow(CRect(1, y, 1 + h, y + h));
      break;
    }
    case MT_POINTER:	// narysowanie strza�ki wskazuj�cej inst. do wykonania
      draw_pointer(dc,1,y,h);
      break;
    case MT_BREAKPOINT:	// narysowanie aktywnego miejsca przerwania
      draw_breakpoint(dc,1,y,h,TRUE);
      break;
    case MT_DISBRKP:	// narysowanie wy��czonego miejsca przerwania
      draw_breakpoint(dc,1,y,h,FALSE);
      break;
    case MT_ERROR:	// narysowanie strza�ki wskazuj�cej b��d
      draw_mark(dc,4,y,h);
      break;
    default:
      ASSERT(FALSE);	// b��dna warto�� typu
      break;
  }
}
*/


// edit view info
//
void CSrc6502View::GetDispInfo(int &nTopLine, int &nLineCount, int &nLineHeight)
{
    UNUSED(nTopLine);
    UNUSED(nLineCount);
    UNUSED(nLineHeight);

#ifdef USE_CRYSTAL_EDIT
    nLineHeight = GetLineHeight();
    return;
#else
# if REWRITE_TO_WX_WIDGET
    CEdit &edit = GetEditCtrl();
    nLineCount = edit.GetLineCount();
    nTopLine = edit.GetFirstVisibleLine();

    CClientDC dc(&edit);
    CFont *pOld = dc.SelectObject(&s_Font);
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    nLineHeight = (int)tm.tmHeight + (int)tm.tmExternalLeading;
    dc.SelectObject(pOld);
# endif
#endif
}

/*
void CSrc6502View::DrawMark(int line, MarkType type, bool scroll, HDC hDC)
{
  if (m_wndLeftBar.m_hWnd == 0)
    return;

  CClientDC dc(&m_wndLeftBar);

  drawMark(dc, line, type, scroll);

#if 0
  if (hDC == NULL)
  {
    CClientDC dc(&GetEditCtrl());
    drawMark(dc,line,type,scroll);
  }
  else
    drawMark(*CDC::FromHandle(hDC),line,type,scroll);
#endif
} */

/*
void CSrc6502View::draw_breakpoints(HDC hDC)
{
  if (m_mapBreakpoints.IsEmpty())
    return;
  POSITION pos = m_mapBreakpoints.GetStartPosition();
  int line;
  BYTE bp;
  do
  {
    m_mapBreakpoints.GetNextAssoc(pos,line,bp);
    DrawMark(line,bp & CAsm::BPT_DISABLED ? MT_DISBRKP : MT_BREAKPOINT,FALSE,hDC);
  } while (pos);
}
*/

//-----------------------------------------------------------------------------

void CSrc6502View::SetPointer(int line, bool scroll) // draw/erase an arrow in the line
{
    UNUSED(scroll);

    if (m_nActualPointerLine != -1)
    {
        int tmp_line = m_nActualPointerLine;
        m_nActualPointerLine = -1;
        EraseMark(tmp_line); // delete the old thread
    }

    m_nActualPointerLine = line;

    if (line != -1)
    {
        m_text->ScrollToLine(line);
        //RedrawMarks(line);
    }
}

void CSrc6502View::SetErrMark(int line) // draw/erase the pointer arrow error
{
    if (m_nActualErrMarkLine != -1)
    {
        int tmp = m_nActualErrMarkLine;
        m_nActualErrMarkLine = -1;
        EraseMark(tmp); // Delete the old thread
    }
    m_nActualErrMarkLine = line;

    if (line != -1)
    {
        m_text->ScrollToLine(line);
        //RedrawMarks(line);

#ifdef USE_CRYSTAL_EDIT
        GoToLine(line);
#else
# if REWRITE_To_WX_WIDGET
        CEdit &edit = GetEditCtrl();
        int char_index = edit.LineIndex(line);
        edit.SendMessage(EM_SETSEL, char_index, char_index);//carriage to line 'line'
# endif
#endif
    }
}

void CSrc6502View::OnEnUpdate() // After changing the text
{
    if (m_nActualErrMarkLine != -1)
    {
        SetErrMark(-1); // Erase the pointer, wrong line

#if REWRITE_TO_WX_WIDGET
        CMainFrame *pMain = (CMainFrame *)AfxGetApp()->m_pMainWnd;
        pMain->m_wndStatusBar.SetPaneText(0, NULL); // and error message
#endif
    }
}

void CSrc6502View::SelectEditFont()
{
#if REWRITE_TO_WX_WIDGET

#ifdef USE_CRYSTAL_EDIT
    SetFont(m_LogFont);
    //	m_wndLeftBar.SetWidth(0);
    SetTabSize(m_nTabStep);
#else
    SetFont(&s_Font);
    CEdit &edit = GetEditCtrl();
    CClientDC dc(&edit);
    dc.SelectObject(&s_Font);
    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    int h = (int)tm.tmHeight + (int)tm.tmExternalLeading;
    //DWORD margins = edit.GetMargins();
    //edit.SetMargins(h + 1, UINT(HIWORD(margins))); // set the left margin
    m_wndLeftBar.SetWidth(h + 1);
    dynamic_cast<CFrameWnd *>(GetParent())->RecalcLayout();
#endif

#endif
}

int CSrc6502View::GetCurrLineNo()	// aktualny wiersz
{
#if REWRITE_TO_WX_WIDGET

#ifdef USE_CRYSTAL_EDIT
    return GetCursorPos().y;
#else
    CEdit &edit = GetEditCtrl();
    int idx = edit.LineIndex();
    ASSERT(idx != -1);
    return edit.LineFromChar(idx);
#endif

#endif

    return 0;
}

void CSrc6502View::AddBreakpoint(int line, CAsm::Breakpoint bp, bool draw)
{
    m_mapBreakpoints[line] = (uint8_t)bp;

    if (draw)
    {
        //RedrawMarks(line);
    }
}

void CSrc6502View::RemoveBreakpoint(int line, bool draw)
{
    m_mapBreakpoints.erase(line);

    if (draw)
    {
        //RedrawMarks(line);
    }
}

void CSrc6502View::EraseMark(int line)
{
    UNUSED(line);

    //ASSERT(line >= 0);
    //DrawMark(line, MT_ERASE);
    //RedrawMarks(line);
}

void CSrc6502View::OnContextMenu(wxWindow *pWnd, const wxPoint &point)
{
    UNUSED(pWnd);
    UNUSED(point);

#if 0
    CMenu menu;

    if (!menu.LoadMenu(IDR_POPUP_EDIT))
        return;

    CMenu *pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    if (point.x == -1 && point.y == -1) // Menu accessed via keyboard?
    {
        CRect rect;
        GetClientRect(rect);

        point = rect.TopLeft();
        wxPoint ptTopLeft(0, 0);
        ClientToScreen(&ptTopLeft);

        point.x = ptTopLeft.x + rect.Width() / 2; // Position ourselves in the middle of the window
        point.y = ptTopLeft.y + rect.Height() / 2;
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
#endif
}

void CSrc6502View::OnRemoveErrMark()
{
    SetErrMark(-1); // erase the pointer, wrong line
}

// return breakpoint info for line 'nLine'
//
uint8_t CSrc6502View::GetBreakpoint(int nLine) const
{
    auto search = m_mapBreakpoints.find(nLine);

    if (search != m_mapBreakpoints.end())
        return search->second;

    return 0;
}

CSrc6502Doc *CSrc6502View::GetDocument(void)
{
    return dynamic_cast<CSrc6502Doc *>(wxView::GetDocument());
}

#ifdef USE_CRYSTAL_EDIT
CCrystalTextBuffer *CSrc6502View::LocateTextBuffer()
{
    return GetDocument()->GetBuffer();
}

void CSrc6502View::DrawMarginMarker(int nLine, CDC *pDC, const CRect &rect)
{
    // Really this shouldn't be here, we should just use the LeftBar. -- B.Simonds (April 26, 2024)

    int nLeft = rect.left + rect.Width() / 6;

    if (BYTE bp = GetBreakpoint(nLine))
        CMarks.draw_breakpoint(*pDC, nLeft, rect.top, rect.Height(), !(bp & CAsm::BPT_DISABLED));

    if (nLine == GetPointerLine())
        CMarks.draw_pointer(*pDC, nLeft, rect.top, rect.Height());

    if (nLine == GetErrorMarkLine())
        CMarks.draw_mark(*pDC, nLeft, rect.top, rect.Height());
}


CCrystalEditView::LineChange CSrc6502View::NotifyEnterPressed(CPoint ptCursor, std::string &strLine)
{
    LineChange eChange = CCrystalEditView::NOTIF_NO_CHANGES;

    if (m_bAutoSyntax || m_bAutoUppercase)
    {
        int start = 0, fin = 0;
        CAsm::Stat stat = CAsm::OK;
        std::string strMsg;

        check_line(strLine, stat, start, fin, strMsg);

        if (m_bAutoUppercase && start > 0 && fin > 0)	// jest instrukcja do zamiany na du�e litery?
        {
            for (int nIndex = start; nIndex < fin; ++nIndex)
            {
                char c = strLine[nIndex];
                char u = toupper(c);
                if (c != u)
                {
                    strLine.SetAt(nIndex, u);
                    eChange = NOTIF_LINE_MODIFIED;
                }
            }
        }

        if (m_bAutoSyntax && stat != CAsm::OK)
        {
            disp_warning(ptCursor.y, strMsg);
            eChange = NOTIF_LINE_ERROR;
        }
    }

    return eChange;
}

void CSrc6502View::NotifyTextChanged() // 1.3.3 added * for changed file marker 1.3.3.4 - removed ;)
{
    OnEnUpdate();
}

wxColor CSrc6502View::GetColor(int nColorIndex)
{
    switch (nColorIndex)
    {
    case COLORINDEX_WHITESPACE:
    case COLORINDEX_BKGND:
        return s_rgbBkgndColor;

    case COLORINDEX_NORMALTEXT:
        return s_rgbTextColor;

    case COLORINDEX_KEYWORD:		// instructions
        return s_vrgbColorSyntax[0];
    case COLORINDEX_PREPROCESSOR:	// directives
        return s_vrgbColorSyntax[1];
    case COLORINDEX_COMMENT:
        return s_vrgbColorSyntax[2];
    case COLORINDEX_NUMBER:
        return s_vrgbColorSyntax[3];
    case COLORINDEX_STRING:
        return s_vrgbColorSyntax[4];
    case COLORINDEX_OPERATOR:
        return s_vrgbColorSyntax[6];
    case COLORINDEX_SELBKGND:
        return s_vrgbColorSyntax[5];

    default:
        return CBaseView::GetColor(nColorIndex);
    }
}

BOOL CSrc6502View::GetBold(int nColorIndex)
{
    switch (nColorIndex)
    {
    case COLORINDEX_KEYWORD:		// instructions
        return m_vbyFontStyle[0] & 1;
    case COLORINDEX_PREPROCESSOR:	// directives
        return m_vbyFontStyle[1] & 1;
    case COLORINDEX_COMMENT:
        return m_vbyFontStyle[2] & 1;
    case COLORINDEX_NUMBER:
        return m_vbyFontStyle[3] & 1;
    case COLORINDEX_STRING:
        return m_vbyFontStyle[4] & 1;

    default:
        return CBaseView::GetBold(nColorIndex);
    }
}

void CSrc6502View::CaretMoved(const std::string &strLine, int nWordStart, int nWordEnd)
{
    if (m_pMainFrame)
        m_pMainFrame->ShowDynamicHelp(strLine, nWordStart, nWordEnd);

    //	TRACE("%s\n", strWord.IsEmpty() ? "=":(const char*)strWord);
}

#endif
