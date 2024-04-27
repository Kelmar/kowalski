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
#include "LeftBar.h"

class CSrc6502Doc;
class CMainFrame;

#ifdef USE_CRYSTAL_EDIT
//#include "CCrystalEditView.h"
//typedef CCrystalEditView CBaseView;
#else
//typedef CEditView CBaseView;
#endif

/**
 * View for assembly source code.
 */
class CSrc6502View : public wxView
{
private:
    //void set_position_info(HWND hWnd);

    std::unordered_map<int, uint8_t> m_mapBreakpoints; // TODO: move to the doc

    //static LRESULT (CALLBACK *m_pfnOldProc)(HWND, UINT, WPARAM, LPARAM);

    //static LRESULT CALLBACK EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void drawMark(wxDC &dc, int line, MarkType type, bool scroll);
    //void draw_breakpoints(HDC hDC = NULL);
    int ScrollToLine(int line, int &height, bool scroll = false);

    //void DrawMark(int line, MarkType type, bool scroll = false, HDC hDC = NULL);

    void RedrawMarks(int line = -1);
    void EraseMark(int line);

    int m_nActualPointerLine;	// TODO: move to the doc
    int m_nActualErrMarkLine;	// TODO: move to the doc

    static void check_line(const char *buf, CAsm::Stat &stat, int &start, int &fin, std::string &msg);
    void disp_warning(int line, std::string &msg);
    void OnRemoveErrMark();

public:
    //static wxFont s_Font;
    //static wxFontInfo s_LogFont;

    static wxColour s_rgbTextColor;
    static wxColour s_rgbBkgndColor;

    static bool m_bAutoIndent;
    static int m_nTabStep;
    static bool m_bAutoSyntax;
    static bool m_bAutoUppercase;

    static wxColour s_vrgbColorSyntax[];

    static uint8_t m_vbyFontStyle[];

    void SelectEditFont();

protected: // create from serialization only
    CSrc6502View();
    //DECLARE_DYNCREATE(CSrc6502View)

// Attributes
public:
    void RemoveBreakpoint(int line, bool draw = true);
    void AddBreakpoint(int line, CAsm::Breakpoint bp, bool draw = true);
    int GetCurrLineNo();
    void SetErrMark(int line); // draw/erase the pointer arrow error
    void SetPointer(int line, bool scroll = false);
    CSrc6502Doc* GetDocument();

    //LRESULT OnPaintPointer(WPARAM /* wParam */, LPARAM /* lParam */);

    // edit view info
    void GetDispInfo(int& nTopLine, int& nLineCount, int& nLineHeight);

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

#ifdef USE_CRYSTAL_EDIT
    using CBaseView::GetLineCount;
#else
    int GetLineCount()
    {
        //return GetEditCtrl().GetLineCount();
        return 0;
    }
#endif
    void GetText(std::string& strText);

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSrc6502View)
public:
    virtual void OnDraw(wxDC* pDC);  // overridden to draw this view
    //virtual bool PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
protected:
#if REWRITE_TO_WX_WIDGET
    virtual bool OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(wxDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(wxDC* pDC, CPrintInfo* pInfo);
#endif

    //virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSrc6502View();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    //{{AFX_MSG(CSrc6502View)
    //int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnEnUpdate();
    void OnContextMenu(wxWindow* pWnd, const wxPoint &point);

    // TODO: Remove this?  Not doing anything?
    void *CtlColor(wxDC* pDC, UINT nCtlColor);
    //}}AFX_MSG
    //DECLARE_MESSAGE_MAP()

private:
    CLeftBar m_wndLeftBar;
    CMainFrame* m_pMainFrame;

#ifdef USE_CRYSTAL_EDIT
    virtual CCrystalTextBuffer* LocateTextBuffer();
    virtual uint32_t ParseLine(uint32_t dwCookie, int nLineIndex, TEXTBLOCK* pBuf, int& nActualItems);
    virtual void DrawMarginMarker(int nLine, wxDC* pDC, const CRect &rect);
    virtual LineChange NotifyEnterPressed(CPoint ptCursor, std::string& strLine);
    virtual void NotifyTextChanged();
    virtual wxColour GetColor(int nColorIndex);
    virtual bool GetBold(int nColorIndex);
    virtual void CaretMoved(const std::string& strLine, int nWordStart, int nWordEnd);
#endif
};

/////////////////////////////////////////////////////////////////////////////
