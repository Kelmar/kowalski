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

#ifndef OPTIONS_EDIT_PAGE_6502_H__
#define OPTIONS_EDIT_PAGE_6502_H__

/*************************************************************************/

#include "SyntaxExample.h"

/*************************************************************************/

class COptionsEditPage : public wxPanel
{
public:
    /* constructor */ COptionsEditPage(wxBookCtrlBase *parent);
    virtual ~COptionsEditPage();

    wxCheckBox m_btnBold;
    wxColourPickerCtrl m_btnColor;
    wxComboBox m_wndElement;

    CSyntaxExample m_wndExample;

    bool m_bAutoIndent;
    int	m_nTabStep;
    bool m_bAutoSyntax;
    bool m_bAutoUppercase;
    bool m_bFileNew;
    int  m_nElement;
    bool m_bColorChanged;

    wxColour *GetColorElement(int nIndex);
    bool *GetFontStyle(int nIndex);

public:
    virtual bool OnSetActive();

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(wxWindow *pWnd, wxPoint point);
    virtual bool OnInitDialog();
    afx_msg void OnChangeTabStep();
    afx_msg void OnColorSyntax();
    afx_msg void OnSelChangeElement();
    afx_msg void OnEditColor();
    afx_msg void OnBoldFont();

    wxColour *GetColorElement();
    bool *GetFontStyle();
};

/*************************************************************************/

#endif /* OPTIONS_EDIT_PAGE_6502_H__ */

/*************************************************************************/

