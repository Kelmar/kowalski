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

#ifndef OPTIONS_MARKS_PAGE_6502_H__
#define OPTIONS_MARKS_PAGE_6502_H__

/*=======================================================================*/

class COptionsMarksPage : public wxPanel
{
private:
    wxColourPickerCtrl m_ColorButtonPointer;
    wxColourPickerCtrl m_ColorButtonBreakpoint;
    wxColourPickerCtrl m_ColorButtonError;

    bool m_bSubclassed;

public:
    wxColour m_rgbPointer;
    wxColour m_rgbBreakpoint;
    wxColour m_rgbError;
    bool m_colorChanged;
    bool m_bFontChanged;
    wxFontInfo m_LogFont;

    // Construction
public:
    /* constructor */ COptionsMarksPage(wxBookCtrlBase *parent);
    virtual ~COptionsMarksPage();

    int m_nProc6502;
    UINT m_uBusWidth;
    int m_nHelpFile; //^^help

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow *pWnd, wxPoint point);
    afx_msg void OnOptFontBtn();
};

/*=======================================================================*/

#endif /* OPTIONS_MARKS_PAGE_6502_H__ */

/*=======================================================================*/
