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

#ifndef IO_WINDOW_H__
#define IO_WINDOW_H__

/*************************************************************************/

#include "Asm.h"
#include "Broadcast.h"

/*************************************************************************/

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
    static constexpr size_t BUF_SIZE = 32 * 1024;

    char m_vchBuffer[BUF_SIZE];
    char* m_pHead;
    char* m_pTail;
};

/*************************************************************************/

class CIOWindow : public wxFrame
{
private:
    static constexpr char REG_ENTRY_IOWINDOW[] = "IOWindow";

    /**
     * @brief Simulated video memory.
     */
    uint8_t *m_memory;

    /**
     * @brief Window size in characters.
     */
    wxSize m_charCnt;

    /**
     * @brief Current location of the cursor.
     */
    wxPoint m_cursorPos;

    /**
     * @brief Cursor on/off flag
     */
    bool m_showCursor;

    int m_nCursorCount;    // Counter hide cursor
    bool m_bCursorVisible; // flag: cursor currently visible

    UINT m_uTimer;

    CInputBuffer m_InputBuffer; // keyboard input buffer
public:
    /* constructor */ CIOWindow(wxWindow *parent);
    virtual          ~CIOWindow();

    int put(char chr, int x, int y);
    int scroll(int dy); // shift the strings by 'dy' lines

    /**
     * @brief Invalidate a single character cell in the IOWindow
     * @param x, y The character to invalidate
     */
    void Invalidate(int x, int y);

public: // Attributes
    static wxColour m_rgbTextColor, m_rgbBackgndColor;

    void SetSize(int w, int h, bool resize = true);
    void GetSize(int &w, int &h) const
    {
        w = m_charCnt.x;
        h = m_charCnt.y;
    }

    void Paste();

    void SetColors(wxColour text, wxColour background);
    void GetColors(wxColour &text, wxColour &background);

    int PutC(int chr);                       // Print the character
    int PutChr(int chr);                     // Print character (verbatim)
    int PutS(const char *str, int len = -1); // String of characters to print
    int PutH(int n);                         // Print hex number (8 bits)
    bool SetPosition(int x, int y);          // Set the position for the text
    void GetPosition(int &x, int &y);        // Read position
    bool Cls();                              // Clear the window
    int Input();

public:
    //virtual bool PreTranslateMessage(MSG* pMsg);

protected: // Event Processing
    void OnPaint(wxPaintEvent &);
    void Draw(wxDC &dc);
    void DrawCursor(wxDC &dc);

    void OnClose(wxCloseEvent &);

    // Generated message map functions
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnContextMenu(wxWindow* pWnd, wxPoint point);
    afx_msg void OnPaste();
    virtual bool ContinueModal();

    afx_msg LRESULT OnCls(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPutC(WPARAM wParam, LPARAM /* lParam */);
    afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnInput(WPARAM /*wParam*/, LPARAM /* lParam */);
    afx_msg LRESULT OnPosition(WPARAM wParam, LPARAM lParam);
};

/*************************************************************************/

#endif /* IO_WINDOW_H__ */

/*************************************************************************/
