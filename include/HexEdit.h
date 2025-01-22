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

#ifndef HEXEDIT_H__
#define HEXEDIT_H__

/*=======================================================================*/

#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <memory>
#include <span>

#include <wx/wx.h>
#include <wx/scrolwin.h>

/*=======================================================================*/

namespace hex
{
    /*===============================================================*/
    /**
     * @brief Abstaction to memory data.
     */
    class MemoryView
    {
    protected:
        /* constructor */ MemoryView() { }

    public:
        virtual ~MemoryView() { }

        /**
         * @brief Gets the size in bytes of the memory area.
         */
        virtual size_t GetSize() const = 0;

        /**
         * @brief Reads a single byte in the memory area.
         * @param address The address to read from.
         */
        virtual uint8_t GetByte(uint32_t address) const = 0;

        /**
         * @brief Sets a single byte in the memory area.
         * @param address The address to set.
         * @param value The new value to write.
         */
        virtual void SetByte(uint32_t address, uint8_t value) = 0;
    };

    typedef std::shared_ptr<MemoryView> PMemoryView;

    /*===============================================================*/
    /**
     * @brief Hex editor control
     */
    class HexEdit : public wxScrolled<wxWindow>
    {
    private:
        friend class SubControl;

        /// Calculated drawing metrics.
        std::shared_ptr<class DrawMetrics> m_metrics;

        /**
         * @brief View into memory to dump.
         */
        PMemoryView m_memory;

        // Column offsets view
        class OffsetColumns *m_offsets;

        // Base address view
        class BaseAddressRows *m_baseAddresses;

        // Hex value display area
        class HexArea *m_hexArea;
        wxSizerItem *m_areaSizer;

        uint32_t m_selStart;
        uint32_t m_selLen;

        wxPoint m_mouseDn; //< Save's where the mouse was first pressed down.

        /**
         * @brief Format string for base address view
         */
        wxString m_baseAddressFmt;

        bool m_mouseTrack;

        void AdjustItemSizes();

        void Init();

    public:
        /* constructor */ HexEdit();

        /* constructor */ HexEdit(wxWindow *parent, wxWindowID id,
            const wxPoint &pos = wxDefaultPosition,
            const wxSize &size = wxDefaultSize,
            const wxString &name = wxControlNameStr);

        virtual ~HexEdit();

        virtual bool SetFont(const wxFont &font) override
        {
            bool rval = wxScrolled<wxWindow>::SetFont(font);

            if (rval)
                AdjustItemSizes();

            return rval;
        }

        /**
         * @brief Gets the current line that we're editing.
         */
        uint32_t GetCurrentLine() const;

        /**
         * @brief Gets the current column we're editing.
         */
        uint32_t GetCurrentColumn() const;

        /**
         * @brief Gets the current address inside of the memory area we're editing.
         */
        uint32_t GetAddress() const;

        void JumpTo(uint32_t address);

        std::shared_ptr<MemoryView> GetMemory() const { return m_memory; }

        void SetMemory(MemoryView *mem)
        {
            m_memory.reset(mem);

            AdjustItemSizes();
        }

    private: // Events
        wxDECLARE_EVENT_TABLE();

        void OnSize(wxSizeEvent &);
    };
}

/*=======================================================================*/

#endif /* HEXEDIT_H__ */

/*=======================================================================*/
