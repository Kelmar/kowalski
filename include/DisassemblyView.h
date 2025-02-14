/*=======================================================================*/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*=======================================================================*/

#ifndef DISASSEMBLY_VIEW_6502_H__
#define DISASSEMBLY_VIEW_6502_H__

/*=======================================================================*/

struct DisassembleInfo;

/*=======================================================================*/

/**
 * @brief Core disassembly view
 */
class DisassemblyView : public wxScrolled<wxWindow>
{
private:
    sim_addr_t m_offset; // Current scroll offset

private: // Drawing state variables
    /// Current program address
    sim_addr_t m_programAddress;

    /// Size of a character
    wxSize m_charSize;

    /// Current drawing position (in pixels)
    int m_drawLine;

private:
    void Init();

public:
    /* constructor */ DisassemblyView();

    /* constructor */ DisassemblyView(
        wxWindow *parent,
        wxWindowID id,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        const wxString &name = wxControlNameStr);

    virtual ~DisassemblyView();

private:
    void OnPaint(wxPaintEvent &);

    void DrawLineMarks(wxDC &, DisassembleInfo &);
    void DrawLine(wxDC &, DisassembleInfo &);
};

/*=======================================================================*/

#endif /* DISASSEMBLY_VIEW_6502_H__ */

/*=======================================================================*/
