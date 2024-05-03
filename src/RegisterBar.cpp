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

#include "StdAfx.h"
//#include "6502.h"
#include "RegisterBar.h"
#include "Sym6502.h"
#include "Deasm.h"

bool CRegisterBar::m_bHidden = false;

/////////////////////////////////////////////////////////////////////////////
// CRegisterBar dialog

CRegisterBar::CRegisterBar()
{
    m_bInUpdate = false;
//  m_bHidden = FALSE;
    //{{AFX_DATA_INIT(CRegisterBar)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

#if REWRITE_FOR_WX_WIDGETS

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
    ON_WM_WINDOWPOSCHANGING()
    ON_BN_CLICKED(IDC_REGS_CYCLES_CLR, OnRegsCyclesClr)
    //}}AFX_MSG_MAP
    ON_MESSAGE(CBroadcast::WM_USER_UPDATE_REG_WND, OnUpdate)
    ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebug)
    ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebug)
//  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CRegisterBar message handlers

bool CRegisterBar::Create(wxWindow *parent, UINT nStyle, UINT nID)
{
#if REWRITE_TO_WX_WIDGET
    bool ret = CDialogBar::Create(pParentWnd, IDD, nStyle, nID);
    if (!ret)
        return FALSE;

    ShowWindow(SW_HIDE);

    std::string title;
    if (title.LoadString(IDD))
        SetWindowText(title);

    UINT vTabs[] = { 16, 12 };

    SendDlgItemMessage(IDC_REGS_A_MEM, EM_SETTABSTOPS, 2, reinterpret_cast<LPARAM>(vTabs));
    SendDlgItemMessage(IDC_REGS_X_MEM, EM_SETTABSTOPS, 2, reinterpret_cast<LPARAM>(vTabs));
    SendDlgItemMessage(IDC_REGS_Y_MEM, EM_SETTABSTOPS, 2, reinterpret_cast<LPARAM>(vTabs));

    return true;
#endif

    return false;
}


bool CRegisterBar::UpdateItem(int itemID)
{
#if REWRITE_TO_WX_WIDGET
    HWND hWnd = ::GetDlgItem(m_hWnd, itemID);
    if (hWnd)
        return ::UpdateWindow(hWnd);
#endif
    return false;
}

void CRegisterBar::Update(const CContext *pCtx, const std::string &stat, const CContext *pOld /*= NULL*/, bool bDraw /*= TRUE*/)
{
#if REWRITE_TO_WX_WIDGETS
    ASSERT(pCtx);

    if (m_bInUpdate)
        return;

    SetDlgItemByteHex(IDC_REGS_A, pCtx->a);
    UpdateRegA(pCtx);
//  SetDlgItemInf(IDC_REGS_A_MEM, pCtx->a);

    SetDlgItemByteHex(IDC_REGS_X, pCtx->x);
    UpdateRegX(pCtx);
//  SetDlgItemInf(IDC_REGS_X_MEM, pCtx->x);

    SetDlgItemByteHex(IDC_REGS_Y, pCtx->y);
    UpdateRegY(pCtx);
//  SetDlgItemInf(IDC_REGS_Y_MEM, pCtx->y);

    SetDlgItemByteHex(IDC_REGS_P, pCtx->get_status_reg());
    UpdateRegP(pCtx);
    /*
      CheckDlgButton(IDC_REGS_NEG,pCtx->negative);
      CheckDlgButton(IDC_REGS_ZERO,pCtx->zero);
      CheckDlgButton(IDC_REGS_OVER,pCtx->overflow);
      CheckDlgButton(IDC_REGS_CARRY,pCtx->carry);
      CheckDlgButton(IDC_REGS_INT,pCtx->interrupt);
      CheckDlgButton(IDC_REGS_BRK,pCtx->break_bit);
      CheckDlgButton(IDC_REGS_DEC,pCtx->decimal);
    */
    SetDlgItemByteHex(IDC_REGS_S, pCtx->s);
    UpdateRegS(pCtx);
    /*
      if (pCtx->s != 0xFF)		// jest co� na stosie?
        SetDlgItemMem(IDC_REGS_S_MEM, 0xFF - pCtx->s, pCtx->s+0x0100, pCtx);
      else				// wypisujemy, �e nic
      {
        CString str;
        str.LoadString(IDS_REGS_S_EMPTY);
        SetDlgItemText(IDC_REGS_S_MEM,str);
      }
    */
    SetDlgItemWordHex(IDC_REGS_PC, pCtx->pc);
    UpdateRegPC(pCtx);
    /*
      CDeasm deasm;
      int ptr= -1;
      SetDlgItemText(IDC_REGS_INSTR,deasm.DeasmInstr(*pCtx,CAsm::DF_BRANCH_INFO,ptr));
      UpdateItem(IDC_REGS_INSTR);
      SetDlgItemText(IDC_REGS_INSTR_ARG,deasm.ArgumentValue(*pCtx));
      UpdateItem(IDC_REGS_INSTR_ARG);
    */
    SetDlgItemText(IDC_REGS_STAT, stat);
    UpdateItem(IDC_REGS_STAT);

    UpdateCycles(pCtx->uCycles);
#endif
}

void CRegisterBar::SetDlgItemByteHex(int nID, uint8_t val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "$%02X", val & 0xFF);

#if REWRITE_TO_WX_WIDGET
    SetDlgItemText(nID, buf);
    UpdateItem(nID);
#endif
}

void CRegisterBar::SetDlgItemWordHex(int nID, uint16_t val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "$%04X", val & 0xFFFF);

#if REWRITE_TO_WX_WIDGET
    SetDlgItemText(nID, buf);
    UpdateItem(nID);
#endif
}

void CRegisterBar::SetDlgItemMem(int nID, int nBytes, uint16_t ptr, const CContext *pCtx)
{
    std::string str;
    char num[16];

    for (int i = 0; i < nBytes; i++)
    {
        snprintf(num, sizeof(num), i == nBytes ? "%02X" : "%02X ", pCtx->mem[(ptr + i) & pCtx->mem_mask] & 0xFF);
        str += num;
    }

#if REWRITE_TO_WX_WIDGET
    SetDlgItemText(nID, str);
    UpdateItem(nID);
#endif
}

void CRegisterBar::SetDlgItemInf(int nID, uint8_t val)
{
    wxString str;

    if (val != ~0)
        str.Printf("%d,\t'%c',\t%s", val & 0xFF, val >= ' ' ? char(val) : '?', Binary(val).c_str());

#if REWRITE_TO_WX_WIDGET
    SetDlgItemText(nID, str);
    UpdateItem(nID);
#endif
}

std::string CRegisterBar::Binary(uint8_t val)
{
    char bin[9];

    bin[0] = val & 0x80 ? '1' : '0';
    bin[1] = val & 0x40 ? '1' : '0';
    bin[2] = val & 0x20 ? '1' : '0';
    bin[3] = val & 0x10 ? '1' : '0';
    bin[4] = val & 0x08 ? '1' : '0';
    bin[5] = val & 0x04 ? '1' : '0';
    bin[6] = val & 0x02 ? '1' : '0';
    bin[7] = val & 0x01 ? '1' : '0';
    bin[8] = '\0';

    return bin;
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CRegisterBar::OnUpdate(WPARAM wParam, LPARAM lParam)
{
#if REWRITE_TO_WX_WIDGET
    Update((const CContext*)lParam, *(const std::string*)wParam);
#endif
    return 1;
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CRegisterBar::OnStartDebug(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
#if REWRITE_TO_WX_WIDGET
    wxWindow *pWnd = GetDockingFrame();

    if (!m_bHidden && pWnd) // Was the window visible?
    {
        pWnd->ShowControlBar(this, true, true);
    }

    //Show();
#endif

    return 1;
}

afx_msg LRESULT CRegisterBar::OnExitDebug(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
#if REWRITE_TO_WX_WIDGET
    wxWindow *pWnd = GetDockingFrame();

    if (m_hWnd && IsShown()) // window currently displayed?
    {
        m_bHidden = false; // info - the window was displayed

        if (pWnd)
            pWnd->ShowControlBar(this, false, true);
        //Hide();
    }
    else
        m_bHidden = true; // info - the window was hidden
#endif

    return 1;
}

//=============================================================================

void CRegisterBar::UpdateRegA(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //SetDlgItemInf(IDC_REGS_A_MEM, pCtx->a);
}

void CRegisterBar::UpdateRegX(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //SetDlgItemInf(IDC_REGS_X_MEM, pCtx->x);
}

void CRegisterBar::UpdateRegY(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
    //SetDlgItemInf(IDC_REGS_Y_MEM, pCtx->y);
}

void CRegisterBar::UpdateRegP(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
#if REWRITE_TO_WX_WIDGETS
    CheckDlgButton(IDC_REGS_NEG, pCtx->negative);
    CheckDlgButton(IDC_REGS_ZERO, pCtx->zero);
    CheckDlgButton(IDC_REGS_OVER, pCtx->overflow);
    CheckDlgButton(IDC_REGS_CARRY, pCtx->carry);
    CheckDlgButton(IDC_REGS_INT, pCtx->interrupt);
    CheckDlgButton(IDC_REGS_BRK, pCtx->break_bit);
    CheckDlgButton(IDC_REGS_DEC, pCtx->decimal);
#endif
}

void CRegisterBar::UpdateRegPC(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
#if REWRITE_TO_WX_WIDGETS
    CDeasm deasm;
    int ptr= -1;
    SetDlgItemText(IDC_REGS_INSTR, deasm.DeasmInstr(*pCtx, CAsm::DF_BRANCH_INFO, ptr));
    UpdateItem(IDC_REGS_INSTR);
    SetDlgItemText(IDC_REGS_INSTR_ARG, deasm.ArgumentValue(*pCtx));
    UpdateItem(IDC_REGS_INSTR_ARG);
#endif
}

void CRegisterBar::UpdateRegS(const CContext *pCtx, const CContext *pOld /*= NULL*/)
{
#if REWRITE_TO_WX_WIDGETS
    if (pCtx->s != 0xFF) // is there anything on the stack?
        SetDlgItemMem(IDC_REGS_S_MEM, 0xFF - pCtx->s, pCtx->s + 0x0100 + 1, pCtx);
    else // We write that nothing
    {
        std::string str;
        str.LoadString(IDS_REGS_S_EMPTY);
        SetDlgItemText(IDC_REGS_S_MEM, str);
    }
#endif
}

void CRegisterBar::UpdateCycles(ULONG uCycles)
{
#if REWRITE_TO_WX_WIDGETS
    SetDlgItemInt(IDC_REGS_CYCLES, uCycles, false);
    UpdateItem(IDC_REGS_CYCLES);
#endif
}

//=============================================================================

void CRegisterBar::ChangeRegister(int ID, int reg_no)
{
#if REWRITE_TO_WX_WIDGET
    if (m_bInUpdate || wxGetApp().m_global.IsProgramRunning()) // Update or is the program running?
        return; // Ignoring changes

    std::string buf;

    if (GetDlgItemText(ID, buf) == 0)
        return;

    const char *str = buf.c_str();
    int num;

    if (str[0]== '$')
    {
        if (sscanf(str + 1, "%X", &num) <= 0)
            num = 0;
    }
    else if (str[0] == '0' && str[1] == 'x' || str[1]=='X')
    {
        if (sscanf(str + 2, "%X", &num) <= 0)
            num = 0;
    }
    else if (sscanf(str, "%u", &num) <= 0)
        num = 0;

    CSym6502 *pSym = wxGetApp().m_global.GetSimulator();
    if (pSym == nullptr)
    {
        ASSERT(false);
        return;
    }

    m_bInUpdate = true;

    CContext ctx(*(pSym->GetContext())); // program context

    switch (reg_no) // Update register that was changed
    {
    case 0:
        ctx.a = uint8_t(num);
        UpdateRegA(&ctx);
        break;

    case 1:
        ctx.x = uint8_t(num);
        UpdateRegX(&ctx);
        break;

    case 2:
        ctx.y = uint8_t(num);
        UpdateRegY(&ctx);
        break;

    case 3:
        ctx.s = uint8_t(num);
        UpdateRegS(&ctx);
        break;

    case 4:
        ctx.set_status_reg_bits(uint8_t(num));

        if (!wxGetApp().m_global.GetProcType()) // 65c02?
            ctx.reserved = true;                // 'reserved' bit always set
        
        UpdateRegP(&ctx);
        break;

    case 5:
        ctx.pc = uint16_t(num) & ctx.mem_mask;
        UpdateRegPC(&ctx);
        break;

    default:
        ASSERT(false);
        break;
    }

    pSym->SetContext(ctx); // context change

    if (reg_no == 5 && !wxGetApp().m_global.IsProgramFinished()) // PC change?
        pSym->SkipToAddr(ctx.pc);

    m_bInUpdate = false;
#endif
}

void CRegisterBar::ChangeFlags(int flag_bit, bool set) // changing the flag register bit
{
    CSym6502 *pSym = wxGetApp().m_global.GetSimulator();

    if (pSym == nullptr)
    {
        ASSERT(false);
        return;
    }

    CContext ctx(*(pSym->GetContext())); // program context
    uint8_t flags = ctx.get_status_reg();

    if (set)
        flags |= uint8_t(1 << flag_bit);
    else
        flags &= ~uint8_t(1 << flag_bit);

    ctx.set_status_reg_bits(flags);

    if (wxGetApp().m_global.GetProcType() != ProcessorType::M6502)
        ctx.reserved = true; // 'reserved' bit always set

    SetDlgItemByteHex(IDC_REGS_P, ctx.get_status_reg());

    pSym->SetContext(ctx); // context change
}

//-----------------------------------------------------------------------------

void CRegisterBar::OnChangeRegA()
{
    ChangeRegister(IDC_REGS_A,0);
}

void CRegisterBar::OnChangeRegX()
{
    ChangeRegister(IDC_REGS_X,1);
}

void CRegisterBar::OnChangeRegY()
{
    ChangeRegister(IDC_REGS_Y,2);
}

void CRegisterBar::OnChangeRegS()
{
    ChangeRegister(IDC_REGS_S,3);
}

void CRegisterBar::OnChangeRegP()
{
    ChangeRegister(IDC_REGS_P,4);
}

void CRegisterBar::OnChangeRegPC()
{
    ChangeRegister(IDC_REGS_PC,5);
}

void CRegisterBar::UpdateFlag(int checkId, int statusBit)
{
    wxCheckBox *btn = dynamic_cast<wxCheckBox*>(FindWindow(checkId));

    if (!btn)
        return;

    ChangeFlags(statusBit, btn->IsChecked());
}

void CRegisterBar::OnRegFlagNeg()
{
    UpdateFlag(IDC_REGS_NEG, CContext::N_NEGATIVE);
}

void CRegisterBar::OnRegFlagCarry()
{
    UpdateFlag(IDC_REGS_CARRY, CContext::N_CARRY);
}

void CRegisterBar::OnRegFlagDec()
{
    UpdateFlag(IDC_REGS_DEC, CContext::N_DECIMAL);
}

void CRegisterBar::OnRegFlagInt()
{
    UpdateFlag(IDC_REGS_INT, CContext::N_INTERRUPT);
}

void CRegisterBar::OnRegFlagOver()
{
    UpdateFlag(IDC_REGS_OVER, CContext::N_OVERFLOW);
}

void CRegisterBar::OnRegFlagZero()
{
    UpdateFlag(IDC_REGS_ZERO, CContext::N_ZERO);
}

void CRegisterBar::OnRegFlagBrk()
{
    UpdateFlag(IDC_REGS_BRK, CContext::N_BREAK);
}

void CRegisterBar::OnRegsCyclesClr()
{
    if (wxGetApp().m_global.IsProgramRunning())
    {
#if REWRITE_TO_WX_WIDGETS
        MessageBeep(-2);
#endif
        return;
    }

    CSym6502 *pSym = wxGetApp().m_global.GetSimulator();
    
    if (pSym == nullptr)
    {
        ASSERT(false);
        return;
    }

    pSym->ClearCyclesCounter();
    UpdateCycles(0);
}
