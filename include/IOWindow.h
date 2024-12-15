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
    { }

    /**
     * get next available character (returns 0 if there are no chars)
     */
    char GetChar();

    /**
     * @brief Get next available character waiting without advancing the read pointer.
     * @return The next character or 0 if no characters available.
     */
    char PeekChar() const;

    /**
     * places char in the buffer (char is ignored if there is no space)
     */
    void PutChar(char c);

    /**
     * paste clipboard text into buffer
     */
    void Paste(const char *pcText);

private:
    static constexpr size_t BUF_SIZE = 32 * 1024;

    char m_vchBuffer[BUF_SIZE];
    char *m_pHead;
    char *m_pTail;
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
     * @remarks This is controlled externally.
     */
    bool m_showCursor;

    /**
     * @brief Cursor blink state flag.
     * @remarks This is updated by a timer.
     */
    bool m_cursorBlinkState;

    wxTimer m_timer;

    CInputBuffer m_InputBuffer; // keyboard input buffer

    /**
     * @brief Invalidate a single character cell in the IOWindow
     * @param location The character location to invalidate
     */
    void Invalidate(const wxPoint &location);

public:
    /* constructor */ CIOWindow(wxWindow *parent);
    virtual          ~CIOWindow();

public: // Output functions
    /**
     * @brief Write a character directly to video memory at given location
     * @param chr Character to write
     * @param x, y Location to write to
     */
    void Put(char chr, int x, int y);

    /**
     * @brief Scroll the display
     * @param dy Number of lines to scroll the display by
     */
    void Scroll(int dy);

    /**
     * @brief Write a single character at the current cursor location.
     * @param chr The character to write.
     * @remarks Currently only processes control codes for CR, LF, and BS
     */
    void PutChar(int chr);

    /**
     * @brief Write a single character at the current cursor location (verbatim)
     * @param chr The character to write.
     */
    void RawChar(int chr);

    /**
     * @brief Print a string to the console at the cursor location.
     * @param str The string to print.
     * @param len
     */
    void PutStr(const wxString &str);

    /**
     * @brief Set the location of the cursor.
     * @param location
     * @return
     */
    void SetCursorPosition(const wxPoint &location);

    /**
     * @brief Get the current location of the cursor.
     */
    wxPoint GetCursorPosition() const { return m_cursorPos; }

    /**
     * @brief Clears the window and resets the cursor to the top left.
     */
    void Cls();

public: // Input functions
    /**
     * @brief Dump contents of the clipboard to the IOWindow
     */
    void Paste();

    /**
     * @brief Read input from the IOWindow
     * @return The key pressed, or 0 if no keys available.
     */
    int Input();

    /**
     * @brief Read input from IOWindow without consuming it.
     * @return The key pressed, or 0 of no keys available.
     */
    int PeekInput() const;

public: // Attributes
    wxColour m_rgbTextColor, m_rgbBackgndColor;

    /**
     * @brief Set the size of the display area in characters.
     * @param w, h The width and height to size to.
     * @param resize Set if the physical size fo the window should be changed as well.
     */
    void SetSize(int w, int h, bool resize = true);

    /**
     * @brief Get the size of the display area in characters.
     * @param w, h The width and height to return.
     */
    void GetSize(int &w, int &h) const
    {
        w = m_charCnt.x;
        h = m_charCnt.y;
    }

    void SetColors(wxColour text, wxColour background);
    void GetColors(wxColour &text, wxColour &background);

protected: // Event Processing
    void InitEvents();

    void OnPaint(wxPaintEvent &);
    void Draw(wxDC &dc);
    void DrawCursor(wxDC &dc);

    void BlinkCursor(wxTimerEvent &);

    void OnClose(wxCloseEvent &);

    // Generated message map functions
    void OnKeyDown(wxKeyEvent &);
    void OnChar(wxKeyEvent &);

    afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);
};

/*************************************************************************/

#endif /* IO_WINDOW_H__ */

/*************************************************************************/
