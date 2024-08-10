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

class HexView : public wxScrolled<wxWindow>
{
private:
    static const int LINE_WIDTH = 16;

    /**
     * @brief View into memory to dump.
     */
    std::span<uint8_t> m_span;

    /**
     * @brief Set to the size of a single character cell in our control.
     */
    wxSize m_fontSize;

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

    bool m_fontDirty; //< Flag indicating a change in font settings.

    wxFont *m_digitFont; //< Font for drawing Hexidecimal digits.

    std::string GetAddressFormat() const;

    void OnPaint(wxPaintEvent &);

    void Draw(wxDC &dc);

    void CalculateScrollInfo();
    void UpdateScrollInfo();

    void LoadFonts();

public:
    /* constructor */ HexView();

    /* constructor */ HexView(wxWindow *parent, wxWindowID id,
        const wxPoint &pos = wxDefaultPosition,
        const wxSize &size = wxDefaultSize,
        const wxString &name = wxControlNameStr);

    virtual ~HexView();

    std::span<uint8_t> GetSpan(void) const { return m_span; }

    void SetSpan(const std::span<uint8_t> &span)
    {
        m_span = span;
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
