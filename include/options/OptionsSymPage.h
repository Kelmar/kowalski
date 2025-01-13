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
    // Processor
    wxChoice *m_cpuSelect;

    // Finish Running Program By
    wxRadioButton *m_brkRadio;
    wxRadioButton *m_rtsRadio;
    wxRadioButton *m_dbRadio;

    // In/Out Memory Area
    wxCheckBox *m_ioActiveChk;
    wxTextCtrl *m_ioAddrTxt;

    // Input/Output Window
    wxSpinCtrl *m_colSpin;
    wxSpinCtrl *m_rowSpin;

    // Memory Protection
    wxCheckBox *m_writeDetectChk;
    wxTextCtrl *m_writeStartTxt;
    wxTextCtrl *m_writeEndTxt;

private:
    sim_addr_t m_ioAddress;
    sim_addr_t m_protectStart;
    sim_addr_t m_protectEnd;

private:
    void BindChildren();
    void InitChildren();
    void ReadConfig();

    void SetConfig();

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

