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

// IOWindow.h : header file
//

#ifndef IO_WINDOW_H__
#define IO_WINDOW_H__

#include "Asm.h"
#include "Broadcast.h"

/////////////////////////////////////////////////////////////////////////////
// CIOWindow frame

class CInputBuffer
{
public:
    CInputBuffer()
        : m_pHead(m_vchBuffer)
        , m_pTail(m_vchBuffer)
    {
    }

    /**
     * get next available character (returns 0 if there are no chars)
     */
    char GetChar();

    /**
     * places char in the buffer (char is ignored if there is no space)
     */
    void PutChar(char c);

    /**
     * paste clipboard text into buffer
     */
    void Paste(const char* pcText);

private:
    enum { BUF_SIZE = 32 * 1024 };
    char m_vchBuffer[BUF_SIZE];
    char* m_pHead;
    char* m_pTail;
};

class CIOWindow : public wxWindow //CMiniFrameWnd
{
    uint8_t *m_pData;           // Window memory
    int m_nWidth, m_nHeight;    // Window size (columns x rows)
    int m_nCharH, m_nCharW;     // Character size
    static std::string m_strClass;
    static bool m_bRegistered;
    void RegisterWndClass();
    int m_nPosX, m_nPosY;       // Location of the character to print (and cursor)
    int m_nCursorCount;         // Counter hide cursor
    bool m_bCursorOn;           // flag: cursor on/off
    bool m_bCursorVisible;      // flag: cursor currently visible
    UINT m_uTimer;

    CInputBuffer m_InputBuffer; // keyboard input buffer

    int put(char chr, int x, int y);
    int puts(const char *str, int len, int x, int y);
    int scroll(int dy);//shift the strings by 'dy' lines
    int invalidate(int x, int y);//the area of ​​the character under (x,y) to redraw

    afx_msg LRESULT OnCls(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPutC(WPARAM wParam, LPARAM /* lParam */);
    afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnInput(WPARAM /*wParam*/, LPARAM /* lParam */);
    afx_msg LRESULT OnPosition(WPARAM wParam, LPARAM lParam);

public:
    static bool m_bHidden;

    /* constructor */ CIOWindow();           // protected constructor used by dynamic creation
    virtual          ~CIOWindow();

    // Attributes
public:
    static wxFont m_Font;
    static wxFontInfo m_LogFont;
    static wxPoint m_WndPos;     // Window position
    static int m_nInitW, m_nInitH;
    static wxColour m_rgbTextColor, m_rgbBackgndColor;

    // Operations
private:
    void CalcFontSize();        // Calculate character size
    void DrawCursor();          // draw cursor
    void DrawCursor(int nX, int nY, bool bVisible);
    void HideCursor();			// hide cursor if it's on

public:

    enum Commands // Commands for the terminal window
    { 
        CMD_CLS = 100 //CBroadcast::WM_USER_OFFSET + 100,
        CMD_PUTC, 
        CMD_PUTS, 
        CMD_IN, 
        CMD_POSITION
    };

    bool Create();
    void SetSize(int w, int h, int resize = 1);
    void GetSize(int &w, int &h);
    void Resize();
    void SetWndPos(const wxPoint &p);
    wxPoint GetWndPos();
    void Paste();

    void SetColors(wxColour text, wxColour backgnd);
    void GetColors(wxColour &text, wxColour &backgnd);

    int PutC(int chr);                       // Print the character
    int PutChr(int chr);                     // Print character (verbatim)
    int PutS(const char *str, int len = -1); // String of characters to print
    int PutH(int n);                         // Print hex number (8 bits)
    bool SetPosition(int x, int y);          // Set the position for the text
    void GetPosition(int &x, int &y);        // Read position
    bool ShowCursor(bool bVisible = TRUE);   // Enable/disable the cursor
    bool ResetCursor();                      // Turns on the cursor, resets the hide counter
    bool Cls();                              // Clear the window
    int Input();

    bool IsWaiting() const;
    void ExitModalLoop();

public:
    //virtual bool PreTranslateMessage(MSG* pMsg);

protected:
    virtual void PostNcDestroy();

    // Generated message map functions
    afx_msg void OnPaint();
    //afx_msg void OnGetMinMaxInfo(MINMAXINFO* pMMI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnClose();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    afx_msg void OnPaste();
    virtual bool ContinueModal();
};

/////////////////////////////////////////////////////////////////////////////

#endif /* IO_WINDOW_H__ */
