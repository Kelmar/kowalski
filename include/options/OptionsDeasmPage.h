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

#ifndef OPTIONS_DEASM_PAGE_6502_H__
#define OPTIONS_DEASM_PAGE_6502_H__

/*************************************************************************/

class COptionsDeasmPage : public wxPanel
{
private:
    wxColourPickerCtrl m_ColorButtonAddress;
    wxColourPickerCtrl m_ColorButtonCode;
    wxColourPickerCtrl m_ColorButtonInstr;
    bool m_bSubclassed;

public:
    wxColour m_rgbAddress;
    wxColour m_rgbCode;
    //wxColour m_rgbInstr;
    bool m_bColorChanged;

    // Construction
public:
    /* constructor */ COptionsDeasmPage(wxBookCtrlBase *parent);
    virtual ~COptionsDeasmPage();

    bool m_ShowCode;

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow *pWnd, wxPoint point);
};

/*************************************************************************/

#endif /* OPTIONS_DEASM_PAGE_6502_H__ */

/*************************************************************************/
