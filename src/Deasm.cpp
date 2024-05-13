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
#include "Deasm.h"

std::string CDeasm::DeasmInstr(const CmdInfo& ci, DeasmFmt flags)
{
    std::string str;
    wxString fmt;
    
    uint32_t addr = ci.pc;
    uint8_t cmd = ci.cmd;
    uint16_t uLen = cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];

    if (flags & DF_ADDRESS)
    {
        fmt.Printf("%06X  ", (int)addr);
        str += fmt;
    }

    if (flags & DF_CODE_BYTES)
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
            fmt.Printf("%02X %02X %02X    ", int(cmd), int(ci.arg1), int(ci.arg2));
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

    ProcessorType procType = wxGetApp().m_global.m_procType;
    str += Mnemonic(cmd, procType, !!(flags & DF_USE_BRK));

    str += Argument(cmd, (CodeAdr)CodeToMode(procType)[cmd], addr, ci.arg1, ci.arg2, ci.arg3, flags & DF_LABELS, flags & DF_HELP);

    return str;
}

std::string CDeasm::DeasmInstr(const CContext& ctx, DeasmFmt flags, int& ptr)
{
    ASSERT((ptr == -1) || ((ptr >= 0) && (ptr <= 0xFFFF)));

    std::string str;
    wxString fmt;

    uint32_t addr = ptr >= 0 ? ptr : ctx.pc;
    uint8_t cmd = ctx.mem[addr];
    uint16_t uLen = cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];

    if (flags & DF_ADDRESS)
    {
        fmt.Printf("%06X  ", addr);
        str += fmt;
    }

    if (flags & DF_CODE_BYTES)
    {
        switch (uLen)
        {
        case 1:
            fmt.Printf("%02X           ", int(ctx.mem[addr]));
            break;

        case 2:
            fmt.Printf("%02X %02X        ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]));
            break;

        case 3:
            fmt.Printf("%02X %02X %02X     ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]), int(ctx.mem[addr + 2]));
            break;

        case 4:
            fmt.Printf("%02X %02X %02X %02X  ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]), int(ctx.mem[addr + 2]), int(ctx.mem[addr + 3]));
            break;

        default:
            ASSERT(FALSE);
            break;
        }

        str += fmt.ToStdString();
    }

    ProcessorType procType = wxGetApp().m_global.m_procType;
    str += Mnemonic(cmd, procType, 1); //% Bug fix 1.2.12.2 - allow BRK vs. .DB in disassembly listings

    str += Argument(cmd, (CodeAdr)CodeToMode(procType)[cmd], addr, ctx.mem[addr+1], ctx.mem[addr+2], ctx.mem[addr+3], flags & DF_LABELS);

    if (flags & DF_BRANCH_INFO)
    {
        bool sign = false;
        switch (CodeToCommand()[cmd])
        {
        case C_BRL:	// 65816
        case C_BRA:
            sign = true;
            break;

        case C_BPL:
            if (!ctx.negative)
                sign = true;
            break;

        case C_BMI:
            if (ctx.negative)
                sign = true;
            break;

        case C_BVC:
            if (!ctx.overflow)
                sign = true;
            break;

        case C_BVS:
            if (ctx.overflow)
                sign = true;
            break;

        case C_BCC:
            if (!ctx.carry)
                sign = true;
            break;

        case C_BCS:
            if (ctx.carry)
                sign = true;
            break;

        case C_BNE:
            if (!ctx.zero)
                sign = true;
            break;

        case C_BEQ:
            if (ctx.zero)
                sign = true;
            break;

        case C_BBS:
        {
            uint8_t zpg = ctx.mem[addr + 1];
            int bit_no = (cmd >> 4) & 0x07;
            if (ctx.mem[zpg] & uint8_t(1 << bit_no))
                sign = true;
            break;
        }

        case C_BBR:
        {
            uint8_t zpg = ctx.mem[addr + 1];
            int bit_no = (cmd >> 4) & 0x07;
            if (!(ctx.mem[zpg] & uint8_t(1 << bit_no)))
                sign = true;
            break;
        }
        } // switch (CodeToCommand()[cmd])
        
        if (sign)
            str += " ->"; // indication of active jump
    }

    ptr = (addr + uLen) & 0xFFFF; // adr nast. instr.

    return str;
}

std::string CDeasm::Mnemonic(uint8_t code, ProcessorType procType, bool bUseBrk/*= false*/)
{
    ASSERT(CodeToCommand(procType)[code] <= C_ILL && CodeToCommand(procType)[code] >= 0);
    char buf[16];

    uint8_t cmd = CodeToCommand(procType)[code];

    if (cmd == C_ILL || (cmd == C_BRK && !bUseBrk)) // Illegal command code or BRK
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

std::string CDeasm::Argument(uint8_t cmd, CodeAdr mode, uint32_t addr, uint8_t arg1, uint8_t arg2, uint8_t arg3, bool bLabel, bool bHelp)
{
    wxString str;
    addr++;
    uint8_t lo = arg1;
    uint16_t word = arg1 + (arg2 << 8);
    uint32_t big =  arg1 + (arg2 << 8) + (arg3 << 16);

    switch (mode)
    {
    case A_IMP: // implied
    case A_IMP2: // implied for BRK
        if (bHelp)
            str = "           |Implied";
        break;

    case A_ACC: // accumulator
        if (bHelp)
            str = "           |Accumulator";
        break;

    case A_ILL: // Value to mark illegal commands in the simulator (ILLEGAL)
        break;

    case A_IMM: // immediate
        str.Printf(" #$%02X", (int)lo);
        if (bHelp)
            str += "      |Immediate";
        break;

    case A_ZPG: // zero page
        str.Printf(bLabel ? " z%02X" : " $%02X", (int)lo);
        if (bHelp)
            str += "       |Zero Page";
        break;

    case A_ABS: // absolute
        str.Printf(bLabel ? " a%04X" : " $%04X", (int)word);
        if (bHelp)
            str += "     |Absolute";
        break;

    case A_ABS_X: // absolute indexed X
        str.Printf(bLabel ? " a%04X,X" : " $%04X,X", (int)word);
        if (bHelp)
            str += "   |Absolute Indexed, X";
        break;

    case A_ABS_Y: // absolute indexed Y
        str.Printf(bLabel ? " a%04X,Y" : " $%04X,Y", (int)word);
        if (bHelp)
            str += "   |Absolute Indexed, Y";
        break;

    case A_ZPG_X: // zero page indexed X
        str.Printf(bLabel ? " z%02X,X" : " $%02X,X", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indexed, X";
        break;

    case A_ZPG_Y: // zero page indexed Y
        str.Printf(bLabel ? " z%02X,Y" : " $%02X,Y", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indexed, Y";
        break;

    case A_REL: // relative
        if (bHelp)
            str = " label    |Relative";
        else
            str.Printf(bLabel ? " e%04X" : " $%04X", int( lo & 0x80 ? addr + 1 - (0x100 - lo) : addr + 1 + lo ));
        break;

    case A_ZPGI: // zero page indirect
        str.Printf(bLabel ? " (z%02X)" : " ($%02X)", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indirect";
        break;

    case A_ZPGI_X: // zero page indirect, indexed X
        str.Printf(bLabel ? " (z%02X,X)" : " ($%02X,X)", (int)lo);
        if (bHelp)
            str += "   |Zero Page Indexed X, Indirect";
        break;

    case A_ZPGI_Y: // zero page indirect, indexed Y
        str.Printf(bLabel ? " (z%02X),Y" : " ($%02X),Y", (int)lo);
        if (bHelp)
            str += "   |Zero Page Indirect, Indexed Y";
        break;

    case A_ABSI: // absolute indirect
        str.Printf(bLabel ? " (a%04X)" : " ($%04X)", (int)word);
        if (bHelp)
            str += "   |Absolute Indirect";
        break;

    case A_ABSI_X: // absolute indirect, indexed X
        str.Printf(bLabel ? " (a%04X,X)" : " ($%04X,X)", (int)word);
        if (bHelp)
            str += " |Absolute Indexed X, Indirect";
        break;

    case A_ZPG2: // zero page dla RMB i SMB
    {
        unsigned int bit_no = (cmd >> 4) & 0x07;
        str.Printf(bLabel ? " #%u,z%02X" : " #%u,$%02X", (unsigned int)bit_no, (int)lo);
        if (bHelp)
            str += "   |Memory Bit Manipulation";
        break;
    }

    case A_ZREL: // zero page / relative dla BBS i BBR
    {
        uint8_t hi = arg2; //ctx.mem[addr + 1];
        unsigned int bit_no = (cmd >> 4) & 0x07;
        str.Printf(bLabel ? " #%u,z%02X,e%04X" : " #%u,$%02X,$%04X", bit_no, int(lo), int( hi & 0x80 ? addr + 2 - (0x100 - hi) : addr + 2 + hi ));
        if (bHelp)
            str += "   |Relative Bit Branch";
        break;
    }

    case A_ABSL:
        str.Printf(bLabel ? " a%06X" : " $%06X", big);
        if (bHelp)
            str += "   |Absolute Long";
        break;

    case A_ABSL_X:
        str.Printf(bLabel ? " a%06X,X" : " $%06X,X", big);
        if (bHelp)
            str += " |Absolute long indexed X";
        break;

    case A_ZPIL: // zero page indirect
        str.Printf(bLabel ? " [z%02X]" : " [$%02X]", (int)lo);
        if (bHelp)
            str += "     |Zero Page Indirect Long";
        break;

    case A_ZPIL_Y: // zero page indirect, indexed Y
        str.Printf(bLabel ? " [z%02X],Y" : " [$%02X],Y", (int)lo);
        if (bHelp)
            str += "   |Zero Page Indirect Long, Indexed Y";
        break;

    case A_SR:	// zero page indirect
        str.Printf(bLabel ? " z%02X,S" : " $%02X,S", (int)lo);
        if (bHelp)
            str += "     |Stack Relative";
        break;

    case A_SRI_Y: // zero page indirect, indexed Y
        str.Printf(bLabel ? " (z%02X,S),Y" : " ($%02X,S),Y", (int)lo);
        if (bHelp)
            str += " |Stack Relative Indirect, Indexed Y";
        break;

    case A_RELL: // Relative Long
        if (bHelp)
            str = " label    |Relative Long";
        else
            str.Printf(bLabel ? " e%04X" : " $%04X", int( word & 0x8000 ? addr + 2 - (0x10000 - word) : addr + 2 + word));
        break;

    case A_XYC:
        str.Printf(bLabel ? " b%02X, b%02X" : " $%02X, $%02X", (uint8_t)word & 0xFF, (uint8_t)(word >> 8) & 0xFF);
        if (bHelp)
            str += "   |Block Move - Source bank, Destination bank";
        break;

    case A_IMM2:
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

std::string CDeasm::ArgumentValue(const CContext &ctx, int cmd_addr /*= -1*/)
{
    wxString str("-");
    uint8_t arg;
    uint32_t addr, tmp;
    uint32_t laddr;

    ASSERT((cmd_addr == -1) || ((cmd_addr >= 0) && (cmd_addr <= 0xFFFF))); // Invalid address

    if (cmd_addr == -1)
        cmd_addr = ctx.pc;

    //uint8_t cmd = ctx.mem[cmd_addr];
    uint8_t mode = CodeToMode()[ctx.mem[cmd_addr]];
    cmd_addr = (cmd_addr + 1) & 0xFFFF;

    switch (mode)
    {
    case A_IMP:
    case A_IMP2:
    case A_ACC:
        break;

    case A_IMM:
        return SetValInfo(ctx.mem[cmd_addr]);

    case A_REL:
        arg = ctx.mem[cmd_addr];
        if (arg & 0x80) // Jump back
            str.Printf("PC-$%02X", int(0x100 - arg));
        else // Jump forward
            str.Printf("PC+$%02X", int(arg));
        break;

    case A_ZPGI:
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        addr = ctx.mem[arg];     // Reference address through cells
        addr += uint16_t(ctx.mem[(arg + 1) & 0xFF] ) << 8;
        //      addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.mem[addr]);

    case A_ZPG:
    case A_ZPG2:
        addr = ctx.mem[cmd_addr];
        return SetMemZPGInfo((uint8_t)addr, ctx.mem[addr]);

    case A_ZPG_X:
        addr = (ctx.mem[cmd_addr] + ctx.x) & 0xFF;
        return SetMemZPGInfo((uint8_t)addr, ctx.mem[addr]);

    case A_ZPG_Y:
        addr = (ctx.mem[cmd_addr] + ctx.y) & 0xFF;
        return SetMemZPGInfo((uint8_t)addr, ctx.mem[addr]);

    case A_ABS:
        addr = ctx.mem[cmd_addr]; // Least significant byte of address
        addr += uint16_t(ctx.mem[cmd_addr + 1]) << 8;
        //      addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.mem[addr]);

    case A_ABSI:
        addr = ctx.mem[cmd_addr]; // Least significant byte of address
        addr += uint16_t(ctx.mem[cmd_addr + 1]) << 8;
        //      addr &= ctx.mem_mask;
        tmp = ctx.mem[addr]; // Number at address
        tmp += uint16_t(ctx.mem[addr + 1]) << 8;
        //      tmp &= ctx.mem_mask;
        return SetMemInfo(tmp, ctx.mem[tmp]);

    case A_ABSI_X:
        addr = ctx.mem[cmd_addr] + ctx.x;	// Low byte of address + X offset

        if (wxGetApp().m_global.GetProcType() != ProcessorType::M6502 && 
            (cmd_addr & 0xFF) == 0xFF) // low byte == 0xFF?
        {
            addr += uint16_t(ctx.mem[cmd_addr - 0xFF]) << 8; // 65C02 addressing bug
        }
        else
            addr += uint16_t(ctx.mem[cmd_addr + 1]) << 8;
        //      addr &= ctx.mem_mask;
        tmp = ctx.mem[addr]; // Number at address
        tmp += uint16_t( ctx.mem[addr + 1] ) << 8;
        //      tmp &= ctx.mem_mask;
        return SetMemInfo(tmp, ctx.mem[tmp]);

    case A_ABS_X:
        addr = ctx.mem[cmd_addr] + ctx.x; // The low byte of the address and the X offset
        addr += uint16_t(ctx.mem[cmd_addr + 1]) << 8;
        //      addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.mem[addr]);

    case A_ABS_Y:
        addr = ctx.mem[cmd_addr] + ctx.y; // Low byte of the address and Y offset
        addr += uint16_t(ctx.mem[cmd_addr + 1]) << 8;
        //      addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.mem[addr]);

    case A_ZPGI_X:
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        arg = (arg + ctx.x) & 0xFF;
        addr = ctx.mem[arg]; // Reference address through cells
        addr += uint16_t(ctx.mem[(arg + 1)&0xFF]) << 8;
        //      addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.mem[addr]);

    case A_ZPGI_Y:
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        addr = ctx.mem[arg] + ctx.y; // Reference address by cells and Y shift
        addr += uint16_t(ctx.mem[(arg + 1)&0xFF]) << 8;
        //      addr &= ctx.mem_mask;
        return SetMemInfo(addr, ctx.mem[addr]);

    case A_ZREL:
    {
        std::string tmpStr = SetMemZPGInfo((uint8_t)cmd_addr, ctx.mem[cmd_addr]);
        arg = ctx.mem[cmd_addr + 1];
        if (arg & 0x80) // Jump back
            str.Printf("; PC-$%02X", int(0x100 - arg));
        else // Jump forward
            str.Printf("; PC+$%02X", int(arg));
        return tmpStr + str.ToStdString();
    }

    case A_ABSL:
        laddr = ctx.mem[cmd_addr]; // low byte of the address
        laddr += ctx.mem[cmd_addr + 1] << 8;
        laddr += ctx.mem[cmd_addr + 2] << 16;
        return SetMemInfo(laddr, ctx.mem[laddr]);

    case A_ABSL_X:
        laddr = ctx.mem[cmd_addr] + ctx.x; // low byte of the address
        laddr += ctx.mem[cmd_addr + 1] << 8;
        laddr += ctx.mem[cmd_addr + 2] << 16;
        return SetMemInfo(laddr, ctx.mem[laddr]);

    case A_ZPIL: // zero page indirect
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        laddr = ctx.mem[arg]; // Reference address through cells
        laddr += (ctx.mem[(arg + 1) & 0xFF]) << 8;
        laddr += (ctx.mem[(arg + 2) & 0xFF]) << 16;
        return SetMemInfo(laddr, ctx.mem[laddr]);

    case A_ZPIL_Y: // zero page indirect, indexed Y
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        laddr = ctx.mem[arg] + ctx.y; // Reference address through cells
        laddr += (ctx.mem[(arg + 1) & 0xFF]) << 8;
        laddr += (ctx.mem[(arg + 2) & 0xFF]) << 16;
        return SetMemInfo(laddr, ctx.mem[laddr]);

    case A_SR: // zero page indirect
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        laddr = ctx.mem[arg]; // Reference address through cells
        return SetMemInfo(laddr, ctx.mem[laddr]);

    case A_SRI_Y: // zero page indirect, indexed Y
        arg = ctx.mem[cmd_addr]; // Cell address on zeropage
        laddr = ctx.mem[arg] + ctx.y; // Reference address through cells
        return SetMemInfo(laddr, ctx.mem[laddr]);

    case A_RELL: // Relative Long
        addr = ctx.mem[cmd_addr];
        addr += ctx.mem[cmd_addr+1] << 8;
        if (addr & 0x8000) // Jump back
            str.Printf("PC-$%04X", int(0x10000 - addr));
        else // Jump forward
            str.Printf("PC+$%04X", int(addr));
        break;

    case A_XYC:
        addr = ctx.mem[cmd_addr];
        addr += ctx.mem[cmd_addr + 1] << 8;
        str.Printf("S-%02X, D-%02X", (uint8_t)addr & 0xFF, (uint8_t)(addr >> 8) & 0xFF);
        break;

    case A_IMM2:
        addr = ctx.mem[cmd_addr]; // Low byte of the address
        addr += uint16_t( ctx.mem[cmd_addr + 1] ) << 8;
        str.Printf("%04X", int(addr));
        break;

    case A_ILL:
        str.clear();
        break;

    default:
        ASSERT(FALSE);
        str.clear();
    }

    return str.ToStdString();
}

std::string CDeasm::SetMemInfo(uint32_t addr, uint8_t val) // Description of the memory location
{
    char buf[128];
    std::string bin = Binary(val);
    snprintf(buf, sizeof(buf), "[%06X]: $%02X, %d, '%c', %s", int(addr), int(val), val & 0xFF, val ? (char)val : (char)' ', bin.c_str());
    return std::string(buf);
}

std::string CDeasm::SetMemZPGInfo(uint8_t addr, uint8_t val) // Cell description of page zero of memory
{
    char buf[128];
    std::string bin = Binary(val);
    snprintf(buf, sizeof(buf), "[%02X]: $%02X, %d, '%c', %s", int(addr), int(val), val & 0xFF, val ? (char)val : (char)' ', bin.c_str());
    return std::string(buf);
}

std::string CDeasm::SetValInfo(uint8_t val)	// Value description 'val'
{
    char buf[128];
    std::string bin = Binary(val);
    snprintf(buf, sizeof(buf), "%d, '%c', %s", val & 0xFF, val ? (char)val : (char)' ', bin.c_str());
    return std::string(buf);
}

std::string CDeasm::Binary(uint8_t val)
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

    return std::string(bin);
}

// Finding the address of the instruction preceding a given instruction
int CDeasm::FindPrevAddr(uint32_t &addr, const CContext &ctx, int cnt/*= 1*/)
{
    ASSERT(cnt >= 0);

    if (cnt <= 0)
        return 0;

    if (cnt > 1)
    {
        int len = std::max(10, cnt * 3) + 2;
        uint16_t start = int(addr) - len > 0 ? addr - len : 0;
        start &= ~1; // Even address
        std::vector<uint16_t> addresses;
        uint8_t cmd;
        int ret = 0;
        int i;

        addresses.reserve(len + 4);

        for (i = 0; start < addr; i++)
        {
            addresses[i] = start;
            cmd = ctx.mem[start];
            start += cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];
            start &= ctx.mem_mask;
        }
        if (start == addr)
            ret = 1;
        else
            ret = -1;

        if (i-cnt < 0)
            addr = 0;
        else
            addr = addresses[i - cnt];
        return ret;
    }
    else
    {
        uint16_t start = int(addr) - 10 > 0 ? addr - 10 : 0;
        uint16_t prev = start;
        int ret = 0;
        //ASSERT(addr <= ctx.mem_mask); // Invalid address; too big
        uint8_t cmd;

        while (start < addr)
        {
            prev = start;
            cmd = ctx.mem[start];
            if (cmd == 0) // BRK command?
                start++;  // We only increase by 1, although normally BRK increases by 2
            else
                start += mode_to_len[CodeToMode()[cmd]];

            //start &= ctx.mem_mask;
        }

        cmd = ctx.mem[prev];
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
        else if (static_cast<uint32_t>(prev + (cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]])) == addr)
            ret = 1; // is moved by one line
        else
            ret = -1; // there is a shift, but it affected the change of subsequent orders

        addr = prev;
        return ret;
    }
}

// Finding the address of the command following the 'cnt' command from 'addr'
int CDeasm::FindNextAddr(uint32_t &addr, const CContext &ctx, int cnt/*= 1*/)
{
    //ASSERT(addr <= ctx.mem_mask); // Invalid address; too big

    int ret= 0;
    uint32_t next = addr;

    for (uint32_t address = addr; cnt; cnt--)
    {
        address = next;
        next += ctx.mem[address] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[address]]];
        //ASSERT(next != address);
        // &= ctx.mem_mask;

        if (next > ctx.mem_mask)
            next = address;

        if (next < addr)
            ret = 0; // "scroll" the address
        else
            ret = 1; // next address found
    }

    addr = next;
    return ret;
}

// Check how many lines should the window content be moved to reach from 'addr' to 'dest'
int CDeasm::FindDelta(uint32_t &addr, uint32_t dest, const CContext &ctx, int max_lines)
{
    if (dest == addr)
        return 0;

    if (dest < addr)
    {
        uint32_t start = dest;
        int i;

        for (i = 0; start < addr; i++)
        {
            //start += mode_to_len[CodeToMode()[ctx.mem[start]]];
            start += ctx.mem[start] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[start]]];
            //start &= ctx.mem_mask;

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
            //start += mode_to_len[CodeToMode()[ ctx.mem[start]]];
            start += ctx.mem[start] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[start]]];
            //start &= ctx.mem_mask;

            if (i >= max_lines)
                break;
        }

        i = start == addr ? i : -i;
        addr = dest;
        return i;
    }
}
