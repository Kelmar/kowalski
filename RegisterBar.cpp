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

// RegisterBar.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "RegisterBar.h"
#include "Sym6502.h"
#include "Deasm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


bool CRegisterBar::m_bHidden = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CRegisterBar dialog

CRegisterBar::CRegisterBar()
{
    m_bInUpdate = FALSE;
}



BEGIN_MESSAGE_MAP(CRegisterBar, CDialogBar)
    //{{AFX_MSG_MAP(CRegisterBar)
    ON_EN_CHANGE(IDC_REGS_A, OnChangeRegA)
    ON_EN_CHANGE(IDC_REGS_X, OnChangeRegX)
    ON_EN_CHANGE(IDC_REGS_Y, OnChangeRegY)
    ON_EN_CHANGE(IDC_REGS_S, OnChangeRegS)
    ON_EN_CHANGE(IDC_REGS_P, OnChangeRegP)
    ON_EN_CHANGE(IDC_REGS_PC, OnChangeRegPC)
    ON_BN_CLICKED(IDC_REGS_NEG, OnRegFlagNeg)
    ON_BN_CLICKED(IDC_REGS_CARRY, OnRegFlagCarry)
    ON_BN_CLICKED(IDC_REGS_DEC, OnRegFlagDec)
    ON_BN_CLICKED(IDC_REGS_INT, OnRegFlagInt)
    ON_BN_CLICKED(IDC_REGS_OVER, OnRegFlagOver)
    ON_BN_CLICKED(IDC_REGS_ZERO, OnRegFlagZero)
    ON_BN_CLICKED(IDC_REGS_BRK, OnRegFlagBrk)
    ON_BN_CLICKED(IDC_REGS_M16, OnRegFlagM16)
    ON_BN_CLICKED(IDC_REGS_X16, OnRegFlagX16)
    ON_BN_CLICKED(IDC_REGS_EMM, OnRegFlagEmm)
    ON_WM_WINDOWPOSCHANGING()
    ON_BN_CLICKED(IDC_REGS_CYCLES_CLR, OnRegsCyclesClr)
    //}}AFX_MSG_MAP
    ON_MESSAGE(CBroadcast::WM_USER_UPDATE_REG_WND, OnUpdate)
    ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebug)
    ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebug)
    //  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRegisterBar message handlers


bool CRegisterBar::Create(CWnd *pParentWnd, UINT nStyle, UINT nID)
{
    bool ret = CDialogBar::Create(pParentWnd, IDD, nStyle, nID);
    if (!ret)
        return FALSE;

    ShowWindow(SW_HIDE);

    CString title;
    if (title.LoadString(IDD))
        SetWindowText(title);

    UINT vTabs[] = { 16, 12 };
    SendDlgItemMessage(IDC_REGS_A_MEM, EM_SETTABSTOPS, 2, reinterpret_cast<LPARAM>(vTabs));
    SendDlgItemMessage(IDC_REGS_X_MEM, EM_SETTABSTOPS, 2, reinterpret_cast<LPARAM>(vTabs));
    SendDlgItemMessage(IDC_REGS_Y_MEM, EM_SETTABSTOPS, 2, reinterpret_cast<LPARAM>(vTabs));

    return TRUE;
}


bool CRegisterBar::UpdateItem(int itemID)	// od�wie�enie obiektu okna dialogowego
{
    HWND hWnd = ::GetDlgItem(m_hWnd, itemID);
    if (hWnd)
        return ::UpdateWindow(hWnd);
    return FALSE;
}


void CRegisterBar::Update(const CContext *pCtx, const CString &stat, const CContext *pOld /*= NULL*/, bool bDraw /*= TRUE*/)
{
    ASSERT(pCtx != NULL);
    if (m_bInUpdate)
        return;

    ProcessorType procType = theApp.m_global.GetProcType();

    if (procType == ProcessorType::WDC65816 && !pCtx->mem16)
        SetDlgItemWordHex(IDC_REGS_A, pCtx->a);
    else
        SetDlgItemByteHex(IDC_REGS_A, pCtx->a & 0xff);
    UpdateRegA(pCtx);

    if (procType == ProcessorType::WDC65816 && !pCtx->xy16)
        SetDlgItemWordHex(IDC_REGS_X, pCtx->x);
    else
        SetDlgItemByteHex(IDC_REGS_X, pCtx->x & 0xff);
    UpdateRegX(pCtx);

    if (procType == ProcessorType::WDC65816 && !pCtx->xy16)
        SetDlgItemWordHex(IDC_REGS_Y, pCtx->y);
    else
        SetDlgItemByteHex(IDC_REGS_Y, pCtx->y & 0xff);
    UpdateRegY(pCtx);

    SetDlgItemByteHex(IDC_REGS_P, pCtx->get_status_reg());
    UpdateRegP(pCtx);

    SetDlgItemWordHex(IDC_REGS_PC, pCtx->pc);
    UpdateRegPC(pCtx);

    UpdateRegS(pCtx);
    if (procType == ProcessorType::WDC65816) // && !pCtx->emm)
    {
        SetDlgItemWordHex(IDC_REGS_S, pCtx->s);
        SetDlgItemByteHex(IDC_REGS_PBR, pCtx->pbr);
        SetDlgItemByteHex(IDC_REGS_DBR, pCtx->dbr);
        SetDlgItemWordHex(IDC_REGS_DIR, pCtx->dir);
        SetDlgItemByteHex(IDC_REGS_B, pCtx->b);
    }
    else
    {
        SetDlgItemByteHex(IDC_REGS_S, (pCtx->s & 0xff));
        SetDlgItemBlank(IDC_REGS_PBR);
        SetDlgItemBlank(IDC_REGS_DBR);
        SetDlgItemBlank(IDC_REGS_DIR);
        SetDlgItemBlank(IDC_REGS_B);
    }

    SetDlgItemText(IDC_REGS_STAT, stat);
    UpdateItem(IDC_REGS_STAT);
    UpdateCycles(pCtx->uCycles);

    if (pOld == NULL)
        return;

}
void CRegisterBar::SetDlgItemBlank(int nID)
{
    TCHAR buf[32];
    wsprintf(buf, _T(" "));
    SetDlgItemText(nID, buf);
    UpdateItem(nID);
}

void CRegisterBar::SetDlgItemByteHex(int nID, UINT8 val)
{
    TCHAR buf[32];
    wsprintf(buf, _T("$%02X"), val & 0xFF);
    SetDlgItemText(nID, buf);
    UpdateItem(nID);
}

void CRegisterBar::SetDlgItemWordHex(int nID, UINT16 val)
{
    TCHAR buf[32];
    wsprintf(buf, _T("$%04X"), val & 0xFFFF);
    SetDlgItemText(nID, buf);
    UpdateItem(nID);
}

void CRegisterBar::SetDlgItemMem(int nID, int nBytes, UINT16 ptr, const CContext *pCtx)
{
    CString str(_T(' '), 128), num;
    str.Empty();

    for (int i = 0; i < nBytes; i++)
    {
        num.Format(i == nBytes ? _T("%02X") : _T("%02X "), pCtx->mem[(ptr + i) & pCtx->mem_mask] & 0xFF);
        str += num;
    }

    SetDlgItemText(nID, str);
    UpdateItem(nID);
}


void CRegisterBar::SetDlgItemInf(int nID, UINT16 val, bool word)
{
    CString str;
    if (val != ~0)
        str.Format(_T("%d,\t'%c',\t%s"), val & 0xFF, val >= ' ' ? TCHAR(val) : _T('?'), (const TCHAR *)Binary(val, word));
    SetDlgItemText(nID, str);
    UpdateItem(nID);
}


CString CRegisterBar::Binary(UINT16 val, bool word)
{
    CString bin(_T(' '), 16);
    if (word) {
        bin.SetAt(0, val & 0x8000 ? _T('1') : _T('0'));
        bin.SetAt(1, val & 0x4000 ? _T('1') : _T('0'));
        bin.SetAt(2, val & 0x2000 ? _T('1') : _T('0'));
        bin.SetAt(3, val & 0x1000 ? _T('1') : _T('0'));
        bin.SetAt(4, val & 0x0800 ? _T('1') : _T('0'));
        bin.SetAt(5, val & 0x0400 ? _T('1') : _T('0'));
        bin.SetAt(6, val & 0x0200 ? _T('1') : _T('0'));
        bin.SetAt(7, val & 0x0100 ? _T('1') : _T('0'));
        bin.SetAt(8, val & 0x80 ? _T('1') : _T('0'));
        bin.SetAt(9, val & 0x40 ? _T('1') : _T('0'));
        bin.SetAt(10, val & 0x20 ? _T('1') : _T('0'));
        bin.SetAt(11, val & 0x10 ? _T('1') : _T('0'));
        bin.SetAt(12, val & 0x08 ? _T('1') : _T('0'));
        bin.SetAt(13, val & 0x04 ? _T('1') : _T('0'));
        bin.SetAt(14, val & 0x02 ? _T('1') : _T('0'));
        bin.SetAt(15, val & 0x01 ? _T('1') : _T('0'));

    }
    else {
        bin.SetAt(0, val & 0x80 ? _T('1') : _T('0'));
        bin.SetAt(1, val & 0x40 ? _T('1') : _T('0'));
        bin.SetAt(2, val & 0x20 ? _T('1') : _T('0'));
        bin.SetAt(3, val & 0x10 ? _T('1') : _T('0'));
        bin.SetAt(4, val & 0x08 ? _T('1') : _T('0'));
        bin.SetAt(5, val & 0x04 ? _T('1') : _T('0'));
        bin.SetAt(6, val & 0x02 ? _T('1') : _T('0'));
        bin.SetAt(7, val & 0x01 ? _T('1') : _T('0'));
    }

    return bin;
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CRegisterBar::OnUpdate(WPARAM wParam, LPARAM lParam)
{
    Update((const CContext *)lParam, *(const CString *)wParam);
    return 1;
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CRegisterBar::OnStartDebug(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    CFrameWnd *pWnd = GetDockingFrame();
    if (!m_bHidden)		// okno by�o widoczne?
        if (pWnd)
            pWnd->ShowControlBar(this, TRUE, TRUE);
    //    ShowWindow(SW_SHOW);
    return 1;
}


afx_msg LRESULT CRegisterBar::OnExitDebug(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    CFrameWnd *pWnd = GetDockingFrame();

    if (m_hWnd && IsVisible() /*(GetStyle() & WS_VISIBLE)*/)// okno aktualnie wy�wietlone?
    {
        m_bHidden = FALSE;				// info - okno by�o wy�wietlane
        if (pWnd)
            pWnd->ShowControlBar(this, FALSE, TRUE);
        //    ShowWindow(SW_HIDE);			// ukrycie okna
    }
    else
        m_bHidden = TRUE;				// info - okno by�o ukryte
    return 1;
}

//=============================================================================

void CRegisterBar::UpdateRegA(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //  SetDlgItemByteHex(IDC_REGS_A,pCtx->a);
    bool word = (theApp.m_global.GetProcType() == ProcessorType::WDC65816 && !pCtx->emm && !pCtx->mem16);
    SetDlgItemInf(IDC_REGS_A_MEM, pCtx->a, word);
}

void CRegisterBar::UpdateRegX(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //  SetDlgItemByteHex(IDC_REGS_X,pCtx->x);
    bool word = (theApp.m_global.GetProcType() == ProcessorType::WDC65816 && !pCtx->emm && !pCtx->xy16);
    SetDlgItemInf(IDC_REGS_X_MEM, pCtx->x, word);
}

void CRegisterBar::UpdateRegY(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //  SetDlgItemByteHex(IDC_REGS_Y,pCtx->y);
    bool word = (theApp.m_global.GetProcType() == ProcessorType::WDC65816 && !pCtx->emm && !pCtx->xy16);
    SetDlgItemInf(IDC_REGS_Y_MEM, pCtx->y, word);
}

void CRegisterBar::UpdateRegP(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //  SetDlgItemByteHex(IDC_REGS_P,pCtx->get_status_reg());
    CheckDlgButton(IDC_REGS_NEG, pCtx->negative);
    CheckDlgButton(IDC_REGS_ZERO, pCtx->zero);
    CheckDlgButton(IDC_REGS_OVER, pCtx->overflow);
    CheckDlgButton(IDC_REGS_CARRY, pCtx->carry);
    CheckDlgButton(IDC_REGS_INT, pCtx->interrupt);
    CheckDlgButton(IDC_REGS_BRK, pCtx->break_bit);
    CheckDlgButton(IDC_REGS_DEC, pCtx->decimal);
    CheckDlgButton(IDC_REGS_M16, pCtx->mem16);
    CheckDlgButton(IDC_REGS_X16, pCtx->xy16);
    CheckDlgButton(IDC_REGS_EMM, pCtx->emm);
    if (theApp.m_global.m_procType == ProcessorType::WDC65816 && !pCtx->emm)
        CheckDlgButton(IDC_REGS_BRK, 0);
    else
        CheckDlgButton(IDC_REGS_BRK, pCtx->break_bit);
}

void CRegisterBar::UpdateRegPC(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //  SetDlgItemWordHex(IDC_REGS_PC,pCtx->pc);
    CDeasm deasm;
    INT32 ptr = -1;
    SetDlgItemText(IDC_REGS_INSTR, deasm.DeasmInstr(*pCtx, CAsm::DF_BRANCH_INFO, ptr));
    UpdateItem(IDC_REGS_INSTR);
    SetDlgItemText(IDC_REGS_INSTR_ARG, deasm.ArgumentValue(*pCtx));
    UpdateItem(IDC_REGS_INSTR_ARG);
}

void CRegisterBar::UpdateRegS(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    UINT16 sr = pCtx->s;
    if (theApp.m_global.GetProcType() != ProcessorType::WDC65816)
        sr = (pCtx->s & 0xff) + 0x100;

    if (sr != theApp.m_global.m_bSRef)
        SetDlgItemMem(IDC_REGS_S_MEM, theApp.m_global.m_bSRef - sr, sr + 1, pCtx);
    else {				// wypisujemy, �e nic
        CString str;
        str.LoadString(IDS_REGS_S_EMPTY);
        SetDlgItemText(IDC_REGS_S_MEM, str);
    }
}

void CRegisterBar::UpdateCycles(ULONG uCycles)
{
    SetDlgItemInt(IDC_REGS_CYCLES, uCycles, false);
    UpdateItem(IDC_REGS_CYCLES);
}


//=============================================================================

void CRegisterBar::ChangeRegister(int ID, int reg_no)
{
    if (m_bInUpdate || theApp.m_global.IsProgramRunning())  // update lub dzia�a program?
        return;					// zignorowanie zmian
    CString buf;
    if (GetDlgItemText(ID, buf) == 0)
        return;
    const TCHAR *str = buf;
    int num;
    if (str[0] == _T('$'))
    {
        if (sscanf(LPCTSTR(str) + 1, _T("%X"), &num) <= 0)
            num = 0;
    }
    else if (str[0] == _T('0') && (str[1] == _T('x') || str[1] == _T('X')))
    {
        if (sscanf(str + 2, _T("%X"), &num) <= 0)
            num = 0;
    }
    else if (sscanf(str, _T("%u"), &num) <= 0)
        num = 0;

    CSym6502 *pSym = theApp.m_global.GetSimulator();	// symulator
    if (pSym == NULL)
    {
        ASSERT(FALSE);
        return;
    }

    m_bInUpdate = TRUE;

    CContext ctx(*(pSym->GetContext()));	// kontekst programu

    ProcessorType procType = theApp.m_global.GetProcType();

    switch (reg_no)			// wprowadzenie zmiany
    {
    case 0:
        if ((procType == ProcessorType::WDC65816) && !ctx.emm && !ctx.mem16)
            ctx.a = UINT16(num);
        else
            ctx.a = UINT8(num);
        UpdateRegA(&ctx);
        break;
    case 1:
        if ((procType == ProcessorType::WDC65816) && !ctx.emm && !ctx.xy16)
            ctx.x = UINT16(num);
        else
            ctx.x = UINT8(num);
        UpdateRegX(&ctx);
        break;
    case 2:
        if ((procType == ProcessorType::WDC65816) && !ctx.emm && !ctx.xy16)
            ctx.y = UINT16(num);
        else
            ctx.y = UINT8(num);
        UpdateRegY(&ctx);
        break;
    case 3:
        if ((procType == ProcessorType::WDC65816))
            ctx.s = UINT16(num);
        else
            ctx.s = UINT8(num);
        UpdateRegS(&ctx);
        break;
    case 4:
        ctx.set_status_reg_bits(UINT8(num));
        if (theApp.m_global.GetProcType() != ProcessorType::M6502) // 65c02?
            ctx.reserved = TRUE;			// bit 'reserved' zawsze ustawiony
        UpdateRegP(&ctx);
        break;
    case 5:
        ctx.pc = UINT16(num) & ctx.mem_mask;
        UpdateRegPC(&ctx);
        break;
    default:
        ASSERT(FALSE);
    }
    pSym->SetContext(ctx);			// zmiana kontekstu

    if (reg_no == 5 && !theApp.m_global.IsProgramFinished())	// zmiana PC?
        pSym->SkipToAddr(ctx.pc);

    m_bInUpdate = FALSE;
}


void CRegisterBar::ChangeFlags(int flag_bit, bool set)	// zmiana bitu rej. flagowego
{
    CSym6502 *pSym = theApp.m_global.GetSimulator();	// symulator
    if (pSym == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    CContext ctx(*(pSym->GetContext()));	// kontekst programu
    UINT8 flags = ctx.get_status_reg();
    if (set) {
        if (flag_bit == 8) ctx.emm = true;
        else flags |= UINT8(1 << flag_bit);
    }
    else {
        if (flag_bit == 8) ctx.emm = false;
        else flags &= ~UINT8(1 << flag_bit);
    }
    ctx.set_status_reg_bits(flags);
    if (theApp.m_global.GetProcType() != ProcessorType::M6502) // 65c02?
        ctx.reserved = TRUE;			// bit 'reserved' zawsze ustawiony

    SetDlgItemByteHex(IDC_REGS_P, ctx.get_status_reg());

    pSym->SetContext(ctx);		// zmiana kontekstu
}

//-----------------------------------------------------------------------------

void CRegisterBar::OnChangeRegA()
{
    ChangeRegister(IDC_REGS_A, 0);
}

void CRegisterBar::OnChangeRegX()
{
    ChangeRegister(IDC_REGS_X, 1);
}

void CRegisterBar::OnChangeRegY()
{
    ChangeRegister(IDC_REGS_Y, 2);
}

void CRegisterBar::OnChangeRegS()
{
    ChangeRegister(IDC_REGS_S, 3);
}

void CRegisterBar::OnChangeRegP()
{
    ChangeRegister(IDC_REGS_P, 4);
}

void CRegisterBar::OnChangeRegPC()
{
    ChangeRegister(IDC_REGS_PC, 5);
}


void CRegisterBar::OnRegFlagNeg()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_NEG);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_NEGATIVE, pBtn->GetCheck());
}

void CRegisterBar::OnRegFlagCarry()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_CARRY);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_CARRY, pBtn->GetCheck());
}

void CRegisterBar::OnRegFlagDec()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_DEC);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_DECIMAL, pBtn->GetCheck());
}

void CRegisterBar::OnRegFlagInt()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_INT);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_INTERRUPT, pBtn->GetCheck());
}

void CRegisterBar::OnRegFlagOver()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_OVER);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_OVERFLOW, pBtn->GetCheck());
}

void CRegisterBar::OnRegFlagZero()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_ZERO);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_ZERO, pBtn->GetCheck());
}

void CRegisterBar::OnRegFlagBrk()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_BRK);
    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    ChangeFlags(CContext::N_BREAK, pBtn->GetCheck());
}
void CRegisterBar::OnRegFlagM16()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_M16);

    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }

    if (theApp.m_global.GetProcType() == ProcessorType::WDC65816)
        ChangeFlags(CContext::N_MEMORY, pBtn->GetCheck());
}
void CRegisterBar::OnRegFlagX16()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_X16);

    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }

    if (theApp.m_global.GetProcType() == ProcessorType::WDC65816)
        ChangeFlags(CContext::N_INDEX, pBtn->GetCheck());
}
void CRegisterBar::OnRegFlagEmm()
{
    CButton *pBtn = (CButton *)GetDlgItem(IDC_REGS_EMM);

    if (pBtn == NULL)
    {
        ASSERT(FALSE);
        return;
    }

    if (theApp.m_global.GetProcType() == ProcessorType::WDC65816)
        ChangeFlags(CContext::N_EMULATION, pBtn->GetCheck());
}

void CRegisterBar::OnWindowPosChanging(WINDOWPOS FAR *lpwndpos)
{
    CDialogBar::OnWindowPosChanging(lpwndpos);
}


void CRegisterBar::OnRegsCyclesClr()
{
    if (theApp.m_global.IsProgramRunning())  // dzia�a program?
    {
        MessageBeep(-2);
        return;
    }

    CSym6502 *pSym = theApp.m_global.GetSimulator();	// symulator
    if (pSym == NULL)
    {
        ASSERT(FALSE);
        return;
    }

    pSym->ClearCyclesCounter();
    UpdateCycles(0);
}


void CRegisterBar::OnUpdateCmdUI(CFrameWnd *pTarget, BOOL bDisableIfNoHndler)
{}
