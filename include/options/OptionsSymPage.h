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

#ifndef OPTIONS_SYM_PAGE_6502_H__
#define OPTIONS_SYM_PAGE_6502_H__

/*************************************************************************/

class OptionsSymPage : public OptionsPage, public wxExtra
{
private:
    uint32_t m_nIOAddress;
    bool m_bIOEnable;
    int m_nFinish;
    int m_nWndWidth;
    int m_nWndHeight;
    bool m_bProtectMemory;
    uint32_t m_nProtFromAddr;
    uint32_t m_nProtToAddr;

public:
    /* constructor */ OptionsSymPage(wxBookCtrlBase *parent);
    virtual ~OptionsSymPage();

    virtual void AbortChanges() override;
    virtual void SaveChanges() override;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //afx_msg bool OnHelpInfo(HELPINFO* pHelpInfo);
    //afx_msg void OnContextMenu(wxWindow *pWnd, wxPoint point);
};

/*************************************************************************/

#endif /* OPTIONS_SYM_PAGE_6502_H__ */

/*************************************************************************/

