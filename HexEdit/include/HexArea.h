/*=======================================================================*/
/*
 * Copyright(c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files(the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and /or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*=======================================================================*/

#ifndef HE_HEX_AREA_H__
#define HE_HEX_AREA_H__

/*=======================================================================*/

namespace hex
{
    /*===============================================================*/
    /**
     * @brief Information about a selected portion of the working area.
     */
    struct Selection
    {
        /// The first selected cell address.
        uint32_t start;

        /// Number of values selected.
        uint32_t length;

        Selection() : start(0), length(0) { }

        void clear()
        {
            start = 0;
            length = 0;
        }

        /// Last selected cell address.
        uint32_t end() const { return start + length - 1; }

        /// Checks if the given cell is selected or not.
        bool CellSelected(uint32_t cell) const
        {
            return (length != 0) && (start <= cell) && (cell <= end());
        }
    };

    /*===============================================================*/
    /**
     * @brief Precomputed details about the active drawing operation.
     */
    class AreaDrawState
    {
    public:
        wxDC &dc;

        PDrawMetrics metrics;

        PMemoryView mem;

        wxPoint start;

        uint32_t startAddress;
        uint32_t endAddress;

        uint32_t startLine;
        uint32_t endLine;

        AreaDrawState(wxDC &drawContext, HexArea *self);
    };

    /*===============================================================*/

    class CellWorker
    {
    private:
        uint32_t m_cell;
        PMemoryView m_view;
        const Selection &m_selection;

    protected:
        CellWorker(
            uint32_t cell, 
            PMemoryView view,
            const Selection &selection)
            : m_cell(cell)
            , m_view(view)
            , m_selection(selection)
        {
        }

    public:
        virtual ~CellWorker() { }

        /// Gets if this cell is currently selected or not.
        bool selected(int offset = 0) const 
        {
            uint32_t c = m_cell + offset;

            wxASSERT(c < m_view->GetSize());

            return m_selection.CellSelected(c);
        }

        /// Get's the cell index.
        uint32_t cell(void) const { return m_cell; }

        /// Gets the value for this cell.
        uint8_t value(void) const { return m_view->GetByte(m_cell); }

        /// Sets the value for this cell.
        void value(uint8_t v) { m_view->SetByte(m_cell, v); }
    };

    /*===============================================================*/

    class CellPainter : public CellWorker
    {
    private:
        wxDC &m_dc;
        const AreaDrawState &m_drawState;

        int m_line;
        int m_column;

        wxSize m_sizePx;
        wxSize m_gapPx;
        wxSize m_fullPx;

        wxRect m_cellBounds;
        wxRect m_charBounds;

        bool m_aboveSelected;
        bool m_belowSelected;
        bool m_leftSelected;
        bool m_rightSelected;

        wxRect CalcCellBounds();
        wxRect CalcCharBounds();

        void CalcSelections();

    public:
        CellPainter(
            const AreaDrawState &drawState,
            const Selection &selection,
            uint32_t cell);

        virtual ~CellPainter() { }

        void Draw();

    private:
        void DrawBackground();
        void DrawSelectionBounds(wxRect rect);
        void DrawText();
    };

    /*===============================================================*/

    /**
     * @brief Primary display for the HexEdit control
     */
    class HexArea : public wxPanel, public SubControl
    {
    private:
        friend AreaDrawState;

        BaseAddressRows *m_baseRows;
        OffsetColumns *m_offsetCols;

        /// Details of the current selection.
        Selection m_selection;

        /// Set if the mouse selection started over the character area, not the hex area.
        bool m_mouseOnChar;

        /// Set if we are actively tracking the mouse movement.
        bool m_mouseTrack;

        /// The starting cell the user clicked on.
        wxPoint m_mouseDn;

        uint32_t CalcAddressFromCell(wxPoint &) const;

        /**
         * @brief Convert pixel to cell coordinates.
         * @param where The location to convert
         * @return True if hit the character area and not the hex area.
         */
        bool ToCell(wxPoint *where);

        void UpdateSelection(wxMouseEvent &e);

    public:
        HexArea(HexEdit *owner, BaseAddressRows *baseRows, OffsetColumns *offsetCols);

        virtual void ScrollWindow(int dx, int dy, const wxRect *rect) override;

    private: // Event processing
        wxDECLARE_EVENT_TABLE();

        void OnMouseDown(wxMouseEvent &);
        void OnMouseUp(wxMouseEvent &);
        void OnMouseMove(wxMouseEvent &);
        void OnCaptureLost(wxMouseCaptureLostEvent &);

        void OnKeyDown(wxKeyEvent &);

        void OnPaint(wxPaintEvent &);
    };
}

/*=======================================================================*/

#endif /* HE_ITEM_CANVAS_H__ */

/*=======================================================================*/
