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

extern C6502App theApp;		// dost�p do GetProcType() z CGlobal


CString CDeasm::DeasmInstr(const CmdInfo& ci, DeasmFmt flags)
{
	CString str(_T(' '), 128), fmt(_T(' '), 128);
	str.Empty();

	UINT32 addr= ci.pc;
	UINT8 cmd= ci.cmd;
	UINT16 uLen= cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];
	if (cmd==C_BRK && !CAsm6502::generateBRKExtraByte) uLen = 1;

	if (flags & DF_ADDRESS)
	{
		fmt.Format("%06X  ",(int)addr);
		str += fmt;
	}

	if (flags & DF_CODE_BYTES)
	{
		switch (uLen)
		{
		case 1:
			fmt.Format("%02X           ", int(cmd));
			break;
		case 2:
			fmt.Format("%02X %02X        ", int(cmd), int(ci.arg1));
			break;
		case 3:
			fmt.Format("%02X %02X %02X     ", int(cmd), int(ci.arg1), int(ci.arg2));
			break;
		case 4:
			fmt.Format("%02X %02X %02X %02X  ", int(cmd), int(ci.arg1), int(ci.arg2), int(ci.arg3));
			break;
		default:
			fmt.Empty();
			ASSERT(FALSE);
		}
		str += fmt;
	}

	UINT8 b6502= theApp.m_global.m_bProc6502;
	str += Mnemonic(cmd, b6502, !!(flags & DF_USE_BRK));

	str += Argument(cmd, (CodeAdr)CodeToMode(b6502)[cmd], addr, ci.arg1, ci.arg2, ci.arg3, flags & DF_LABELS, flags & DF_HELP);

	return str;
}


CString CDeasm::DeasmInstr(const CContext& ctx, DeasmFmt flags, INT32& ptr)
{
	ASSERT(ptr == -1 || ptr >= 0 && ptr <= 0xFFFFFF);

	CString str(_T(' '), 128), fmt(_T(' '), 128);
	str.Empty();
	UINT32 addr;
	UINT8 b6502= theApp.m_global.m_bProc6502;

	if ((theApp.m_global.m_bProc6502==2)) {
		addr = (ptr >= 0) ? ptr : (ctx.pc+(ctx.pbr<<16));
	} else {
		addr = (ptr >= 0) ? ptr : ctx.pc;
	}

	UINT8 cmd= ctx.mem[addr];
	UINT16 uLen= cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];
	if (cmd==0 && CAsm6502::generateBRKExtraByte) uLen = 2;

	if (flags & DF_ADDRESS)
	{
		fmt.Format("%06X  ",addr);
		str += fmt;
	}

	if (flags & DF_CODE_BYTES)
	{
		switch (uLen)
		{
		case 1:
			fmt.Format("%02X           ", int(ctx.mem[addr]));
			break;
		case 2:
			fmt.Format("%02X %02X        ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]));
			break;
		case 3:
			fmt.Format("%02X %02X %02X     ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]), int(ctx.mem[addr + 2]));
			break;
		case 4:
			fmt.Format("%02X %02X %02X %02X  ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]), int(ctx.mem[addr + 2]), int(ctx.mem[addr + 3]));
			break;
		default:
			fmt.Empty();
			ASSERT(FALSE);
		}
		str += fmt;
	}

	str += Mnemonic(cmd, b6502, 1); //% Bug fix 1.2.12.2 - allow BRK vs. .DB in disassembly listings

	int mode = CodeToMode(b6502)[cmd];
	if (b6502==2 && !ctx.emm) {
		if (cmd==0xA2 && !ctx.xy16)
			mode = A_IMM2;
		else if (cmd==0xA0 && !ctx.xy16)
			mode = A_IMM2;
		else if (mode==2 && !ctx.mem16)
			mode = A_IMM2;
	}

	str += Argument(cmd, (CodeAdr)mode, addr, ctx.mem[addr+1], ctx.mem[addr+2], ctx.mem[addr+3], flags & DF_LABELS);

	if (flags & DF_BRANCH_INFO)
	{
		bool sign= FALSE;
		switch (CodeToCommand()[cmd])
		{
		case C_BRL:	//65816
		case C_BRA:
			sign = TRUE;
			break;
		case C_BPL:
			if (!ctx.negative)
				sign = TRUE;
			break;
		case C_BMI:
			if (ctx.negative)
				sign = TRUE;
			break;
		case C_BVC:
			if (!ctx.overflow)
				sign = TRUE;
			break;
		case C_BVS:
			if (ctx.overflow)
				sign = TRUE;
			break;
		case C_BCC:
			if (!ctx.carry)
				sign = TRUE;
			break;
		case C_BCS:
			if (ctx.carry)
				sign = TRUE;
			break;
		case C_BNE:
			if (!ctx.zero)
				sign = TRUE;
			break;
		case C_BEQ:
			if (ctx.zero)
				sign = TRUE;
			break;
		case C_BBS:
			{
				UINT8 zpg= ctx.mem[addr + 1];
				int bit_no= (cmd >> 4) & 0x07;
				if (ctx.mem[zpg] & UINT8(1 << bit_no))
					sign = TRUE;
				break;
			}
		case C_BBR:
			{
				UINT8 zpg= ctx.mem[addr + 1];
				int bit_no= (cmd >> 4) & 0x07;
				if (!(ctx.mem[zpg] & UINT8(1 << bit_no)))
					sign = TRUE;
				break;
			}
		}
		if (sign)
			str += " ->";		// oznaczenie aktywnego skoku
	}

    ptr = (addr + uLen) & 0xFFFFFF;	// adr nast. instr.

	return str;
}


CString CDeasm::Mnemonic(UINT8 code, UINT8 bUse6502, bool bUseBrk/*= false*/)
{
	ASSERT(CodeToCommand(bUse6502)[code] <= C_ILL && CodeToCommand(bUse6502)[code] >= 0);
	TCHAR buf[16];

	UINT8 cmd= CodeToCommand(bUse6502)[code];

	if (cmd == C_ILL || (cmd == C_BRK && !bUseBrk))	// kod nielegalnego rozkazu lub BRK
		wsprintf(buf, ".DB $%02X", int(code));
	else
	{
		memcpy(buf, mnemonics + 3 * cmd, 3);
		buf[3] = _T('\0');
	}
	return CString(buf);
}


const TCHAR CDeasm::mnemonics[]=
	"LDALDXLDYSTASTXSTYSTZTAXTXATAYTYATXSTSXADCSBCCMPCPXCPYINCDECINADEAINXDEXINYDEY"
	"ASLLSRROLRORANDORAEORBITTSBTRBJMPJSRBRKBRABPLBMIBVCBVSBCCBCSBNEBEQRTSRTIPHAPLA"
	"PHXPLXPHYPLYPHPPLPCLCSECCLVCLDSEDCLISEINOP"
	"BBRBBSRMBSMB"
	"BRLCOPJMLJSLMVNMVPPEAPEIPERPHBPHDPHKPLBPLDREPRTLSEPSTPTCDTCSTDCTSCTXYTYXWAIWDMXBAXCE"
	"???";


CString CDeasm::Argument(UINT8 cmd, CodeAdr mode, UINT32 addr, UINT8 arg1, UINT8 arg2, UINT8 arg3, bool bLabel, bool bHelp)
{
	CString str;
	addr++;
	UINT8 lo= arg1;
	UINT16 word= arg1 + (arg2 << 8);
	UINT32 big=  arg1 + (arg2 << 8) + (arg3 <<16);

	switch (mode)
	{
	case A_IMP:		// implied
	case A_IMP2:	// implied dla BRK
		if (bHelp)
			str = _T("           |Implied");
		break;
	case A_ACC:		// accumulator
		if (bHelp)
			str = _T("           |Accumulator");
		break;
	case A_ILL:		// warto�� do oznaczania nielegalnych rozkaz�w w symulatorze (ILLEGAL)
		break;
	case A_IMM:		// immediate
		str.Format(" #$%02X",(int)lo);
		if (bHelp)
			str += _T("      |Immediate");
		break;
	case A_ZPG:		// zero page
		str.Format(bLabel ? " z%02X" : " $%02X",(int)lo);
		if (bHelp)
			str += _T("       |Zero Page");
		break;
	case A_ABS:		// absolute
		str.Format(bLabel ? " a%04X" : " $%04X",(int)word);
		if (bHelp)
			str += _T("     |Absolute");
		break;
	case A_ABS_X:	// absolute indexed X
		str.Format(bLabel ? " a%04X,X" : " $%04X,X",(int)word);
		if (bHelp)
			str += _T("   |Absolute Indexed, X");
		break;
	case A_ABS_Y:	// absolute indexed Y
		str.Format(bLabel ? " a%04X,Y" : " $%04X,Y",(int)word);
		if (bHelp)
			str += _T("   |Absolute Indexed, Y");
		break;
	case A_ZPG_X:	// zero page indexed X
		str.Format(bLabel ? " z%02X,X" : " $%02X,X",(int)lo);
		if (bHelp)
			str += _T("     |Zero Page Indexed, X");
		break;
	case A_ZPG_Y:	// zero page indexed Y
		str.Format(bLabel ? " z%02X,Y" : " $%02X,Y",(int)lo);
		if (bHelp)
			str += _T("     |Zero Page Indexed, Y");
		break;
	case A_REL:		// relative
		if (bHelp)
			str = _T(" label    |Relative");
		else
			str.Format(bLabel ? " e%04X" : " $%04X", int( lo & 0x80 ? addr+1 - (0x100 - lo) : addr+1 + lo ));
		break;
	case A_ZPGI:	// zero page indirect
		str.Format(bLabel ? " (z%02X)" : " ($%02X)",(int)lo);
		if (bHelp)
			str += _T("     |Zero Page Indirect");
		break;
	case A_ZPGI_X:	// zero page indirect, indexed X
		str.Format(bLabel ? " (z%02X,X)" : " ($%02X,X)",(int)lo);
		if (bHelp)
			str += _T("   |Zero Page Indexed X, Indirect");
		break;
	case A_ZPGI_Y:	// zero page indirect, indexed Y
		str.Format(bLabel ? " (z%02X),Y" : " ($%02X),Y",(int)lo);
		if (bHelp)
			str += _T("   |Zero Page Indirect, Indexed Y");
		break;
	case A_ABSI:	// absolute indirect
		str.Format(bLabel ? " (a%04X)" : " ($%04X)",(int)word);
		if (bHelp)
			str += _T("   |Absolute Indirect");
		break;
	case A_ABSI_X:	// absolute indirect, indexed X
		str.Format(bLabel ? " (a%04X,X)" : " ($%04X,X)",(int)word);
		if (bHelp)
			str += _T(" |Absolute Indexed X, Indirect");
		break;
	case A_ZPG2:	// zero page dla RMB i SMB
		{
			unsigned int bit_no= (cmd>>4) & 0x07;
			str.Format(bLabel ? " #%u,z%02X" : " #%u,$%02X",(unsigned int)bit_no,(int)lo);
			if (bHelp)
				str += _T("   |Memory Bit Manipulation");
			break;
		}
	case A_ZREL:	// zero page / relative dla BBS i BBR
		{
			UINT8 hi= arg2; //ctx.mem[addr + 1];
			unsigned int bit_no= (cmd >> 4) & 0x07;
			str.Format(bLabel ? " #%u,z%02X,e%04X" : " #%u,$%02X,$%04X", bit_no, int(lo), int( hi & 0x80 ? addr+2 - (0x100 - hi) : addr+2 + hi ));
			if (bHelp)
				str += _T("   |Relative Bit Branch");
			break;
		}
	case A_ABSL:
		str.Format(bLabel ? " a%06X" : " $%06X", big);
		if (bHelp)
			str += _T("   |Absolute Long");
		break;

	case A_ABSL_X:
		str.Format(bLabel ? " a%06X,X" : " $%06X,X", big);
		if (bHelp)
			str += _T(" |Absolute long indexed X");
		break;

	case A_ZPIL:	// zero page indirect
		str.Format(bLabel ? " [z%02X]" : " [$%02X]",(int)lo);
		if (bHelp)
			str += _T("     |Zero Page Indirect Long");
		break;

	case A_ZPIL_Y:	// zero page indirect, indexed Y
		str.Format(bLabel ? " [z%02X],Y" : " [$%02X],Y",(int)lo);
		if (bHelp)
			str += _T("   |Zero Page Indirect Long, Indexed Y");
		break;

	case A_SR:	// zero page indirect
		str.Format(bLabel ? " z%02X,S" : " $%02X,S",(int)lo);
		if (bHelp)
			str += _T("     |Stack Relative");
		break;

	case A_SRI_Y:	// zero page indirect, indexed Y
		str.Format(bLabel ? " (z%02X,S),Y" : " ($%02X,S),Y",(int)lo);
		if (bHelp)
			str += _T(" |Stack Relative Indirect, Indexed Y");
		break;

	case A_RELL:	// Relative Long
		if (bHelp)
			str = _T(" label    |Relative Long");
		else
			str.Format(bLabel ? " e%04X" : " $%04X", int( word & 0x8000 ? addr+2 - (0x10000 - word) : addr+2 + word ));
		break;

	case A_XYC:
		str.Format(bLabel ? " b%02X, b%02X" : " $%02X, $%02X",(UINT8)word & 0xFF, (UINT8)(word >>8) & 0xFF  );
		if (bHelp)
			str += _T("   |Block Move - Source bank, Destination bank");
		break;
	case A_IMM2:
		str.Format(" #$%04X",word);
		if (bHelp)
			str += _T("    |Immediate Long");
		break;

		break;
	default:
		ASSERT(FALSE);
	}

	return str;
}


CString CDeasm::ArgumentValue(const CContext &ctx, UINT32 cmd_addr /*= -1*/)
{
	CString str(_T("-"));
	UINT8 arg;
	UINT32 addr,tmp;
	UINT32 laddr;

	ASSERT(cmd_addr==-1 || cmd_addr>=0 && cmd_addr<=0xFFFF);	// b��dny adres

	if (cmd_addr == -1)
		if (theApp.m_global.m_bBank)
			cmd_addr = ctx.pc+(ctx.pbr<<16);
		else
			cmd_addr = ctx.pc;

    UINT8 cmd= ctx.mem[cmd_addr];
	UINT8 mode= CodeToMode()[cmd];
	cmd_addr = (cmd_addr + 1);

	if (theApp.m_global.m_bBank) {
	// fix for 16 bit immediates
		if ((cmd == 0xA9)&& !ctx.mem16)
			mode = A_IMM2;
		if ((cmd == 0xA2)&& !ctx.xy16)
			mode = A_IMM2;
		if ((cmd == 0xA0)&& !ctx.xy16)
			mode = A_IMM2;
	}
	UINT16 sp = ctx.s;

	switch (mode)
	{
	case A_IMP:
		switch (cmd) {
			case 0x60: // RTS
				if (theApp.m_global.m_bProc6502!=2)  sp = (sp&0xff) + 0x100;
				addr = (ctx.mem[sp+1] + (ctx.mem[sp+2]<<8))+1;
				str.Format(_T("RTS->$%04X"),addr);
				return str;
			case 0x6B: // RTL
				addr = (ctx.mem[ctx.s+1] + (ctx.mem[ctx.s+2]<<8)+ (ctx.mem[ctx.s+3]<<16))+1;
				str.Format(_T("RTL->$%06X"),addr);
				return str;
			case 0xAB: // PLB
				addr = (ctx.mem[ctx.s+1]);
				str.Format(_T("DBR=$%02X"),addr);
				return str;
			case 0x2B: // PLD
				addr = (ctx.mem[ctx.s+1])+(ctx.mem[ctx.s+2]<<8);
				str.Format(_T("DIR=$%04X"),addr);
				return str;
			case 0x68: // PLA
				addr = (ctx.mem[ctx.s+1]);
				if (!ctx.mem16) {
					addr+= (ctx.mem[ctx.s+2]<<8);
					str.Format(_T("A=$%04X"),addr);
				} else
					str.Format(_T("A=$%02X"),addr);
				return str;
			case 0xFA: // PLX
				addr = (ctx.mem[ctx.s+1]);
				if (!ctx.xy16) {
					addr+= (ctx.mem[ctx.s+2]<<8);
					str.Format(_T("X=$%04X"),addr);
				} else
				str.Format(_T("X=$%02X"),addr);
				return str;
			case 0x7A: // PLY
				addr = (ctx.mem[ctx.s+1]);
				if (!ctx.xy16) {
					addr+= (ctx.mem[ctx.s+2]<<8);
					str.Format(_T("Y=$%04X"),addr);
				} else
				str.Format(_T("Y=$%02X"),addr);
				return str;
			case 0x28: // PLP
				addr = (ctx.mem[ctx.s+1]);
				str.Format("P=$%02X", addr);
				return str;
		}
	case A_IMP2:
	case A_ACC:
		break;

	case A_IMM:
		return SetValInfo(ctx.mem[cmd_addr]);

	case A_REL:
		arg = ctx.mem[cmd_addr];
		if (arg & 0x80)	// skok do ty�u
			str.Format(_T("PC-$%02X"),int(0x100 - arg));
		else		// skok do przodu
			str.Format(_T("PC+$%02X"),int(arg));
		return str;

	case A_ZPGI:
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		addr = ctx.mem[arg];		// adres wsk. przez kom�rki
		addr += UINT16( ctx.mem[(arg+1)&0xFF] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZPG:
	case A_ZPG2:
		addr = ctx.mem[cmd_addr];
		return SetMemZPGInfo((UINT8)addr, ctx.mem[addr]);

	case A_ZPG_X:
		addr = (ctx.mem[cmd_addr]+ctx.x) & 0xFF;
		return SetMemZPGInfo((UINT8)addr, ctx.mem[addr]);

	case A_ZPG_Y:
		addr = (ctx.mem[cmd_addr]+ctx.y) & 0xFF;
		return SetMemZPGInfo((UINT8)addr, ctx.mem[addr]);

	case A_ABS:
		addr = ctx.mem[cmd_addr];	// m�odszy bajt adresu
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ABSI:
		addr = ctx.mem[cmd_addr];	// m�odszy bajt adresu
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		tmp = ctx.mem[addr];		// liczba pod adresem
		tmp += UINT16( ctx.mem[addr + 1] ) << 8;
		//      tmp &= ctx.mem_mask;
		return SetMemInfo(tmp, ctx.mem[tmp]);

	case A_ABSI_X:
		addr = ctx.mem[cmd_addr] + ctx.x;	// m�odszy bajt adresu + przesuni�cie X
		if (theApp.m_global.GetProcType() && (cmd_addr & 0xFF)==0xFF)    // m�odszy bajt == 0xFF?
			addr += UINT16( ctx.mem[cmd_addr - 0xFF] ) << 8;  // zgodnie z b��dem w 6502
		else
			addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		tmp = ctx.mem[addr];		// liczba pod adresem
		tmp += UINT16( ctx.mem[addr + 1] ) << 8;
		//      tmp &= ctx.mem_mask;
		return SetMemInfo(tmp, ctx.mem[tmp]);

	case A_ABS_X:
		addr = ctx.mem[cmd_addr] + ctx.x;	// m�odszy bajt adresu i przesuni�cie X
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ABS_Y:
		addr = ctx.mem[cmd_addr] + ctx.y;	// m�odszy bajt adresu i przesuni�cie Y
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZPGI_X:
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		arg = (arg + ctx.x) & 0xFF;
		addr = ctx.mem[arg];		// adres wsk. przez kom�rki
		addr += UINT16( ctx.mem[(arg+1)&0xFF] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZPGI_Y:
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		addr = ctx.mem[arg] + ctx.y;	// adres wsk. przez kom�rki i przesuni�cie Y
		addr += UINT16( ctx.mem[(arg+1)&0xFF] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZREL:
		{
			CString tmp= SetMemZPGInfo((UINT8)cmd_addr, ctx.mem[cmd_addr]);
			arg = ctx.mem[cmd_addr + 1];
			if (arg & 0x80)	// skok do ty�u
				str.Format(_T("; PC-$%02X"),int(0x100 - arg));
			else		// skok do przodu
				str.Format(_T("; PC+$%02X"),int(arg));
			return tmp+str;
		}

	case A_ABSL:
		laddr = ctx.mem[cmd_addr];	// m�odszy bajt adresu
		laddr += ctx.mem[cmd_addr + 1] << 8;
		laddr += ctx.mem[cmd_addr + 2] << 16;
		return SetMemInfo(laddr, ctx.mem[laddr]);

	case A_ABSL_X:
		laddr = ctx.mem[cmd_addr] + ctx.x;	// m�odszy bajt adresu
		laddr += ctx.mem[cmd_addr + 1] << 8;
		laddr += ctx.mem[cmd_addr + 2] << 16;
		return SetMemInfo(laddr, ctx.mem[laddr]);

	case A_ZPIL:	// zero page indirect
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		laddr = ctx.mem[arg];		// adres wsk. przez kom�rki
		laddr +=(ctx.mem[(arg+1)&0xFF]) << 8;
		laddr +=(ctx.mem[(arg+2)&0xFF]) << 16;
		return SetMemInfo(laddr, ctx.mem[laddr]);

	case A_ZPIL_Y:	// zero page indirect, indexed Y
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		laddr = ctx.mem[arg] + ctx.y;		// adres wsk. przez kom�rki
		laddr +=(ctx.mem[(arg+1)&0xFF]) << 8;
		laddr +=(ctx.mem[(arg+2)&0xFF]) << 16;
		return SetMemInfo(laddr, ctx.mem[laddr]);

	case A_SR:	// zero page indirect
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		laddr = ctx.mem[arg];		// adres wsk. przez kom�rki
		return SetMemInfo(laddr, ctx.mem[laddr]);

	case A_SRI_Y:	// zero page indirect, indexed Y
		arg = ctx.mem[cmd_addr];	// adres kom�rki na str. 0
		laddr = ctx.mem[arg] + ctx.y;		// adres wsk. przez kom�rki
		return SetMemInfo(laddr, ctx.mem[laddr]);

	case A_RELL:	// Relative Long
		addr = ctx.mem[cmd_addr];
		addr += ctx.mem[cmd_addr+1] << 8;
		if (addr & 0x8000)	// skok do ty�u
			str.Format(_T("PC-$%04X"),int(0x10000 - addr));
		else		// skok do przodu
			str.Format(_T("PC+$%04X"),int(addr));
		return str;

	case A_XYC:
		addr = ctx.mem[cmd_addr];
		addr += ctx.mem[cmd_addr+1] << 8;
		str.Format(_T("S-%02X, D-%02X"),(UINT8)addr & 0xFF, (UINT8)(addr >>8) & 0xFF);
		return str;

	case A_IMM2:
		addr = ctx.mem[cmd_addr];	// m�odszy bajt adresu
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		return SetWordInfo(addr);
//		str.Format(_T("%04X"),int(addr));
//		return str;

	case A_ILL:
		str.Empty();
		break;

	default:
		ASSERT(FALSE);
		str.Format(_T("MISSING MODE=%D"),mode);
		//str.Empty();
	}

	return str;
}


CString CDeasm::SetMemInfo(UINT32 addr, UINT8 val)	// opis kom�rki pami�ci
{
  CString str;
  str.Format("[%06X]: $%02X, %d, '%c', %s", int(addr), int(val), val&0xFF, val ? (char)val : (char)' ', (const TCHAR *)Binary(val));
  return str;
}


CString CDeasm::SetMemZPGInfo(UINT8 addr, UINT8 val)	// opis kom�rki strony zerowej pami�ci
{
  CString str;
  str.Format("[%02X]: $%02X, %d, '%c', %s", int(addr), int(val), val&0xFF, val ? (char)val : (char)' ', (const TCHAR *)Binary(val));
  return str;
}


CString CDeasm::SetValInfo(UINT8 val)	// opis warto�ci 'val'
{
  CString str;
  str.Format("%d, '%c', %s", val&0xFF, val ? (char)val : (char)' ', (const TCHAR *)Binary(val));
  return str;
}
CString CDeasm::SetWordInfo(UINT16 word)	// opis warto�ci 'val'
{
  CString str;
  str.Format("%d, '%c', %s", word&0xFFFF, (word&0xff) ? (char)(word&0xff) : (char)' ', (const TCHAR *)Binary2(word));
  return str;
}

CString CDeasm::Binary(UINT8 val)
{
  CString bin(_T(' '),8);

  bin.SetAt(0, val & 0x80 ? _T('1') : _T('0') );
  bin.SetAt(1, val & 0x40 ? _T('1') : _T('0') );
  bin.SetAt(2, val & 0x20 ? _T('1') : _T('0') );
  bin.SetAt(3, val & 0x10 ? _T('1') : _T('0') );
  bin.SetAt(4, val & 0x08 ? _T('1') : _T('0') );
  bin.SetAt(5, val & 0x04 ? _T('1') : _T('0') );
  bin.SetAt(6, val & 0x02 ? _T('1') : _T('0') );
  bin.SetAt(7, val & 0x01 ? _T('1') : _T('0') );

  return bin;
}
CString CDeasm::Binary2(UINT16 val)
{
  CString bin(_T(' '),16);

  bin.SetAt(0, val & 0x8000 ? _T('1') : _T('0') );
  bin.SetAt(1, val & 0x4000 ? _T('1') : _T('0') );
  bin.SetAt(2, val & 0x2000 ? _T('1') : _T('0') );
  bin.SetAt(3, val & 0x1000 ? _T('1') : _T('0') );
  bin.SetAt(4, val & 0x0800 ? _T('1') : _T('0') );
  bin.SetAt(5, val & 0x0400 ? _T('1') : _T('0') );
  bin.SetAt(6, val & 0x0200 ? _T('1') : _T('0') );
  bin.SetAt(7, val & 0x0100 ? _T('1') : _T('0') );
  bin.SetAt(8, val & 0x80 ? _T('1') : _T('0') );
  bin.SetAt(9, val & 0x40 ? _T('1') : _T('0') );
  bin.SetAt(10, val & 0x20 ? _T('1') : _T('0') );
  bin.SetAt(11, val & 0x10 ? _T('1') : _T('0') );
  bin.SetAt(12, val & 0x08 ? _T('1') : _T('0') );
  bin.SetAt(13, val & 0x04 ? _T('1') : _T('0') );
  bin.SetAt(14, val & 0x02 ? _T('1') : _T('0') );
  bin.SetAt(15, val & 0x01 ? _T('1') : _T('0') );

  return bin;
}

// odszukanie adresu rozkazu poprzedzaj�cego dany rozkaz
int CDeasm::FindPrevAddr(UINT32 &addr, const CContext &ctx, int cnt/*= 1*/)
{
  ASSERT(cnt >= 0);
  if (cnt <= 0)
    return 0;

  if (cnt > 1)
  {
    int len= max(10, cnt * 3) + 2;
    UINT32 start= int(addr) - len > 0 ? addr - len : 0;
    start &= ~1;	// parzysty adres
    CWordArray addresses;
    addresses.SetSize(len + 4);
    UINT8 cmd;
    int ret= 0;
    int i;

    for (i = 0; start < addr; i++)
    {
      addresses[i] = start;
      cmd = ctx.mem[start];
      start += cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];

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
    UINT32 start= int(addr)-10 > 0 ? addr-10 : 0;
    UINT32 prev= start;
    int ret= 0;
//    ASSERT(addr <= ctx.mem_mask);		// niepoprawny adres; za du�y
    UINT8 cmd;

    while (start < addr)
    {
      prev = start;
      cmd = ctx.mem[start];
      if (cmd == 0)		// rozkaz BRK?
	start++;		// zwi�kszamy tylko o 1, chocia� normalnie BRK zwi�ksza o 2
      else
	start += mode_to_len[CodeToMode()[cmd]];

//      start &= ctx.mem_mask;
    }
    cmd = ctx.mem[prev];
/*
    if (cmd == 0 && ctx.mem[prev - 1] == 0)	// dwa zera przed rozkazem?
    {
      prev--;				// cofamy adres o 2
      ret = 1;				// bo jest to jeden rozkaz BRK
    }
    else
*/
    if (prev == addr)
      ret = 0;	// jeste�my na pocz�tku - nie ma przesuni�cia
    else if ((prev + (cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]])) == addr)
      ret = 1;	// jest przesuni�cie o jeden wiersz
    else
      ret = -1;	// jest przesuni�cie, ale wp�yn�o na zmian� kolejnych rozkaz�w

    addr = prev;
    return ret;
  }
}


// odszukanie adresu rozkazu nast�puj�cego po 'cnt'-ym rozkazie od 'addr'
int CDeasm::FindNextAddr(UINT32 &addr, const CContext &ctx, int cnt/*= 1*/)
{
//  ASSERT(addr <= ctx.mem_mask);		// niepoprawny adres; za du�y

  int ret= 0;
  UINT32 next= addr;

  for (UINT32 address= addr; cnt; cnt--)
  {
    address = next;
    next += ctx.mem[address] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[address]]];
    if (next < addr)
      ret = 0;	// "przewini�cie si�" adresu
	else
      ret = 1;	// nast�pny adres znaleziony
	if  (next > ctx.mem_mask) next = address;
  }

  addr = next;
  return ret;
}


// spr. o ile wierszy nale�y przesun�� zawarto�� okna aby dotrze� od 'addr' do 'dest'
int CDeasm::FindDelta(UINT32 &addr, UINT32 dest, const CContext &ctx, int max_lines)
{
  if (dest == addr)
    return 0;

  if (dest < addr)
  {
    UINT32 start= dest;
    int i;
    for (i=0; start < addr; i++)
    {
      start += ctx.mem[start] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[start]]];
      start &= ctx.mem_mask;
      if (i >= max_lines)
	    break;
    }
    i = start == addr ? i : -i;
    addr = dest;
    return i;
  }
  else
  {
    UINT32 start= addr;
    int i;
    for (i=0; start < dest; i++)
    {
      start += ctx.mem[start] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[start]]];
      start &= ctx.mem_mask;
      if (i >= max_lines)
	break;
    }
    i = start == addr ? i : -i;
    addr = dest;
    return i;
  }
}
