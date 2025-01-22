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

#ifndef HE_DRAW_METRICS_H__
#define HE_DRAW_METRICS_H__

/*=======================================================================*/

namespace hex
{
    /*===============================================================*/

    struct Colors
    {
        /// Get's the foreground color of unselected text.
        wxColour FontColor;

        /// Get's the foreground color of selected text.
        wxColour SelectFontColor;

        /// Get's the background color for basic windows.
        wxColour WindowBackground;

        /// Get's the background color for controls.
        wxColour ControlBackground;

        /// Get's the background color for highlighted text.
        wxColour HighlightBackground;

        /// Get's the pen color for highlighted areas.
        wxColour HighlightBorder;

        Colors();
    };

    /*===============================================================*/

    struct CellMetrics
    {
        /// Number of characters in each cell
        int CharacterCount;

        /// Gap between each cell
        wxSize GapPx;

        /// Size of each cell
        wxSize SizePx;
    };

    /*===============================================================*/

    /**
     * @brief Holds precomputed colors, sizes and other drawing metrics.
     */
    class DrawMetrics
    {
    private:
        // Assume 64KB if no memory defined yet.
        const size_t DEFAULT_MAX = 0x0010000;

        // In pixels
        const int MIN_BASE_ADDR_SIZE = 50;

        const HexEdit *m_owner;

        void GetBaseItems();

        void CalcCellMetrics();
        void CalcSizes();

    public:
        DrawMetrics(const HexEdit *owner);
        virtual ~DrawMetrics() { }

    public: // Properties
        Colors colors;

        CellMetrics cellMetrics;

        /// Size of the character area in pixels.
        int CharAreaWidthPx;

        /// Get the starting X offset of the character area.
        int CharAreaStartX;

        /// Size of the gap between items in the grid.
        wxSize GapSizePx;

        /// Size of a single 'M' character in the font.
        wxSize CharSizePx;

        /// Number of lines (rows) for the overall view.
        int TotalLineCount;

        /// NUmber of bytes displayed per line.
        int LineByteCount;

        /// Size of the cell area line in characters.
        int LineCellsWidthChars;

        /// Size of the cell area line in pixels.
        int LineCellsWidthPx;

        /// Number of characters in the base address area.
        int BaseAddressCharCount;

        /// Width of the base address area.
        int BaseAddressWidthPx;

        // Size of a line in pixels.
        wxSize LineSizePx;

        /// The height (in pixels) of the offset header.
        int OffsetHeightPx;

        /// The total size of the scrollable area in pixels.
        wxSize TotalScrollPx;
    };

    typedef std::shared_ptr<DrawMetrics> PDrawMetrics;

    /*===============================================================*/
}

/*=======================================================================*/

#endif /* HE_DRAW_METRICS_H__ */

/*=======================================================================*/
