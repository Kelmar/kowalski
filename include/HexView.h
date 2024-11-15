/*************************************************************************/
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
 /*************************************************************************/

#ifndef HEXVIEW_H__
#define HEXVIEW_H__

/*************************************************************************/

#include <sigslot/signal.hpp>

#include "OutputMem.h"

/*************************************************************************/

class HexView : public wxScrolled<wxWindow>
{
private:
    static const int LINE_WIDTH = 16; // Values per line
    sigslot::connection m_memoryConnection;

    /**
     * @brief View into memory to dump.
     */
    COutputMem *m_memory;

    /**
     * @brief The overall "virtual" size of our control.
     */
    wxSize m_virtualSize;

    /**
     * @brief Defines how memory page boundaries should shown.
     *
     * @remarks 0 indicates no page boundaries, otherwise a page
     * boundary is shown every pageBoundary bytes.
     */
    unsigned int m_pageSize;

    uint32_t m_selStart;
    uint32_t m_selLen;

    wxPoint m_mouseDn; //< Save's where the mouse was first pressed down.
    wxString m_addrFmt;
    bool m_mouseTrack;

    int CalcAddressChars() const;

    void CalcAddressFormat();

    int CalcAddressFromCell(wxPoint &p) const
    {
        return p.x + p.y * LINE_WIDTH;
    }

    wxPoint GetHitCell(const wxPoint &p) const;

    void UpdateSelection(wxMouseEvent &);

    void OnMouseDown(wxMouseEvent &);
    void OnMouseUp(wxMouseEvent &);
    void OnMouseMove(wxMouseEvent &);

    void OnPaint(wxPaintEvent &);
    void Draw(wxDC &dc);

    void CalculateScrollInfo();
    void UpdateScrollInfo();

    void Init();

    inline
        void MemoryUpdated() { Refresh(); }

public:
    /* constructor */ HexView();

    /* constructor */ HexView(wxWindow *parent, wxWindowID id,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        const wxString &name = wxControlNameStr);

    virtual ~HexView();

    uint32_t GetAddress() const;

    void JumpTo(uint32_t address);

    void SetMemory(COutputMem *mem)
    {
        if (m_memoryConnection.connected())
            m_memoryConnection.disconnect();

        m_memory = mem;

        if (m_memory)
            m_memoryConnection = m_memory->onUpdate.connect(&HexView::MemoryUpdated, this);

        UpdateScrollInfo();
    }

    unsigned int GetPageSize(void) const { return m_pageSize; }

    void SetPageSize(unsigned int pageSize)
    {
        m_pageSize = pageSize;
        UpdateScrollInfo();
    }
};

/*************************************************************************/

#endif /* HEXVIEW_H__ */

/*************************************************************************/
