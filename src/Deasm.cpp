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

#include "StdAfx.h"
#include "6502.h"
#include "Deasm.h"
#include "M6502.h"

std::string CDeasm::DeasmInstr(const CmdInfo &ci, CAsm::DeasmFmt flags)
{
    std::string str;
    wxString fmt;

    uint32_t addr = ci.pc;
    uint8_t cmd = ci.cmd;
    uint16_t uLen = cmd == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[cmd]];
    if (cmd == CAsm::C_BRK && !CAsm6502::generateBRKExtraByte)
        uLen = 1;

    if (flags & CAsm::DF_ADDRESS)
    {
        fmt.Printf("%06X  ", (int)addr);
        str += fmt;
    }

    if (flags & CAsm::DF_CODE_BYTES)
    {
        switch (uLen)
        {
        case 1:
            fmt.Printf("%02X           ", int(cmd));
            break;

        case 2:
            fmt.Printf("%02X %02X        ", int(cmd), int(ci.arg1));
            break;

        case 3:
            fmt.Printf("%02X %02X %02X     ", int(cmd), int(ci.arg1), int(ci.arg2));
            break;

        case 4:
            fmt.Printf("%02X %02X %02X %02X  ", int(cmd), int(ci.arg1), int(ci.arg2), int(ci.arg3));
            break;

        default:
            ASSERT(FALSE);
            break;
        }

        str += fmt.ToStdString();
    }

    ProcessorType procType = wxGetApp().simulatorController().processor();
    str += Mnemonic(cmd, procType, !!(flags & CAsm::DF_USE_BRK));

    str += Argument(cmd, (CAsm::CodeAdr)CAsm::CodeToMode(procType)[cmd], addr, ci.arg1, ci.arg2, ci.arg3, flags & CAsm::DF_LABELS, flags & CAsm::DF_HELP);

    return str;
}

std::string CDeasm::DeasmInstr(CAsm::DeasmFmt flags, int &ptr)
{
    ASSERT((ptr == -1) || ((ptr >= 0) && (ptr <= 0xFFFFFF)));

    std::string str;
    str.reserve(128); // Preallocate some initial space to work with.

    sim_addr_t addr;

    ProcessorType procType = wxGetApp().simulatorController().processor();

    const CContext &ctx =m_sim->GetContext();

    addr = (ptr >= 0) ? ptr : ctx.getProgramAddress();

    uint8_t cmd = ctx.peekByte(addr);
    uint16_t len = cmd == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[cmd]];

    if (cmd == 0 && CAsm6502::generateBRKExtraByte)
        len = 2;

    if (flags & CAsm::DF_ADDRESS)
        str += fmt::format("%06X  ", addr);

    if (flags & CAsm::DF_CODE_BYTES)
    {
        int pad = 13;

        for (uint16_t i = 0; i < len; ++i, pad -= 3)
            str += fmt::format("%02X ", (int)(ctx.peekByte(addr + i)));

        // Add remaining needed padding
        str += std::string(pad, ' ');
    }

    str += Mnemonic(cmd, procType, 1); //% Bug fix 1.2.12.2 - allow BRK vs. .DB in disassembly listings

    int mode = CAsm::CodeToMode(procType)[cmd];

    if (procType == ProcessorType::WDC65816 && !ctx.emm)
    {
        if (cmd == 0xA2 && !ctx.xy16)
            mode = CAsm::A_IMM2;
        else if (cmd == 0xA0 && !ctx.xy16)
            mode = CAsm::A_IMM2;
        else if (mode == 2 && !ctx.mem16)
            mode = CAsm::A_IMM2;
    }

    str += Argument(
        cmd,
        (CAsm::CodeAdr)mode,
        addr,
        ctx.peekByte(addr + 1), ctx.peekByte(addr + 2), ctx.peekByte(addr + 3),
        flags & CAsm::DF_LABELS);

    if (flags & CAsm::DF_BRANCH_INFO)
    {
        bool sign = false;
        switch (CAsm::CodeToCommand()[cmd])
        {
        case CAsm::C_BRL: // 65816
        case CAsm::C_BRA:
            sign = true;
            break;

        case CAsm::C_BPL:
            if (!ctx.negative)
                sign = true;
            break;

        case CAsm::C_BMI:
            if (ctx.negative)
                sign = true;
            break;

        case CAsm::C_BVC:
            if (!ctx.overflow)
                sign = true;
            break;

        case CAsm::C_BVS:
            if (ctx.overflow)
                sign = true;
            break;

        case CAsm::C_BCC:
            if (!ctx.carry)
                sign = true;
            break;

        case CAsm::C_BCS:
            if (ctx.carry)
                sign = true;
            break;

        case CAsm::C_BNE:
            if (!ctx.zero)
                sign = true;
            break;

        case CAsm::C_BEQ:
            if (ctx.zero)
                sign = true;
            break;

        case CAsm::C_BBS:
        {
            uint8_t zpg = ctx.peekByte(addr + 1);
            int bit_no = (cmd >> 4) & 0x07;
            if (ctx.peekByte(zpg) & uint8_t(1 << bit_no))
                sign = true;
            break;
        }

        case CAsm::C_BBR:
        {
            uint8_t zpg = ctx.peekByte(addr + 1);
            int bit_no = (cmd >> 4) & 0x07;
            if (!(ctx.peekByte(zpg) & uint8_t(1 << bit_no)))
                sign = true;
            break;
        }
        } // switch (CodeToCommand()[cmd])

        if (sign)
            str += " ->"; // indication of active jump
    }

    ptr = (addr + len) & 0xFFFFFF; // adr next instr.

    return str;
}

std::string CDeasm::Mnemonic(uint8_t code, ProcessorType procType, bool bUseBrk/*= false*/) const
{
    ASSERT(CAsm::CodeToCommand(procType)[code] <= CAsm::C_ILL && CAsm::CodeToCommand(procType)[code] >= 0);
    char buf[16];

    uint8_t cmd = CAsm::CodeToCommand(procType)[code];

    if (cmd == CAsm::C_ILL || (cmd == CAsm::C_BRK && !bUseBrk)) // Illegal command code or BRK
        snprintf(buf, sizeof(buf), ".DB $%02X", int(code));
    else
    {
        memcpy(buf, mnemonics + 3 * cmd, 3);
        buf[3] = '\0';
    }

    return std::string(buf);
}

const char CDeasm::mnemonics[] =
"LDALDXLDYSTASTXSTYSTZTAXTXATAYTYATXSTSXADCSBCCMPCPXCPYINCDECINADEAINXDEXINYDEY"
"ASLLSRROLRORANDORAEORBITTSBTRBJMPJSRBRKBRABPLBMIBVCBVSBCCBCSBNEBEQRTSRTIPHAPLA"
"PHXPLXPHYPLYPHPPLPCLCSECCLVCLDSEDCLISEINOP"
"BBRBBSRMBSMB"
"BRLCOPJMLJSLMVNMVPPEAPEIPERPHBPHDPHKPLBPLDREPRTLSEPSTPTCDTCSTDCTSCTXYTYXWAIWDMXBAXCE"
"???";

std::string CDeasm::Argument(uint8_t cmd, CAsm::CodeAdr mode, uint32_t addr, uint8_t arg1, uint8_t arg2, uint8_t arg3, bool bLabel, bool bHelp) const
{
    wxString str;
    addr++;
    uint8_t lo = arg1;
    uint16_t word = arg1 + (arg2 << 8);
    uint32_t big = arg1 + (arg2 << 8) + (arg3 << 16);

    switch (mode)
    {
    case CAsm::A_IMP: // implied
    case CAsm::A_IMP2: // implied for BRK
        if (bHelp)
            str = "           |Implied";
        break;

    case CAsm::A_ACC: // accumulator
        if (bHelp)
            str = "           |Accumulator";
        break;

    case CAsm::A_ILL: // Value to mark illegal commands in the simulator (ILLEGAL)
        break;

    case CAsm::A_IMM: // immediate
        str.Printf(" #$%02X", (int)lo);
        if (bHelp)
            str += "      |Immediate";
        break;

    case CAsm::A_ZPG: // zero page
        str.Printf(bLabel ? " z%02X" : " $%02X", (int)lo);
        if (bHelp)
            str += "       |Zero Page";
        break;

    case CAsm::A_ABS: // absolute
        str.Printf(bLabel ? " a%04X" : " $%04X", (int)word);
        if (bHelp)
            str += "     |Absolute";
        break;

    case CAsm::A_ABS_X: // absolute indexed X
        str.Printf(bLabel ? " a%04X,X" : " $%04X,X", (int)word);
        if (bHelp)
            str += "   |Absolute Indexed, X";
        break;

    case CAsm::A_ABS_Y: // absolute indexed Y
        str.Printf(bLabel ? " a%04X,Y" : " $%04X,Y", (int)word);
        if (bHelp)
            str += "   |Absolute Indexed, Y";
        break;

    case CAsm::A_ZPG_X: // zero page indexed X
        str.Printf(bLabel ? " z%02X,X" : " $%02X,X", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indexed, X";
        break;

    case CAsm::A_ZPG_Y: // zero page indexed Y
        str.Printf(bLabel ? " z%02X,Y" : " $%02X,Y", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indexed, Y";
        break;

    case CAsm::A_REL: // relative
        if (bHelp)
            str = " label    |Relative";
        else
            str.Printf(bLabel ? " e%04X" : " $%04X", int(lo & 0x80 ? addr + 1 - (0x100 - lo) : addr + 1 + lo));
        break;

    case CAsm::A_ZPGI: // zero page indirect
        str.Printf(bLabel ? " (z%02X)" : " ($%02X)", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indirect";
        break;

    case CAsm::A_ZPGI_X: // zero page indirect, indexed X
        str.Printf(bLabel ? " (z%02X,X)" : " ($%02X,X)", (int)lo);
        if (bHelp)
            str += "   |Zero Page Indexed X, Indirect";
        break;

    case CAsm::A_ZPGI_Y: // zero page indirect, indexed Y
        str.Printf(bLabel ? " (z%02X),Y" : " ($%02X),Y", (int)lo);
        if (bHelp)
            str += "   |Zero Page Indirect, Indexed Y";
        break;

    case CAsm::A_ABSI: // absolute indirect
        str.Printf(bLabel ? " (a%04X)" : " ($%04X)", (int)word);
        if (bHelp)
            str += "   |Absolute Indirect";
        break;

    case CAsm::A_ABSI_X: // absolute indirect, indexed X
        str.Printf(bLabel ? " (a%04X,X)" : " ($%04X,X)", (int)word);
        if (bHelp)
            str += " |Absolute Indexed X, Indirect";
        break;

    case CAsm::A_ZPG2: // zero page dla RMB i SMB
    {
        unsigned int bit_no = (cmd >> 4) & 0x07;
        str.Printf(bLabel ? " #%u,z%02X" : " #%u,$%02X", (unsigned int)bit_no, (int)lo);
        if (bHelp)
            str += "   |Memory Bit Manipulation";
        break;
    }

    case CAsm::A_ZREL: // zero page / relative dla BBS i BBR
    {
        uint8_t hi = arg2; //ctx.mem[addr + 1];
        unsigned int bit_no = (cmd >> 4) & 0x07;
        str.Printf(bLabel ? " #%u,z%02X,e%04X" : " #%u,$%02X,$%04X", bit_no, int(lo), int(hi & 0x80 ? addr + 2 - (0x100 - hi) : addr + 2 + hi));
        if (bHelp)
            str += "   |Relative Bit Branch";
        break;
    }

    case CAsm::A_ABSL:
        str.Printf(bLabel ? " a%06X" : " $%06X", big);
        if (bHelp)
            str += "   |Absolute Long";
        break;

    case CAsm::A_ABSL_X:
        str.Printf(bLabel ? " a%06X,X" : " $%06X,X", big);
        if (bHelp)
            str += " |Absolute long indexed X";
        break;

    case CAsm::A_ZPIL: // zero page indirect
        str.Printf(bLabel ? " [z%02X]" : " [$%02X]", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indirect Long";
        break;

    case CAsm::A_ZPIL_Y: // zero page indirect, indexed Y
        str.Printf(bLabel ? " [z%02X],Y" : " [$%02X],Y", (int)lo);
        if (bHelp)
            str += "   |Zero Page Indirect Long, Indexed Y";
        break;

    case CAsm::A_SR:	// zero page indirect
        str.Printf(bLabel ? " z%02X,S" : " $%02X,S", (int)lo);
        if (bHelp)
            str += "     |Stack Relative";
        break;

    case CAsm::A_SRI_Y: // zero page indirect, indexed Y
        str.Printf(bLabel ? " (z%02X,S),Y" : " ($%02X,S),Y", (int)lo);
        if (bHelp)
            str += " |Stack Relative Indirect, Indexed Y";
        break;

    case CAsm::A_RELL: // Relative Long
        if (bHelp)
            str = " label    |Relative Long";
        else
            str.Printf(bLabel ? " e%04X" : " $%04X", int(word & 0x8000 ? addr + 2 - (0x10000 - word) : addr + 2 + word));
        break;

    case CAsm::A_XYC:
        str.Printf(bLabel ? " b%02X, b%02X" : " $%02X, $%02X", (uint8_t)word & 0xFF, (uint8_t)(word >> 8) & 0xFF);
        if (bHelp)
            str += "   |Block Move - Source bank, Destination bank";
        break;

    case CAsm::A_IMM2:
        str.Printf(" #$%04X", word);
        if (bHelp)
            str += "    |Immediate Long";
        break;

    default:
        ASSERT(false);
        break; // Keeps some compilers happy.
    }

    return str.ToStdString();
}

std::string CDeasm::ArgumentValue(uint32_t cmd_addr /*= -1*/)
{
    wxString str("-");
    uint8_t arg;
    uint32_t addr, tmp;
    uint32_t laddr;

    ASSERT((cmd_addr == CAsm::INVALID_ADDRESS) || (cmd_addr <= 0xFFFF)); // Invalid address

    const CContext &ctx =m_sim->GetContext();

    if (cmd_addr == CAsm::INVALID_ADDRESS)
    {
        if (wxGetApp().m_global.m_bBank)
            cmd_addr = ctx.pc + (ctx.pbr << 16);
        else
            cmd_addr = ctx.pc;
    }

    uint8_t cmd = ctx.peekByte(cmd_addr);
    uint8_t mode = CAsm::CodeToMode()[cmd];
    ++cmd_addr;

    if (wxGetApp().m_global.m_bBank)
    {
        if ((cmd == 0xA9) && !ctx.mem16)
            mode = CAsm::A_IMM2;
        if ((cmd == 0xA2) && !ctx.xy16)
            mode = CAsm::A_IMM2;
        if ((cmd == 0xA0) && !ctx.xy16)
            mode = CAsm::A_IMM2;
    }

    sim_addr_t sp = ctx.getStackPointer();

    switch (mode)
    {
    case CAsm::A_IMP:
        switch (cmd)
        {
        case 0x60: // RTS
            addr = ctx.peekWord(sp) + 1;
            return fmt::format("RTS->$%04X", addr);

        case 0x6B: // RTL
            addr = ctx.peekLWord(sp) + 1;
            return fmt::format("RTL->$%06X", addr);

        case 0xAB: // PLB
            addr = (ctx.peekByte(sp + 1));
            return fmt::format("DBR=$%02X", addr);

        case 0x2B: // PLD
            addr = ctx.peekWord(sp + 1);
            return fmt::format("DIR=$%04X", addr);

        case 0x68: // PLA
            addr = ctx.peekByte(sp + 1);

            if (!ctx.mem16)
            {
                addr += ctx.peekByte(sp + 2) << 8;
                return fmt::format("A=$%04X", addr);
            }

            return fmt::format("A=$%02X", addr);

        case 0xFA: // PLX
            addr = ctx.peekByte(sp + 1);

            if (!ctx.xy16)
            {
                addr += ctx.peekByte(sp + 2) << 8;
                return fmt::format("X=$%04X", addr);
            }

            return fmt::format("X=$%02X", addr);

        case 0x7A: // PLY
            addr = ctx.peekByte(sp + 1);

            if (!ctx.xy16)
            {
                addr += ctx.peekByte(sp + 2) << 8;
                return fmt::format("Y=$%04X", addr);
            }
            
            return fmt::format("Y=$%02X", addr);

        case 0x28: // PLP
            addr = ctx.peekByte(sp + 1);
            return fmt::format("P=$%02X", addr);
        }

    case CAsm::A_IMP2:
    case CAsm::A_ACC:
        break;

    case CAsm::A_IMM:
        return SetValInfo(ctx.peekByte(cmd_addr));

    case CAsm::A_REL:
        arg = ctx.peekByte(cmd_addr);
        if (arg & 0x80) // Jump back
            str.Printf("PC-$%02X", int(0x100 - arg));
        else // Jump forward
            str.Printf("PC+$%02X", int(arg));
        break;

    case CAsm::A_ZPGI:
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        addr = ctx.peekWord(arg); // Reference address through cells
        //addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.peekByte(addr));

    case CAsm::A_ZPG:
    case CAsm::A_ZPG2:
        addr = ctx.peekByte(cmd_addr);
        return SetMemZPGInfo((uint8_t)addr, ctx.peekByte(addr));

    case CAsm::A_ZPG_X:
        addr = (ctx.peekByte(cmd_addr) + ctx.x) & 0xFF;
        return SetMemZPGInfo((uint8_t)addr, ctx.peekByte(addr));

    case CAsm::A_ZPG_Y:
        addr = (ctx.peekByte(cmd_addr) + ctx.y) & 0xFF;
        return SetMemZPGInfo((uint8_t)addr, ctx.peekByte(addr));

    case CAsm::A_ABS:
        addr = ctx.peekWord(cmd_addr);
        //addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.peekByte(addr));

    case CAsm::A_ABSI:
        addr = ctx.peekWord(cmd_addr);
        //addr &= ctx.mem_mask;
        tmp = ctx.peekWord(addr);
        //tmp &= ctx.mem_mask;
        return SetMemInfo(tmp, ctx.peekByte(tmp));

    case CAsm::A_ABSI_X:
        addr = ctx.peekByte(cmd_addr) + ctx.x; // Low byte of address + X offset

        if (wxGetApp().m_global.GetProcType() != ProcessorType::M6502 &&
            (cmd_addr & 0xFF) == 0xFF) // low byte == 0xFF?
        {
            addr |= uint16_t(ctx.peekByte(cmd_addr - 0xFF)) << 8; // 65C02 addressing bug
        }
        else
            addr |= uint16_t(ctx.peekByte(cmd_addr + 1)) << 8;

        //addr &= ctx.mem_mask;
        tmp = ctx.peekWord(addr); // Number at address
        //tmp &= ctx.mem_mask;
        return SetMemInfo(tmp, ctx.peekByte(tmp));

    case CAsm::A_ABS_X:
        addr = ctx.peekWord(cmd_addr) + ctx.x;
        //addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.peekByte(addr));

    case CAsm::A_ABS_Y:
        addr = ctx.peekWord(cmd_addr) + ctx.y;
        //addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.peekByte(addr));

    case CAsm::A_ZPGI_X:
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        arg = (arg + ctx.x) & 0xFF;
        addr = ctx.peekWord(arg); // Reference address through cells
        //addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.peekByte(addr));

    case CAsm::A_ZPGI_Y:
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        addr = ctx.peekWord(arg) + ctx.y;
        //addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.peekByte(addr));

    case CAsm::A_ZREL:
    {
        std::string tmpStr = SetMemZPGInfo((uint8_t)cmd_addr, ctx.peekByte(cmd_addr));
        arg = ctx.peekByte(cmd_addr + 1);
        if (arg & 0x80) // Jump back
            str.Printf("; PC-$%02X", int(0x100 - arg));
        else // Jump forward
            str.Printf("; PC+$%02X", int(arg));
        return tmpStr + str.ToStdString();
    }

    case CAsm::A_ABSL:
        laddr = ctx.peekLWord(cmd_addr);
        return SetMemInfo(laddr, ctx.peekByte(laddr));

    case CAsm::A_ABSL_X:
        laddr = ctx.peekLWord(cmd_addr) + ctx.x;
        return SetMemInfo(laddr, ctx.peekByte(laddr));

    case CAsm::A_ZPIL: // zero page indirect
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        laddr = ctx.peekLWord(arg); // Reference address through cells
        return SetMemInfo(laddr, ctx.peekByte(laddr));

    case CAsm::A_ZPIL_Y: // zero page indirect, indexed Y
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        laddr = ctx.peekLWord(arg) + ctx.y; // Reference address through cells
        return SetMemInfo(laddr, ctx.peekByte(laddr));

    case CAsm::A_SR: // zero page indirect
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        laddr = ctx.peekByte(arg); // Reference address through cells
        return SetMemInfo(laddr, ctx.peekByte(laddr));

    case CAsm::A_SRI_Y: // zero page indirect, indexed Y
        arg = ctx.peekByte(cmd_addr); // Cell address on zero page
        laddr = ctx.peekByte(arg) + ctx.y; // Reference address through cells
        return SetMemInfo(laddr, ctx.peekByte(laddr));

    case CAsm::A_RELL: // Relative Long
        addr = ctx.peekWord(cmd_addr);
        if (addr & 0x8000) // Jump back
            str.Printf("PC-$%04X", int(0x10000 - addr));
        else // Jump forward
            str.Printf("PC+$%04X", int(addr));
        break;

    case CAsm::A_XYC:
        addr = ctx.peekWord(cmd_addr);
        return fmt::format("S-%02X, D-%02X", (uint8_t)addr & 0xFF, (uint8_t)(addr >> 8) & 0xFF);

    case CAsm::A_IMM2:
        addr = ctx.peekWord(cmd_addr);
        return SetWordInfo(addr);

    case CAsm::A_ILL:
        return "";

    default:
        ASSERT(FALSE);
        return fmt::format("MISSING MODE=%D", mode);
    }

    return str.ToStdString();
}

std::string CDeasm::SetMemInfo(uint32_t addr, uint8_t val) // Description of the memory location
{
    char c = val ? val : ' ';
    return fmt::format("[{0:06X}]: ${1:02X}, {1:d}, {2:c}, {1:08b}", addr, val, c);
}

std::string CDeasm::SetMemZPGInfo(uint8_t addr, uint8_t val) // Cell description of page zero of memory
{
    char c = val ? val : ' ';
    return fmt::format("[{0:02X}]: ${1:02X}, {1:d}, {2:c}, {1:08b}", addr, val, c);
}

std::string CDeasm::SetValInfo(uint8_t val)	// Value description 'val'
{
    char c = val ? val : ' ';
    return fmt::format("{0:d}, {1:c}, {0:08b}", val, c);
}

std::string CDeasm::SetWordInfo(uint16_t word)
{
    char c = (word & 0xFF);
    c = c ? c : ' ';

    return fmt::format("{0:d}, {1:c}, {0:016b}", word, c);
}

// Finding the address of the instruction preceding a given instruction
int CDeasm::FindPrevAddr(uint32_t &addr, int cnt/*= 1*/)
{
    ASSERT(cnt >= 0);

    if (cnt <= 0)
        return 0;

    const CContext &ctx =m_sim->GetContext();

    if (cnt > 1)
    {
        int len = std::max(10, cnt * 3) + 2;
        uint32_t start = int(addr) - len > 0 ? addr - len : 0;
        start &= ~1; // Even address
        std::vector<uint16_t> addresses;
        uint8_t cmd;
        int ret = 0;
        int i;

        addresses.reserve(len + 4);

        for (i = 0; start < addr; i++)
        {
            addresses[i] = start;
            cmd = ctx.peekByte(start);
            start += cmd == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[cmd]];
        }

        if (start == addr)
            ret = 1;
        else
            ret = -1;

        if (i - cnt < 0)
            addr = 0;
        else
            addr = addresses[i - cnt];
        return ret;
    }
    else
    {
        uint32_t start = int(addr) - 10 > 0 ? addr - 10 : 0;
        uint32_t prev = start;
        int ret = 0;
        //ASSERT(addr <= ctx.mem_mask); // Invalid address; too big
        uint8_t cmd;

        while (start < addr)
        {
            prev = start;
            cmd = ctx.peekByte(start);
            if (cmd == 0) // BRK command?
                start++;  // We only increase by 1, although normally BRK increases by 2
            else
                start += CAsm::mode_to_len[CAsm::CodeToMode()[cmd]];

            //start &= ctx.mem_mask;
        }

        cmd = ctx.peekByte(prev);
        /*
            if (cmd == 0 && ctx.mem[prev - 1] == 0) // What about zeros before the command?
            {
              prev--;  // We move the address back by 2
              ret = 1; // because it is one BRK instruction
            }
            else
        */
        if (prev == addr)
            ret = 0; // We are at the beginning -there is no offset
        else if (static_cast<uint32_t>(prev + (cmd == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[cmd]])) == addr)
            ret = 1; // is moved by one line
        else
            ret = -1; // there is a shift, but it affected the change of subsequent orders

        addr = prev;
        return ret;
    }
}

// Finding the address of the command following the 'cnt' command from 'addr'
int CDeasm::FindNextAddr(uint32_t &addr, int cnt/*= 1*/)
{
    int ret = 0;
    uint32_t next = addr;

    const CContext &ctx =m_sim->GetContext();

    for (uint32_t address = addr; cnt; cnt--)
    {
        address = next;
        uint8_t byte = ctx.peekByte(address);
        next += byte == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[byte]];

        if (next < addr)
            ret = 0; // "scroll" the address
        else
            ret = 1; // next address found

        if (address >= ctx.bus.maxAddress())
            next = address;
    }

    addr = next;
    return ret;
}

// Check how many lines should the window content be moved to reach from 'addr' to 'dest'
int CDeasm::FindDelta(uint32_t &addr, uint32_t dest, int max_lines)
{
    if (dest == addr)
        return 0;

    const CContext &ctx =m_sim->GetContext();

    if (dest < addr)
    {
        uint32_t start = dest;
        int i;

        for (i = 0; start < addr; i++)
        {
            uint8_t byte = ctx.peekByte(start);
            start += byte == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[byte]];

            if (i >= max_lines)
                break;
        }

        i = start == addr ? i : -i;
        addr = dest;
        return i;
    }
    else
    {
        uint32_t start = addr;
        int i;

        for (i = 0; start < dest; i++)
        {
            uint8_t byte = ctx.peekByte(start);
            start += byte == 0 ? 1 : CAsm::mode_to_len[CAsm::CodeToMode()[byte]];

            if (i >= max_lines)
                break;
        }

        i = start == addr ? i : -i;
        addr = dest;
        return i;
    }
}
