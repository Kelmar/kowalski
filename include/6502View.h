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

// 6502View.h : interface of the CSrc6502View class
//
/////////////////////////////////////////////////////////////////////////////

#include "DrawMarks.h"

class CSrc6502Doc;

/**
 * View for assembly source code.
 */
class CSrc6502View : public wxView
{
private:
    wxDECLARE_DYNAMIC_CLASS(CSrc6502View);

    friend class CSrc6502Doc;

    wxFrame *m_frame;
    wxStyledTextCtrl *m_text;
    wxStatusBar *m_status;

    std::unordered_map<int, uint8_t> m_mapBreakpoints; // TODO: move to the doc

    int m_nActualPointerLine;	// TODO: move to the doc
    int m_nActualErrMarkLine;	// TODO: move to the doc

    //static LRESULT CALLBACK EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void drawMark(wxDC &dc, int line, MarkType type, bool scroll);
    //void draw_breakpoints(HDC hDC = NULL);

    //void DrawMark(int line, MarkType type, bool scroll = false, HDC hDC = NULL);

    void EraseMark(int line);

    static void check_line(const char *buf, CAsm::Stat &stat, int &start, int &fin, std::string &msg);
    void disp_warning(int line, const std::string &msg);
    void OnRemoveErrMark();

    void UpdatePositionInfo();

public:
    //static wxFont s_Font;
    //static wxFontInfo s_LogFont;

    static wxColour s_rgbTextColor;
    static wxColour s_rgbBkgndColor;

    static wxFontInfo s_logFont;
    static wxFont s_font;

    static bool m_bAutoIndent;
    static int m_nTabStep;
    static bool m_bAutoSyntax;
    static bool m_bAutoUppercase;

    static wxColour s_vrgbColorSyntax[];

    static uint8_t m_vbyFontStyle[];
        
public:
    /* constructor */ CSrc6502View();
    virtual          ~CSrc6502View();

    void RemoveBreakpoint(int line, bool draw = true);
    void AddBreakpoint(int line, CAsm::Breakpoint bp, bool draw = true);
    int GetCurrLineNo();
    void SetErrMark(int line); // draw/erase the pointer arrow error
    void SetPointer(int line, bool scroll = false);

    void SelectEditFont();

    CSrc6502Doc *GetDocument();

    bool OnCreate(wxDocument *doc, long flags) override;

    //LRESULT OnPaintPointer(WPARAM /* wParam */, LPARAM /* lParam */);

    // edit view info
    void GetDispInfo(int &nTopLine, int &nLineCount, int &nLineHeight);

    int GetPointerLine() const
    {
        return m_nActualPointerLine;
    }

    int GetErrorMarkLine() const
    {
        return m_nActualErrMarkLine;
    }

    // return breakpoint info for line 'nLine'
    uint8_t GetBreakpoint(int nLine) const;

    int GetLineCount() const { return m_text ? m_text->GetLineCount() : 0; }
    wxString GetText() const { return m_text ? m_text->GetText() : wxString(""); }

public:
    void OnDraw(wxDC *dc) override;

protected:
#if REWRITE_TO_WX_WIDGET
    virtual bool OnPreparePrinting(CPrintInfo *pInfo);
    virtual void OnBeginPrinting(wxDC *pDC, CPrintInfo *pInfo);
    virtual void OnEndPrinting(wxDC *pDC, CPrintInfo *pInfo);
#endif

protected:
    void OnTextUpdate(wxCommandEvent &);

    void OnEnUpdate();
    void OnContextMenu(wxWindow *pWnd, const wxPoint &point);

private:

#ifdef USE_CRYSTAL_EDIT
    virtual CCrystalTextBuffer *LocateTextBuffer();
    virtual uint32_t ParseLine(uint32_t dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems);
    virtual void DrawMarginMarker(int nLine, wxDC *pDC, const CRect &rect);
    virtual LineChange NotifyEnterPressed(CPoint ptCursor, std::string &strLine);
    virtual void NotifyTextChanged();
    virtual wxColour GetColor(int nColorIndex);
    virtual bool GetBold(int nColorIndex);
    virtual void CaretMoved(const std::string &strLine, int nWordStart, int nWordEnd);
#endif
};

/////////////////////////////////////////////////////////////////////////////
