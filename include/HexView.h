/*************************************************************************/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
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

class HexView : public wxControl
{
private:
    static const int LINE_WIDTH = 16;

    std::span<uint8_t> m_span;

    /**
     * @brief Defines how bytes should be grouped in the display.
     * 
     * @remarks This defines the number of bytes per line shown.
     * If left to 0, then the control will attempt to fit as many
     * bytes as it can within it's client area.
     */
    unsigned int m_groupBy;

    /**
     * @brief Defines how memory page boundaries should shown.
     * 
     * @remarks 0 indicates no page boundaries, otherwise a page
     * boundary is shown every pageBoundary bytes.
     */
    unsigned int m_pageSize;

public:
    /* constructor */ HexView();

    /* constructor */ HexView(wxWindow *parent, wxWindowID id,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        long sytle = 0,
        const wxString &name = wxControlNameStr);

    virtual ~HexView();

    std::span<uint8_t> GetSpan(void) const { return m_span; }

    void SetSpan(const std::span<uint8_t> &span)
    {
        m_span = span;
        Refresh();
    }

    unsigned int GetGroupBy(void) const { return m_groupBy; }

    void SetGroupBy(unsigned int groupBy)
    {
        m_groupBy = groupBy;
        Refresh();
    }

    unsigned int GetPageSize(void) const { return m_pageSize; }

    void SetPageSize(unsigned int pageSize)
    {
        m_pageSize = pageSize;
        Refresh();
    }

    int GetLineCount() const;

protected:
    bool m_fontDirty; //< Flag indicating a change in font settings.

    wxFont *m_digitFont; //< Font for drawing Hexidecimal digits.

    std::string GetAddressFormat() const;

    void OnPaint(wxPaintEvent &);

    void Draw(wxDC &dc);

    void LoadFonts();

private:
    wxDECLARE_DYNAMIC_CLASS(HexView);
    wxDECLARE_EVENT_TABLE();
};

/*************************************************************************/

#endif /* HEXVIEW_H__ */

/*************************************************************************/
