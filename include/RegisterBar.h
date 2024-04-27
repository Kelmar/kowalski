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

#ifndef REGISTER_BAR_H__
#define REGISTER_BAR_H__

// RegisterBar.h : header file
//

#include "resource.h"
#include "Asm.h"

/////////////////////////////////////////////////////////////////////////////
// CRegisterBar dialog bar

class CContext;

class CRegisterBar : public wxFrame //CDialogBar, public CAsm
{
private:
    bool m_bInUpdate;

    bool UpdateItem(int itemID);

    void UpdateRegA(const CContext *pCtx, const CContext *pOld = NULL);
    void UpdateRegX(const CContext *pCtx, const CContext *pOld = NULL);
    void UpdateRegY(const CContext *pCtx, const CContext *pOld = NULL);
    void UpdateRegP(const CContext *pCtx, const CContext *pOld = NULL);
    void UpdateRegS(const CContext *pCtx, const CContext *pOld = NULL);
    void UpdateRegPC(const CContext *pCtx, const CContext *pOld = NULL);
    void UpdateCycles(ULONG uCycles);
    void ChangeRegister(int ID, int reg_no);
    void ChangeFlags(int flag_bit, bool set); // Change the flag register bit

    afx_msg LRESULT OnUpdate(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);

    //virtual void OnUpdateCmdUI(CFrameWnd* pTarget, bool disableIfNoHndler);

public:
    static bool m_bHidden;

    /* constructor */ CRegisterBar();

    void Update(const CContext *ctx, const std::string &stat, const CContext *old = NULL, bool draw = true);

    bool Create(wxWindow* pParentWnd, UINT nStyle, UINT nID);
    
    enum { IDD = IDD_REGISTERBAR };

protected:
    afx_msg void OnChangeRegA();
    afx_msg void OnChangeRegX();
    afx_msg void OnChangeRegY();
    afx_msg void OnChangeRegS();
    afx_msg void OnChangeRegP();
    afx_msg void OnChangeRegPC();
    afx_msg void OnRegFlagNeg();
    afx_msg void OnRegFlagCarry();
    afx_msg void OnRegFlagDec();
    afx_msg void OnRegFlagInt();
    afx_msg void OnRegFlagOver();
    afx_msg void OnRegFlagZero();
    afx_msg void OnRegFlagBrk();
    afx_msg void OnRegsCyclesClr();

    //afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);

private:
    std::string Binary(uint8_t val);
    void SetDlgItemInf(int nID, uint8_t val);
    void SetDlgItemMem(int nID, int nBytes, uint16_t ptr, const CContext *pCtx);
    void SetDlgItemWordHex(int nID, uint16_t val);
    void SetDlgItemByteHex(int nID, uint8_t val);
};

#endif /* REGISTER_BAR_H__ */
