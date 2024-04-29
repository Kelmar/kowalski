/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#ifndef DIAL_ASM_STAT_H__
#define DIAL_ASM_STAT_H__

class CAsm6502;

/////////////////////////////////////////////////////////////////////////////
// CDialAsmStat dialog

class CDialAsmStat : public wxDialog //, public CAsm, CBroadcast
{
private:
    wxControl *FindControl(long id) const;

    static UINT start_asm_thread(void *pDial);

    UINT StartAsm();
    void SetProgressRange(int max_line);
    void ProgressStep();
    void SetCtrlText(int id, int val);
    void SetLineNo(int val);
    void SetPassNo(int val);

    uint16_t m_dwTimer;
    //int m_nUpdateInit;
    //int m_nUpdateDelay; // Delay refreshing info. about the assembled line
    //int m_nUpdateLineDelay;

    CAsm::Stat m_stAsmRetCode;
    CAsm6502 *m_pAsm6502;
    int m_nPassNo;
    int m_nCurrLine;
    int m_nLines;
    std::string m_strRow;
    std::string m_strPassNo;
    CSrc6502View *m_pView;
    bool m_bFinished;
    std::string m_strText; // All text to assemble
    const char *m_pText;

    std::string GetLine(int nLine);
    void GetLine(int nLine, char *buf, size_t max_len);

    afx_msg LRESULT OnAbortAsm(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetNextLine(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetLineNo(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetDocTitle(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNextPass(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFinished(WPARAM wParam, LPARAM /* lParam */);

public:
    virtual ~CDialAsmStat();
//  CDialAsmStat(CWnd* pParent = NULL);   // standard constructor
    CDialAsmStat(CSrc6502View *pView); // Active document (via 'pView')

    bool Create();
    void SetValues(int row, int pass);

    enum { IDD = IDD_ASSEMBLE };

    wxString m_strCtrlRow;
    wxString m_strCtrlPassNo;

protected:
    //virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    virtual bool OnInitDialog();
    virtual void OnCancel();
    afx_msg bool OnSetCursor(wxWindow* pWnd, UINT nHitTest, UINT message);
};

#endif /* DIAL_ASM_STAT_H__ */
