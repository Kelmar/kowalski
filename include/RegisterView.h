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

#ifndef REGISTER_VIEW_6502_H__
#define REGISTER_VIEW_6502_H__

/*=======================================================================*/

class RegisterView : public wxFrame, public wxExtra
{
private:
    wxTextCtrl* m_instDisplay;
    wxStaticBitmap* m_instAlert;

    wxTextCtrl* m_aEdit;
    wxTextCtrl* m_xEdit;
    wxTextCtrl* m_yEdit;
    wxTextCtrl* m_spEdit;
    wxTextCtrl* m_pcEdit;
    wxTextCtrl* m_cycleEdit;
    wxTextCtrl* m_pEdit;

private:
    void InitControlls();
    void BindEvents();

public:
    RegisterView(wxWindow *parent);
    virtual ~RegisterView() { }

    void UpdateStatus();

private:
    void OnClose(wxCloseEvent &);
};

/*=======================================================================*/

#endif /* REGISTER_VIEW_6502_H__ */

/*=======================================================================*/
