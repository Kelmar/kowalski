////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextView.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Interface of the CCrystalTextView class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef CCRYSTALTEXTVIEW_H__
#define CCRYSTALTEXTVIEW_H__

#include "cedefs.h"

////////////////////////////////////////////////////////////////////////////
// Forward class declarations

class CCrystalTextBuffer;
class CUpdateContext;

//	Syntax coloring overrides
struct TEXTBLOCK
{
    int	m_nCharPos;
    int m_nColorIndex;
};

enum
{
    // Base colors
    COLORINDEX_WHITESPACE,
    COLORINDEX_BKGND,
    COLORINDEX_NORMALTEXT,
    COLORINDEX_SELMARGIN,
    COLORINDEX_SELBKGND,
    COLORINDEX_SELTEXT,
    COLORINDEX_ELLIPSIS,
    // Syntax colors
    COLORINDEX_KEYWORD,
    COLORINDEX_COMMENT,
    COLORINDEX_NUMBER,
    COLORINDEX_OPERATOR, // [JRT]:
    COLORINDEX_STRING,
    COLORINDEX_PREPROCESSOR,
    // Compiler/debugger colors
    COLORINDEX_ERRORBKGND,
    COLORINDEX_ERRORTEXT,
    COLORINDEX_EXECUTIONBKGND,
    COLORINDEX_EXECUTIONTEXT,
    COLORINDEX_BREAKPOINTBKGND,
    COLORINDEX_BREAKPOINTTEXT
    // ...
    // Expandable: custom elements are allowed.
};

////////////////////////////////////////////////////////////////////////////
// CCrystalTextView class declaration

//	CCrystalTextView::FindText() flags
enum
{
    FIND_MATCH_CASE		= 0x0001,
    FIND_WHOLE_WORD		= 0x0002,
    FIND_DIRECTION_UP	= 0x0010,
    REPLACE_SELECTION	= 0x0100
};

//	CCrystalTextView::UpdateView() flags
enum
{
    UPDATE_HORZRANGE	= 0x0001,		//	update horz scrollbar
    UPDATE_VERTRANGE	= 0x0002,		//	update vert scrollbar
    UPDATE_SINGLELINE	= 0x0100,		//	single line has changed
    UPDATE_FLAGSONLY	= 0x0200,		//	only line-flags were changed

    UPDATE_RESET		= 0x1000		//	document was reloaded, update all!
};

class CRYSEDIT_CLASS_DECL CCrystalTextView : public wxView
{
private:
    // Search parameters
    bool m_bLastSearch;
    uint32_t m_dwLastSearchFlags;
    const char *m_pszLastFindWhat;
    bool m_bMultipleSearch; // More search

    bool m_bCursorHidden;

    // Painting caching bitmap
    wxBitmap *m_pCacheBitmap;

    // Line/character dimensions
    int m_nLineHeight, m_nCharWidth;
    void CalcLineCharDim();

    // Text attributes
    int m_nTabSize;
    bool m_bViewTabs;
    bool m_bSelMargin;

    // Amount of lines/characters that completely fits the client area
    int m_nScreenLines, m_nScreenChars;

    int m_nMaxLineLength;
    int m_nIdealCharPos;

    bool m_bFocused;
    wxPoint m_ptAnchor;
    wxFontInfo m_lfBaseFont;
    wxFont *m_apFonts[4];

    // Parsing stuff
    uint32_t *m_pdwParseCookies;
    int m_nParseArraySize;
    uint32_t GetParseCookie(int nLineIndex);

    // Pre-calculated line lengths (in characters)
    int m_nActualLengthArraySize;
    int *m_pnActualLineLength;

    bool m_bPreparingToDrag;
    bool m_bDraggingText;
    bool m_bDragSelection, m_bWordSelection, m_bLineSelection;
    UINT m_nDragSelTimer;
    wxPoint WordToRight(wxPoint pt);
    wxPoint WordToLeft(wxPoint pt);

    wxPoint m_ptDrawSelStart, m_ptDrawSelEnd;
    wxPoint m_ptCursorPos;
    wxPoint m_ptSelStart, m_ptSelEnd;
    void PrepareSelBounds();

    // Helper functions
    void ExpandChars(const char *pszChars, int nOffset, int nCount, std::string &line);

    int ApproxActualOffset(int nLineIndex, int nOffset);
    void AdjustTextPoint(wxPoint &point);
    void DrawLineHelperImpl(wxDC *pdc, wxPoint &ptOrigin, const wxRect &rcClip,
                            const char * pszChars, int nOffset, int nCount);
    bool IsInsideSelBlock(wxPoint ptTextPos);

    bool m_bBookmarkExist; // More bookmarks

    // collapsible blocks
    bool IsFirstLineOfCollapsedBlock(int nLine);
    int CalcVisibleLines();
    int CalcVisibleLines(int nFromLine, int nToLine);

    void FindBlockBoundaries(int nTopLine, int nLines);
    std::vector<bool> m_vLineInsideBlock;

protected:
    //CImageList *m_pIcons;

    CCrystalTextBuffer *m_pTextBuffer;
    //HACCEL m_hAccel;

    bool m_bVertScrollBarLocked, m_bHorzScrollBarLocked;
    wxPoint m_ptDraggedTextBegin, m_ptDraggedTextEnd;
    virtual void ResetView();
    void UpdateCaret();
    void SetAnchor(const wxPoint &ptNewAnchor);
    int GetMarginWidth();

    bool m_bShowInactiveSelection;
    // [JRT]
    bool m_bDisableDragAndDrop;

    wxPoint ClientToText(const wxPoint &point);
    wxPoint TextToClient(const wxPoint &point);
    void InvalidateLines(int nLine1, int nLine2, bool bInvalidateMargin = false);
    int CalculateActualOffset(int nLineIndex, int nCharIndex);

    // Printing
    int m_nPrintPages;
    int *m_pnPages;
    wxFont *m_pPrintFont;
    int m_nPrintLineHeight;
    bool m_bPrintHeader, m_bPrintFooter;
    wxRect m_ptPageArea, m_rcPrintArea;
    int PrintLineHeight(wxDC *pdc, int nLine);
#if 0
    void RecalcPageLayouts(wxDC *pdc, CPrintInfo *pInfo);
#endif
    virtual void PrintHeader(wxDC *pdc, int nPageNum);
    virtual void PrintFooter(wxDC *pdc, int nPageNum);
    virtual void GetPrintHeaderText(int nPageNum, std::string &text);
    virtual void GetPrintFooterText(int nPageNum, std::string &text);

    // Keyboard handlers
    void MoveLeft(bool bSelect);
    void MoveRight(bool bSelect);
    void MoveWordLeft(bool bSelect);
    void MoveWordRight(bool bSelect);
    void MoveUp(bool bSelect);
    void MoveDown(bool bSelect);
    void MoveHome(bool bSelect);
    void MoveEnd(bool bSelect);
    void MovePgUp(bool bSelect);
    void MovePgDn(bool bSelect);
    void MoveCtrlHome(bool bSelect);
    void MoveCtrlEnd(bool bSelect);

    void SelectAll();
    void Copy();

    bool IsSelection();
    bool IsInsideSelection(const wxPoint &ptTextPos);
    void GetSelection(wxPoint &ptStart, wxPoint &ptEnd);
    void SetSelection(const wxPoint &ptStart, const wxPoint &ptEnd);

    int m_nTopLine, m_nOffsetChar;
    bool m_bSmoothScroll;

    int GetLineHeight();
    int GetCharWidth();
    int GetMaxLineLength();
    int GetScreenLines();
    int GetScreenChars();
    wxFont* GetFont(bool bItalic = false, bool bBold = false);

    void RecalcVertScrollBar(bool bPositionOnly = false);
    void RecalcHorzScrollBar(bool bPositionOnly = false);

    // Scrolling helpers
    void ScrollToChar(int nNewOffsetChar, bool bNoSmoothScroll = false, bool bTrackScrollBar = true);
    void ScrollToLine(int nNewTopLine, bool bNoSmoothScroll = false, bool bTrackScrollBar = true);

    // Splitter support
    virtual void UpdateSiblingScrollPos(bool bHorz);
    virtual void OnUpdateSibling(CCrystalTextView *pUpdateSource, bool bHorz);
    CCrystalTextView *GetSiblingView(int nRow, int nCol);

    virtual int GetLineCount();
    virtual int GetLineLength(int nLineIndex);
    virtual int GetLineActualLength(int nLineIndex);
    virtual const char *GetLineChars(int nLineIndex);
    virtual uint32_t GetLineFlags(int nLineIndex);
    virtual void GetText(const wxPoint &ptStart, const wxPoint &ptEnd, std::string &text);
    std::string GetCurLine();

    // Clipboard overridable
    virtual bool TextInClipboard();
    virtual bool PutToClipboard(const std::string &text);
    virtual bool GetFromClipboard(std::string &text);

    // Drag-n-drop overrideable
#if 0
    virtual HGLOBAL PrepareDragData();
    virtual DROPEFFECT GetDropEffect();
    virtual void OnDropSource(DROPEFFECT de);
#endif
    bool IsDraggingText() const;

    virtual wxColour GetColor(int nColorIndex);
    virtual void GetLineColors(int nLineIndex, wxColour &crBkgnd,
                               wxColour &crText, bool &bDrawWhitespace);
    virtual bool GetItalic(int nColorIndex);
    virtual bool GetBold(int nColorIndex);

    void DrawLineHelper(wxDC *pdc, wxPoint &ptOrigin, const wxRect &rcClip, int nColorIndex,
                        const char *pszChars, int nOffset, int nCount, wxPoint ptTextPos);
    virtual void DrawSingleLine(wxDC *pdc, const wxRect &rect, int nLineIndex);
    virtual void DrawMargin(wxDC *pdc, const wxRect &rect, int nLineIndex);
    void DrawEllipsis(wxDC* pDC, const wxRect& rcLine);

    // margin icon drawing helper fn
    void DrawMarginIcon(wxDC* pDC, const wxRect &rect, int nImageIndex);
    virtual void DrawMarginMarker(int nLine, wxDC* pDC, const wxRect &rect);

    virtual uint32_t ParseLine(uint32_t dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems);

    //virtual HINSTANCE GetResourceHandle();

    // MiK: collapsible block methods
    void MarkCollapsibleBlockLine(int nLineIndex, bool bStart);
    void ClearCollapsibleBlockMark(int nLineIndex);
    //
    bool CollapseBlock(int nLineInsideBlock);
    bool CollapseBlock(int nLineFrom, int nLineTo);
    void CollapseAllBlocks(bool bCollapse);
    //
    void GoToLine(int nLineIndex, int nCharOffset = 0);
    bool FindCollapsibleBlock(int nLineInsideBlock, int& nLineFrom, int& nLineTo);
    bool IsLineHidden(int nLine);
    bool IsLineCollapsed(int nLine);
    void ExpandCurrentLine();
    void ExpandLine(int nLine);
    int FindVisibleLine(int nTopLine, int bDelta);

// Attributes
public:
    bool GetViewTabs();
    void SetViewTabs(bool bViewTabs);
    int GetTabSize();
    void SetTabSize(int nTabSize);
    bool GetSelectionMargin();
    void SetSelectionMargin(bool bSelMargin);
    void GetFont(wxFontInfo &lf);
    void SetFont(const wxFontInfo &lf);
    bool GetSmoothScroll() const;
    void SetSmoothScroll(bool bSmoothScroll);
    // [JRT]:
    bool GetDisableDragAndDrop() const;
    void SetDisableDragAndDrop(bool bDDAD);

    // Default handle to resources
    //static HINSTANCE s_hResourceInst;

// Operations
public:
    void AttachToBuffer(CCrystalTextBuffer *pBuf = nullptr);
    void DetachFromBuffer();

    // Buffer-view interaction, multiple views
    virtual CCrystalTextBuffer *LocateTextBuffer();
    virtual void UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext, uint32_t dwFlags, int nLineIndex = -1);
    virtual void NotifyTextChanged();
    virtual void CaretMoved(const std::string& strLine, int nWordStart, int nWordEnd);

    // Attributes
    wxPoint GetCursorPos();
    void SetCursorPos(const wxPoint &ptCursorPos);
    void ShowCursor();
    void HideCursor();

    // Operations
    void EnsureVisible(wxPoint pt);

    //	Text search helpers
    bool FindText(const char *pszText, const wxPoint &ptStartPos, uint32_t dwFlags, bool bWrapSearch, wxPoint *pptFoundPos);
    bool FindTextInBlock(const char *pszText, const wxPoint &ptStartPos, const wxPoint &ptBlockBegin, const wxPoint &ptBlockEnd,
                         uint32_t dwFlags, bool bWrapSearch, wxPoint *pptFoundPos);
    bool HighlightText(const wxPoint &ptStartPos, int nLength);

    //	Overridable: an opportunity for Auto-Indent, Smart-Indent etc.
    virtual void OnEditOperation(int nAction, const char *pszText);

public:
    virtual void OnDraw(wxDC* pDC);  // overridden to draw this view

#if 0
    virtual bool PreCreateWindow(CREATESTRUCT& cs);
    virtual bool PreTranslateMessage(MSG* pMsg);
    virtual void OnPrepareDC(wxDC* pDC, CPrintInfo* pInfo = NULL);
#endif

protected:
    virtual void OnInitialUpdate(); // called first time after construct

#if 0
    virtual bool OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(wxDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(wxDC* pDC, CPrintInfo* pInfo);
    virtual void OnPrint(wxDC* pDC, CPrintInfo* pInfo);
#endif

// Implementation
public:
    /* constructor */ CCrystalTextView();
    virtual          ~CCrystalTextView();

protected:

// Generated message map functions
protected:
#ifdef _DEBUG
    void AssertValidTextPos(const wxPoint &pt);
#endif

    //{{AFX_MSG(CCrystalTextView)
    afx_msg void OnDestroy();
    afx_msg bool OnEraseBkgnd(wxDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar);
    afx_msg bool OnSetCursor(wxWindow* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnLButtonDown(UINT nFlags, wxPoint point);
    afx_msg void OnSetFocus(wxWindow* pOldWnd);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, wxScrollBar* pScrollBar);
    afx_msg void OnLButtonUp(UINT nFlags, wxPoint point);
    afx_msg void OnMouseMove(UINT nFlags, wxPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnKillFocus(wxWindow* pNewWnd);
    afx_msg void OnLButtonDblClk(UINT nFlags, wxPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
    afx_msg void OnRButtonDown(UINT nFlags, wxPoint point);
    afx_msg void OnSysColorChange();
    //afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnEditFind();
    afx_msg void OnEditRepeat();
    afx_msg void OnUpdateEditRepeat(CCmdUI* pCmdUI);
    afx_msg void OnEditFindPrevious();                 // More search
    afx_msg void OnUpdateEditFindPrevious(CCmdUI* pCmdUI);
    afx_msg bool OnMouseWheel(UINT nFlags, short zDelta, wxPoint pt);
    afx_msg void OnEditViewTabs();
    afx_msg void OnUpdateEditViewTabs(CCmdUI* pCmdUI);
    //}}AFX_MSG
    afx_msg void OnFilePageSetup();

    afx_msg void OnCharLeft();
    afx_msg void OnExtCharLeft();
    afx_msg void OnCharRight();
    afx_msg void OnExtCharRight();
    afx_msg void OnWordLeft();
    afx_msg void OnExtWordLeft();
    afx_msg void OnWordRight();
    afx_msg void OnExtWordRight();
    afx_msg void OnLineUp();
    afx_msg void OnExtLineUp();
    afx_msg void OnLineDown();
    afx_msg void OnExtLineDown();
    afx_msg void OnPageUp();
    afx_msg void OnExtPageUp();
    afx_msg void OnPageDown();
    afx_msg void OnExtPageDown();
    afx_msg void OnLineEnd();
    afx_msg void OnExtLineEnd();
    afx_msg void OnHome();
    afx_msg void OnExtHome();
    afx_msg void OnTextBegin();
    afx_msg void OnExtTextBegin();
    afx_msg void OnTextEnd();
    afx_msg void OnExtTextEnd();
    afx_msg void OnUpdateIndicatorCRLF(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorPosition(CCmdUI* pCmdUI);
    afx_msg void OnToggleBookmark(UINT nCmdID);
    afx_msg void OnGoBookmark(UINT nCmdID);
    afx_msg void OnClearBookmarks();

    afx_msg void OnToggleBookmark();	// More bookmarks
    afx_msg void OnClearAllBookmarks();
    afx_msg void OnNextBookmark();
    afx_msg void OnPrevBookmark();
    afx_msg void OnUpdateClearAllBookmarks(CCmdUI* pCmdUI);
    afx_msg void OnUpdateNextBookmark(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePrevBookmark(CCmdUI* pCmdUI);
    afx_msg void OnGotoLineNumber();

    afx_msg void ScrollUp();
    afx_msg void ScrollDown();
    afx_msg void ScrollLeft();
    afx_msg void ScrollRight();

    afx_msg void OnToggleCollapsibleBlock();
    afx_msg void OnCollapseAllBlocks();
    afx_msg void OnExpandAllBlocks();

    void CaretMoved();
};

#ifdef _DEBUG
#define ASSERT_VALIDTEXTPOS(pt)		AssertValidTextPos(pt);
#else
#define ASSERT_VALIDTEXTPOS(pt)
#endif

#if ! (defined(CE_FROM_DLL) || defined(CE_DLL_BUILD))
#include "CCrystalTextView.inl"
#endif

#endif /* CCRYSTALTEXTVIEW_H__ */
