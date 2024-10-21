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

#ifndef FONT_CONTROLLER_6502_H__
#define FONT_CONTROLLER_6502_H__

/*************************************************************************/
/**
 * @brief Manages configured fonts throughout the application.
 */
class FontController : public Singleton<FontController>
{
private:
    /**
     * @brief Monospaced font for rendering HexView and IOWindow
     */
    wxFont *m_monoFont;

    /**
     * @brief Precalculated size of m_monoFont.
     */
    wxSize m_cellSize;

    void LoadFonts();

    void CalcCellSizes();

public:
    /* constructor */ FontController();
    virtual          ~FontController();

    const wxFont &getMonoFont() const { return *m_monoFont; }

    wxSize getCellSize() const { return m_cellSize; }
};

/*************************************************************************/

#endif /* FONT_CONTROLLER_6502_H__ */

/*************************************************************************/
