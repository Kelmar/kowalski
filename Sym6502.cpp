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

#include "stdafx.h" 
#include "resource.h"
//#include "6502.h"
#include "MainFrm.h"
#include "Deasm6502Doc.h"
#include "6502View.h"
#include "6502Doc.h"
#include "Deasm.h"

//-----------------------------------------------------------------------------

bool cpu16 = !!(theApp.m_global.m_bProc6502==2);
bool waiflag = false;

UINT16 CSym6502::io_addr= 0xE000;		// pocz¹tek obszaru we/wy symulatora
bool CSym6502::io_enabled= true;
int CSym6502::bus_width= 16;
static const int SIM_THREAD_PRIORITY= THREAD_PRIORITY_BELOW_NORMAL; // priorytet (oprócz animate)
bool CSym6502::s_bWriteProtectArea= false;
UINT32 CSym6502::s_uProtectFromAddr= 0xc000;
UINT32 CSym6502::s_uProtectToAddr= 0xcfff;
bool extracycle= false;			//% Bug Fix 1.2.12.1 - fix cycle counts, used to signal page crossings
ULONG saveCycles= 0; //% Bug Fix 1.2.12.18 - fix command log display


//-----------------------------------------------------------------------------

UINT8 CContext::get_status_reg() const
{
	ASSERT(negative==false || negative==true);
	ASSERT(overflow==false || overflow==true);
	ASSERT(zero==false || zero==true);
	ASSERT(carry==false || carry==true);
	ASSERT(reserved==false || reserved==true);
	ASSERT(break_bit==false || break_bit==true);
	ASSERT(decimal==false || decimal==true);
	ASSERT(interrupt==false || interrupt==true);

	if ((theApp.m_global.m_bProc6502==2) & !emm)
	  return negative<<N_NEGATIVE | overflow<<N_OVERFLOW | zero<<N_ZERO | carry<<N_CARRY |
		mem16<<N_MEMORY | xy16<<N_INDEX | decimal<<N_DECIMAL | interrupt<<N_INTERRUPT; //% Bug fix 1.2.12.3&10 - S reg status bits wrong
	else
	  return negative<<N_NEGATIVE | overflow<<N_OVERFLOW | zero<<N_ZERO | carry<<N_CARRY |
		true<<N_RESERVED | break_bit<<N_BREAK | decimal<<N_DECIMAL | interrupt<<N_INTERRUPT; //% Bug fix 1.2.12.3&10 - S reg status bits wrong
}


void CContext::set_status_reg_bits(UINT8 reg)
{
	negative	= !!(reg & NEGATIVE);
	overflow	= !!(reg & OVERFLOW);
	zero		= !!(reg & ZERO);
	decimal	= !!(reg & DECIMAL);
	interrupt = !!(reg & INTERRUPT);
    carry 	= !!(reg & CARRY);

	if ((theApp.m_global.m_bProc6502==2) & !emm) {
	  mem16	= !!(reg & MEMORY);
	  xy16  = !!(reg & INDEX);
	} else {
	  reserved	= 1; 
	  break_bit = 1; 
	}
}

//=============================================================================


UINT32 CSym6502::get_argument_address(bool bWrite)  // always returns valid address
{
	UINT16 arg;
	UINT32 addr;
	UINT16 pc = ctx.pc; // save original pc
	UINT8 mode;
    
	if (cpu16) 
	   mode = m_vCodeToMode[ctx.mem[ctx.pc+(ctx.pbr<<16)]];
	 else 
	   mode = m_vCodeToMode[ctx.mem[ctx.pc]];
	
	inc_prog_counter();			

	extracycle = false; 

	switch (mode)
	{
	case A_ZPG:
	case A_ZPG2:
		if (cpu16) { 
			addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir) & 0xffff;	
			if ((ctx.dir & 0xff)!= 0)
				extracycle = true;
		} else
			addr = ctx.mem[ctx.pc];	
	    inc_prog_counter();
		break;

	case A_ZPG_X:
		if (cpu16) {
			if (ctx.emm && ((ctx.dir&0xff)==0))
				addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr<<16)] + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xff);	
			else
				addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xffff;	

			if ((ctx.dir & 0xff)!= 0)
				extracycle = true;
		} else
			addr = (ctx.mem[ctx.pc] + (ctx.x & 0xff))& 0xff;  
		inc_prog_counter();
		break;

	case A_ZPG_Y:
		if (cpu16) { 
			if (ctx.emm && (ctx.dir&0xff)==0) {
				addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr<<16)] + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y)) & 0xff); //**
			} else
				addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y))& 0xffff;
			if ((ctx.dir & 0xff)!= 0)
				extracycle = true;
		} else
			addr = (ctx.mem[ctx.pc] + (ctx.y & 0xff))& 0xff;  
		inc_prog_counter();
		break;

	case A_ZPGI: // 65c02 65816 only
		if (cpu16) { 
			arg = (ctx.mem[ctx.pc + (ctx.pbr<<16)]+ ctx.dir)&0xffff;	
			addr = ctx.mem.GetWordInd(arg)+(ctx.dbr<<16);
			if ((ctx.dir & 0xff)!= 0)
				extracycle = true;
		} else {
			arg = ctx.mem[ctx.pc];	
			addr = ctx.mem.GetWordInd(arg);
		}
		inc_prog_counter();
		break;

	case A_ABS:
		if (cpu16) 
		    addr = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16))+(ctx.dbr<<16);
		else
			addr = ctx.mem.GetWord(ctx.pc);
		inc_prog_counter(2);
		break;

	case A_ABS_X:
		if (cpu16) { 
			addr = (ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16)) + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x )) + (ctx.dbr<<16);
		} else {
			addr = ctx.mem.GetWord(ctx.pc) + (ctx.x & 0xff);
		}
		if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
		inc_prog_counter(2);
		break;

	case A_ABS_Y:
		if (cpu16) {
			addr = (ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16)) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y )) + (ctx.dbr<<16);
		} else {
			addr = ctx.mem.GetWord(ctx.pc) + (ctx.y & 0xff);
		}
		if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; //% bug Fix 1.2.12.1 - fix cycle timing
		inc_prog_counter(2);
		break;

	case A_ZPGI_X: 
		if (cpu16) { 
			if (ctx.emm) {
				if  ((ctx.dir&0xff)==0x00) {
					arg = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr<<16)] + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xff);			
					addr = ctx.mem[arg] + (ctx.dbr<<16);
					arg = ((arg+1)&0xff) + (arg&0xff00);
					addr += (ctx.mem[arg]<<8);
				} else {
					arg = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr<<16)] + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xffff);			
					addr = ctx.mem[arg] + (ctx.dbr<<16);
					arg = ((arg+1)&0xff) + (arg&0xff00);
					addr += (ctx.mem[arg]<<8);
				}
			} else {
				arg = (ctx.mem[ctx.pc + (ctx.pbr<<16)]+ ctx.dir + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xffff;			
				addr = ctx.mem.GetWordInd(arg) + (ctx.dbr<<16);
			}
			if ((ctx.dir & 0xff)!= 0)
				extracycle = true;
		} else {
			arg = ctx.mem[ctx.pc];			
			addr = ctx.mem.GetWordInd((arg + (ctx.x & 0xff))&0xff);
		}
		inc_prog_counter();
		break;

	case A_ZPGI_Y:
		if (cpu16) { 
			arg = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir) & 0xffff;			
			addr = ctx.mem.GetWordInd(arg)+(ctx.dbr<<16) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
			if ((ctx.dir & 0xff)!= 0)
				extracycle = true;
		} else {
			arg = ctx.mem[ctx.pc];			
			addr = ctx.mem.GetWordInd(arg) + (ctx.y & 0xff);
		}
		if ((addr>>8) != (ctx.mem.GetWordInd(arg)>>8)) extracycle = true; 	
		inc_prog_counter();
		break;

	case A_ABSI:						
		if (cpu16 && !ctx.emm)
			addr = ctx.mem.GetWord(ctx.pc + (ctx.pbr<<16));
		else
			addr = ctx.mem.GetWord(ctx.pc);
		if (!theApp.m_global.GetProcType() && (addr & 0xFF) == 0xFF)	
			addr = ctx.mem.GetWord(addr, addr - 0xFF);	
		else
			addr = ctx.mem.GetWord(addr);
		if (cpu16 && !ctx.emm) addr+= (ctx.pbr<<16);

		inc_prog_counter(2);
		break;

	case A_ABSI_X:  
		if (cpu16) { 
			addr = (ctx.mem.GetWord(ctx.pc + (ctx.pbr<<16)) + (ctx.xy16 ?(ctx.x & 0xff) : ctx.x)) & 0xffff;
			addr = ctx.mem.GetWord(addr + (ctx.pbr<<16));
		} else {
			addr = ctx.mem.GetWord(ctx.pc) + (ctx.x & 0xff);
			addr = ctx.mem.GetWord(addr);
		}
		inc_prog_counter(2);
		break;

	case A_ZREL:  // 65816 only 
		addr = ctx.mem.GetWord(ctx.pc);
		inc_prog_counter(2);
		break;

	case A_ABSL: // 65816 only 
		addr = ctx.mem.GetLWord(ctx.pc + (ctx.pbr<<16));
		inc_prog_counter(3);
		break;

	case A_ABSL_X: // 65816 only 
		addr = (ctx.mem.GetLWord(ctx.pc + (ctx.pbr<<16)) + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xffffff;
		inc_prog_counter(3);
		break;

	case A_ZPIL: // 65816 only 
		arg = ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir;	
		addr = ctx.mem.GetLWordInd(arg);
		inc_prog_counter();
		if ((ctx.dir & 0xff)!= 0)
			extracycle = true;
		break;

	case A_ZPIL_Y: // 65816 only 
		arg = ctx.mem[ctx.pc + (ctx.pbr<<16)]+ ctx.dir;			
		addr = ctx.mem.GetLWordInd(arg) + (ctx.dbr<<16) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
		inc_prog_counter();
		if ((ctx.dir & 0xff)!= 0)
			extracycle = true;
		break;

	case A_SR: // 65816 only 
		addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.s) & 0xffff;	// adres na str. 0
		inc_prog_counter();
		break;

	case A_SRI_Y:  // 65816 only
		arg = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.s) & 0xffff;	// adres na str. 0
		addr = ctx.mem.GetWordInd(arg) + (ctx.dbr << 16) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
		inc_prog_counter();
		break;

	case A_INDL: // 65816 only
		arg = ctx.mem.GetWord(ctx.pc + (ctx.pbr<<16));
		addr = ctx.mem.GetLWordInd(arg);
		inc_prog_counter(2);
		break;

	case A_IMP:
	case A_IMP2:
	case A_ACC:
	case A_IMM:
	default:
		ASSERT(false);
		return 0;
	}

	if (bWrite && addr >= s_uProtectFromAddr && addr <= s_uProtectToAddr)
	{
		ctx.pc = pc;	// restore original value
		throw SYM_ILL_WRITE;
	}

	return addr;
}


UINT16 CSym6502::get_argument_value(bool rmask)  // use rmask to set return data size false=byte, true=word
{
	UINT8 arg;
	UINT32 addr;
	UINT16 val;
	UINT8 mode;

	if (cpu16) 
		mode = m_vCodeToMode[ctx.mem[ctx.pc+(ctx.pbr<<16)]];
	else
		mode = m_vCodeToMode[ctx.mem[ctx.pc]];

	inc_prog_counter();			

	extracycle = false; 

	switch (mode)
	{
	case A_IMP:
	case A_ACC:
		return 0;

	case A_IMM:
		if (cpu16 && !ctx.emm) {
			val = ctx.mem[ctx.pc + (ctx.pbr<<16)];
			if (rmask) {
     			inc_prog_counter();
				val += ctx.mem[ctx.pc + (ctx.pbr<<16)]<<8;
			}
		} else	
			val = ctx.mem[ctx.pc];

		inc_prog_counter();

		return val;

	case A_IMP2:
	case A_REL: 
		if (cpu16) 
			arg = ctx.mem[ctx.pc + (ctx.pbr<<16)];
		else	
			arg = ctx.mem[ctx.pc];
		inc_prog_counter();
		return arg;

	case A_ZPGI:
		if (cpu16) { 
			arg = ctx.mem[ctx.pc + (ctx.pbr<<16)];			
			if (!ctx.emm) {
				addr = ctx.mem.GetWordInd((arg + ctx.dir)&0xffff)+(ctx.dbr<<16);
				val = check_io_read(addr) ? io_function() : ctx.mem[addr];
				if (rmask) val+= (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1]) << 8;
				if ((ctx.dir & 0xff)!= 0)
					extracycle = true;
				inc_prog_counter();
				return val;
			} else {
				UINT32 adr1, adr2;
				adr1 = arg + ctx.dir;
				if ((ctx.dir & 0xff)==0)
					adr2 = ((arg + 1)&0xff) + ctx.dir;
				else
					adr2 = arg + 1 + ctx.dir;
				addr = ctx.mem[adr1] + (ctx.dbr<<16);
				addr += ctx.mem[adr2]<<8;

				val = check_io_read(addr) ? io_function() : ctx.mem[addr];
				if (rmask) val+= (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1]) << 8;
				if ((ctx.dir & 0xff)!= 0)
					extracycle = true;
				inc_prog_counter();
				return val;
			}
		} else {
			arg = ctx.mem[ctx.pc];			
			addr = ctx.mem.GetWordInd(arg);
			inc_prog_counter();
			return check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		}

	case A_ZPG:
		if (cpu16) {
			addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)]+ ctx.dir) & 0xffff;
			val = ctx.mem[addr];	
			if (rmask) val += (ctx.mem[addr+1])<<8;
			if ((ctx.dir & 0xff)!= 0)
			  extracycle = true;
		} else {
			val = ctx.mem[ctx.mem[ctx.pc]]&0xff;	
		}
		inc_prog_counter();
		return val;

	case A_ZPG_X: 
		if (cpu16) { 
			if (ctx.emm && ((ctx.dir&0xff)==0))
				addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr<<16)] + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xff);	// adres
			else
				addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir + (ctx.xy16 ? (ctx.x & 0xFF) : ctx.x)) & 0xffff;	// adres
			val = ctx.mem[addr];						
			if (rmask) val += (ctx.mem[addr+1])<<8;
			if ((ctx.dir & 0xff)!= 0)
			  extracycle = true;
		} else {
			addr = (ctx.mem[ctx.pc] + (ctx.x & 0xff)) & 0xFF;	
			val = ctx.mem[addr];						
		}
		inc_prog_counter();
		return val;

	case A_ZPG_Y:
		if (cpu16) { 
			if (ctx.emm && ((ctx.dir&0xff)==0))
				addr = ctx.dir + ((ctx.mem[ctx.pc + (ctx.pbr<<16)] + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xff);	
			else
				addr = (ctx.mem[ctx.pc + (ctx.pbr<<16)] + ctx.dir + (ctx.xy16 ? (ctx.y & 0xFF) : ctx.y)) & 0xffff;	
			val = ctx.mem[addr];		
			if (rmask) val += (ctx.mem[addr+1])<<8;
			if ((ctx.dir & 0xff)!= 0)
			  extracycle = true;
		} else {
			val = ctx.mem[ (ctx.mem[ctx.pc] + (ctx.y &0xff)) & 0xFF ];		
		}
		inc_prog_counter();
		return val;

	case A_ABS:
		if (cpu16) { 
			addr = (ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16))&0xffff)+(ctx.dbr<<16);
			inc_prog_counter(2);
			val = check_io_read(addr) ? io_function() : ctx.mem[addr];
			if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
			return val;
		} else {
			addr = ctx.mem.GetWord(ctx.pc);
		    inc_prog_counter(2);
		    return check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		}

	case A_ABS_X:
		if (cpu16) { 
			addr = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16)) + (ctx.dbr<<16) + (ctx.xy16 ? (ctx.x & 0xff) : ctx.x);
			inc_prog_counter(2);
			val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
			if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
			if (!ctx.xy16) 
				ctx.uCycles++;
			else
				if ((addr>>8) != (ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16))>>8)) extracycle = true; 
			
			return val;
		} else {
			addr = ctx.mem.GetWord(ctx.pc) + (ctx.x & 0xff);
			if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; 
		}
		inc_prog_counter(2);
		return check_io_read(addr) ? io_function() : ctx.mem[addr];

	case A_ABS_Y:
		if (cpu16) { 
			addr = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16)) + (ctx.dbr<<16) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
			inc_prog_counter(2);
			val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
			if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
			if (!ctx.xy16) 
				ctx.uCycles++;
			else
				if ((addr>>8) != (ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16))>>8)) extracycle = true; 
			
			return val;
		} else {
			addr = ctx.mem.GetWord(ctx.pc) + (ctx.y & 0xff);
			if ((addr>>8) != (ctx.mem.GetWord(ctx.pc)>>8)) extracycle = true; 
		}
		inc_prog_counter(2);
		return check_io_read(addr) ? io_function() : ctx.mem[addr]; 

	case A_ZPGI_X: 
		UINT32 adr1, adr2;
		if (cpu16) { 
			arg = ctx.mem[ctx.pc+(ctx.pbr<<16)];	
			if (ctx.emm) {
				if ((ctx.dir&0xff)==0) {
					adr1 = ((arg + (ctx.xy16? (ctx.x & 0xff) : ctx.x))&0xff) + ctx.dir;
					adr2 = ((arg + 1 + (ctx.xy16? (ctx.x & 0xff) : ctx.x))&0xff) + ctx.dir;
					addr = ctx.mem[adr1] + (ctx.dbr<<16);
					addr += ctx.mem[adr2]<<8;
				} else {
					adr1 = ((arg + (ctx.xy16? (ctx.x & 0xff) : ctx.x))&0xffff) + ctx.dir;
					adr2 = (adr1 & 0xffff00) + (((adr1&0xff)+1)&0xff);
					addr = ctx.mem[adr1] + (ctx.dbr<<16);
					addr += ctx.mem[adr2]<<8;
				}
			} else 
				addr = ctx.mem.GetWordInd((arg + ctx.dir + (ctx.xy16? (ctx.x & 0xff) : ctx.x)) & 0xffff)+(ctx.dbr<<16);

			if ((ctx.dir & 0xff)!= 0)
			  extracycle = true;
			inc_prog_counter();
			val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
			if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
			return val;
		} else {
			arg = ctx.mem[ctx.pc];	
			addr = ctx.mem.GetWordInd((arg + (ctx.x & 0xff))&0xff);
			inc_prog_counter();
			return check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		}

	case A_ZPGI_Y:
		if (cpu16) {
			arg = ctx.mem[ctx.pc+(ctx.pbr<<16)];	
			if (!ctx.emm) {
				addr = ((ctx.mem.GetWordInd((arg+ctx.dir)&0xffff) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y))&0xffffff) + (ctx.dbr<<16);
				if ((ctx.dir & 0xff)!= 0)
				  ctx.uCycles++;
				inc_prog_counter();
				if (!ctx.xy16) {
					ctx.uCycles++;				
					if ((addr>>8) != (ctx.mem.GetWordInd(arg+ctx.dir)>>8)) extracycle = true; 
				}
				val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
				if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
				return val;
			} else {
				addr = ((ctx.mem.GetWordInd((arg+ctx.dir)&0xffff) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y))&0xffff) + (ctx.dbr<<16);
				if ((ctx.dir & 0xff)!= 0)
				  extracycle = true;
				inc_prog_counter();
				if (!ctx.xy16) {
					ctx.uCycles++;				
					if ((addr>>8) != (ctx.mem.GetWordInd(arg+ctx.dir)>>8)) extracycle = true; 
				}
				val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
				if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
				return val;
			}
		} else {
			arg = ctx.mem[ctx.pc];	
			addr = ctx.mem.GetWordInd(arg) + (ctx.y & 0xff);
		}
		if ((addr>>8) != (ctx.mem.GetWordInd(arg)>>8)) extracycle = true; 
		inc_prog_counter();
		return check_io_read(addr) ? io_function() : ctx.mem[addr]; 

	case A_ABSL: // 65816 only
		addr = ctx.mem.GetLWord(ctx.pc + (ctx.pbr<<16));
		inc_prog_counter(3);
		val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
		return val;

	case A_ABSL_X: // 65816 only
		addr = (ctx.mem.GetLWord(ctx.pc + (ctx.pbr<<16))+ (ctx.xy16 ? (ctx.x & 0xff) : ctx.x)) & 0xffffff;
		inc_prog_counter(3);
		val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
		return val;

	case A_ZPIL: // 65816 only
		arg = ctx.mem[ctx.pc+(ctx.pbr<<16)];			
		addr = ctx.mem.GetLWordInd((arg+ctx.dir)&0xffff);
		inc_prog_counter();
		if ((ctx.dir & 0xff)!= 0)
		  extracycle = true;
		val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
		return val;

	case A_ZPIL_Y: // 65816 only
		arg = ctx.mem[ctx.pc+(ctx.pbr<<16)];	
		addr = (ctx.mem.GetLWordInd((arg+ctx.dir)&0xffff) + (ctx.xy16 ? (ctx.y & 0xff) : ctx.y)) & 0xffffff;
		inc_prog_counter();
		if ((ctx.dir & 0xff)!= 0)
		  extracycle = true;
		val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
		return val;

	case A_SR: // 65816 only
		addr = (ctx.mem[ctx.pc+(ctx.pbr<<16)]+ctx.s)&0xffff;
		val = ctx.mem[addr];	
        if (rmask) val += (ctx.mem[addr+1])<<8;   
		inc_prog_counter();
		return val;

	case A_SRI_Y: // 65816 only
		arg = ctx.mem[ctx.pc+(ctx.pbr<<16)];	
		addr = (ctx.mem.GetWordInd(((arg+ctx.s)&0xffff)+(ctx.dbr<<16)) + (ctx.xy16 ? (ctx.y&0xff) : ctx.y) + (ctx.dbr<<16)) & 0xffffff;
		inc_prog_counter();
		val = check_io_read(addr) ? io_function() : ctx.mem[addr]; 
		if (rmask) val += (check_io_read(addr+1) ? io_function() : ctx.mem[addr+1])<<8;
		return val;

	case A_RELL:
	case A_ABSI_X:
	case A_ABSI:		
	case A_ZREL:		
	case A_ZPG2:		
	default:
		ASSERT(false);
		return 0;
	}
}


// funkcja wykonuje rozkaz wskazywany przez ctx.pc zmieniaj¹c odpowiednio stan
// rejestrów i pamiêci (ctx.mem)
CAsm::SymStat CSym6502::perform_cmd()
{
	try
	{
		return perform_command();
	}
	catch (SymStat s)
	{
		return s;
	}
	return SYM_OK;
}


CAsm::SymStat CSym6502::perform_command()
{
	UINT8 cmd; 
	UINT16 arg, acc;
	UINT32 addr,acc32;
	UINT8 zero, overflow, carry, negative; 
	UINT8 acc8,arg8;
	#define TOBCD(a) (((((a)/10) % 10) << 4) | ((a) % 10))
	#define TOBIN(a) (((a) >> 4)*10 + ((a) & 0x0F))

	cpu16 = !!(theApp.m_global.m_bProc6502==2);
	theApp.m_global.m_bBank = (cpu16); 
	theApp.m_global.m_bPBR = ctx.pbr;
	if (cpu16 && !ctx.emm && !ctx.mem16)
		ctx.a = (ctx.a & 0xff)+(ctx.b<<8);

	if (m_nInterruptTrigger != NONE)	
	{
		interrupt(m_nInterruptTrigger);
	}
	if (cpu16) 
		cmd= ctx.mem[ctx.pc + (ctx.pbr<<16)];		
	else
		cmd= ctx.mem[ctx.pc];		

	pre = ctx; 
	pre.intFlag =false;
	if (pre.uCycles > saveCycles)
		pre.intFlag = true;
	pre.uCycles = saveCycles;

	switch (m_vCodeToCommand[cmd])
	{

	case C_ADC:
		carry = ctx.carry;
		if (cpu16 && !ctx.mem16 && !ctx.emm ) {
			arg = get_argument_value(true);
			acc = ctx.a;
			if (ctx.decimal) {
				addr = (acc&0x0f) + (arg&0x0f) + (carry ? 1 : 0);
				if ((addr & 0xff)>9) addr+=0x06;
				addr += (acc&0xf0) + (arg&0xf0);
				if ((addr & 0xff0)>0x9F) addr+=0x60;
				addr += (acc&0xf00) + (arg&0xf00);
				if ((addr & 0xff00)>0x9FF) addr+=0x0600;
				addr += (acc&0xf000) + (arg&0xf000);
				if ((addr & 0xff000)>0x9FFF) addr+=0x06000;
				if (addr > 0xffff) carry = true ; else carry = false;
				if ((addr&0xffff) == 0) zero = true ; else zero = false;
				if (addr & 0x8000) negative = true ; else negative = false;
				if (((acc&0x8000)==(arg&0x8000))&& (addr&0x8000)!=(acc&0x8000)) overflow = true ; else overflow = false;
				if (ctx.decimal && (addr&0xff000)>0x19000) overflow= false;
				ctx.a = (addr & 0xff );
				ctx.b = (addr>>8)&0xff;
			} else {
				addr = acc + arg + (carry ? 1 : 0);
				if (addr > 65535) carry = true ; else carry = false;
				if ((addr&0xffff) == 0) zero = true ; else zero = false;
				if (addr & 0x8000) negative = true ; else negative = false;
				if (((acc&0x8000)==(arg&0x8000))&& (acc&0x8000)!=(addr&0x8000)) overflow = true ; else overflow = false;
				ctx.a = addr & 0xffff;
				ctx.b = (addr>>8)&0xff;
			}
			ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);
			arg8 = arg & 0xff;
			acc8 = ctx.a & 0xff;
			if (ctx.decimal) {
				acc = (acc8&0x0f) + (arg8&0x0f) + (carry ? 1 : 0);
				if ((acc & 0xff)>9) acc+=0x06;
				acc +=(acc8&0xf0) + (arg8&0xf0);
				if (((acc & 0xF0)>0x9f) || (acc > 255) ) acc+=0x60;
				if (acc > 255) carry = true ; else carry = false;
				if ((acc&0xff) == 0) zero = true ; else zero = false;
				if (acc & 0x80) negative = true ; else negative = false;
				if (((acc8&0x80)==(arg8&0x80))&& (acc&0x80)!=(acc8&0x80)) overflow = true ; else overflow = false;
				if (ctx.decimal && (acc&0xff0)>0x190) overflow = false;
				ctx.a = (acc & 0xff );
				if ((theApp.m_global.m_bProc6502==1))	//% bug Fix 1.2.12.1 - fix cycle timing
					ctx.uCycles++;
			} else {
				acc = acc8 + arg8 + (carry ? 1 : 0);
				ctx.a = (acc & 0xff );
				if (acc > 255) carry = true ; else carry = false;
				if ((acc&0xff) == 0) zero = true ; else zero = false;
				if (acc & 0x80) negative = true ; else negative = false;
				if (((acc8&0x80)==(arg8&0x80))&& (acc&0x80)!=(acc8&0x80)) overflow = true ; else overflow = false;
			}
			ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_SBC:
		carry = ctx.carry;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = get_argument_value(true);
			acc = ctx.a;
			if (ctx.decimal) {
				arg = ~ arg;
				addr = (acc & 0x0F) + (arg & 0x0F) + (carry ? 1 : 0);
				if(addr <= 0x0F) addr = ((addr - 0x06)& 0x0f)+ (addr & 0xfff0);
				addr = (acc & 0xF0) + (arg & 0xF0) + (addr > 0x0F ? 0x10 : 0) + (addr & 0x0F);
				if(addr <= 0xFF) addr = ((addr - 0x60)& 0xF0)+ (addr & 0xff0f);
				addr = (acc & 0xF00) + (arg & 0xF00) + (addr > 0xFF ? 0x100 : 0) + (addr & 0xFF); 
				if(addr <= 0xFFF) addr = ((addr - 0x0600)& 0x0f00)+ (addr & 0xf0ff);
				addr = (acc & 0xF000) + (arg & 0xF000) + (addr > 0xFFF ? 0x1000 : 0) + (addr & 0xFFF);
				if(~(acc ^ arg) & (acc ^ addr) & 0x8000) overflow=true; else overflow=false;
				if(addr <= 0xFFFF) addr = ((addr - 0x6000)& 0xf000)+ (addr & 0xf0fff);
				if(addr > 0xFFFF) carry=true; else carry = false;
				if ((addr & 0xffff)==0) zero=true; else zero = false;
				if ((addr & 0x8000)!=0) negative = true; else negative = false;
				ctx.a = addr & 0xffff;
				ctx.b = (addr>>8)&0xff;
			} else {
				addr = acc - arg - (carry ? 0 : 1);
				if (addr > 65535) carry = false ; else carry = true;
				if ((addr&0xffff) == 0) zero = true ; else zero = false;
				if (addr & 0x8000) negative = true ; else negative = false;
				if (((acc&0x8000)-(0x8000-(arg&0x8000))==0) && (arg&0x8000)==(addr&0x8000)) overflow = true ; else overflow = false;
				ctx.a = addr & 0xffff;
				ctx.b = (addr>>8)&0xff;
			}
			ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);		
			arg8 = (arg & 0xff);
			acc8 = ctx.a & 0xff;
			if (ctx.decimal) {
				arg8 = ~ arg8;
				acc = (acc8 & 0x0F) + (arg8 & 0x0F) + (carry ? 1 : 0);
				if(acc <= 0x0F) acc -= 0x06;
				acc = (acc8 & 0xF0) + (arg8 & 0xF0) + (acc > 0x0F ? 0x10 : 0) + (acc & 0x0F);
				if(~(acc8 ^ arg8) & (acc8 ^ acc) & 0x80) overflow = true ; else overflow = false;
				if (acc <= 0xff) acc-= 0x60; 
				if (acc > 255) carry = true ; else carry = false;
				if ((acc&0xff) == 0) zero = true ; else zero = false;
				if (acc & 0x80) negative = true ; else negative = false;
				ctx.a = (acc & 0xff );
				if ((theApp.m_global.m_bProc6502==1))	//% bug Fix 1.2.12.1 - fix cycle timing
					ctx.uCycles++;
			} else {
				acc = acc8 - arg8 - (carry ? 0 : 1);
				ctx.a = (acc & 0xff );
				if (acc > 255) carry = false ; else carry = true;
				if ((acc&0xff) == 0) zero = true ; else zero = false;
				if (acc & 0x80) negative = true ; else negative = false;
				if (((acc8&0x80)-(0x80-(arg8&0x80))==0) && ((acc&0x80)==(arg8&0x80))) overflow = true ; else overflow = false;
			}
			ctx.set_status_reg_VZNC(overflow, zero, negative, carry);
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_CMP:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = get_argument_value(true);
			acc = ctx.a;
			addr = acc - arg;
			if (addr > 65535) carry = false ; else carry = true;
			if ((addr&0xffff) == 0) zero = true ; else zero = false;
			if (addr & 0x8000) negative = true ; else negative = false;
			ctx.set_status_reg_ZNC(zero, negative, carry);
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);		
			arg8 = arg & 0xff;
			acc8 = ctx.a & 0xff;
			acc = acc8 - arg8;
			if (acc > 255) carry = false ; else carry = true;
			if ((acc&0xff) == 0) zero = true ; else zero = false;
			if (acc & 0x80) negative = true ; else negative = false;
			ctx.set_status_reg_ZNC(zero, negative, carry);
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_CPX:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			arg = get_argument_value(true);
			acc = ctx.x;
			addr = acc - arg;
			if (addr > 65535) carry = false ; else carry = true;
			if ((addr&0xffff) == 0) zero = true ; else zero = false;
			if (addr & 0x8000) negative = true ; else negative = false;
			ctx.set_status_reg_ZNC(zero, negative, carry);
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);		
			arg8 = arg & 0xff;
			acc8 = ctx.x & 0xff;
				acc = acc8 - arg8;
				if (acc > 255) carry = false ; else carry = true;
				if ((acc&0xff) == 0) zero = true ; else zero = false;
				if (acc & 0x80) negative = true ; else negative = false;
			ctx.set_status_reg_ZNC(zero, negative, carry);
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_CPY:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			arg = get_argument_value(true);
			acc = ctx.y;
			addr = acc - arg;
			if (addr > 65535) carry = false ; else carry = true;
			if ((addr&0xffff) == 0) zero = true ; else zero = false;
			if (addr & 0x8000) negative = true ; else negative = false;
			ctx.set_status_reg_ZNC(zero, negative, carry);
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);		
			arg8 = arg & 0xff;
			acc8 = ctx.y & 0xff;
				acc = acc8 - arg8;
				if (acc > 255) carry = false ; else carry = true;
				if ((acc&0xff) == 0) zero = true ; else zero = false;
				if (acc & 0x80) negative = true ; else negative = false;
			ctx.set_status_reg_ZNC(zero, negative, carry);
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_ASL:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc32 = ctx.a<<1;
				carry = !!(ctx.a & 0x8000);
				zero = !!((acc32&0xffff)==0);
				negative = !!(acc32&0x8000);
				ctx.a = acc32 & 0xffff;
				ctx.b = (acc32>>8)&0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				acc = ctx.mem.GetWord(addr);
				acc32 = acc<<1;
				carry = !!(acc & 0x8000);
				zero = !!((acc32&0xffff)==0);
				negative = !!(acc32&0x8000);
				ctx.mem[addr] = acc32&0xff;
				ctx.mem[addr+1] = (acc32>>8)&0xff;
				ctx.uCycles+=2; // add 2 cycles for 16 bit operations
			}
		} else {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc8 = ctx.a & 0xff;
				acc = acc8<<1;
				carry = !!(acc8 & 0x80);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.a = acc & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg8 = ctx.mem[addr];
				acc = arg8<<1;
				carry = !!(arg8 & 0x80);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.mem[addr] = acc&0xff;
			}
		}
		if (!(theApp.m_global.m_bProc6502==0) && extracycle) ctx.uCycles++; 	
		ctx.set_status_reg_ZNC(zero, negative, carry);
		break;


	case C_LSR:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc = ctx.a>>1;
				carry = !!(ctx.a & 0x01);
				zero = !!(acc==0);
				negative = !!(acc&0x8000);
				ctx.a = acc;
				ctx.b = (acc>>8) & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg = ctx.mem.GetWord(addr);
				acc = arg>>1;
				carry = !!(arg & 0x01);
				zero = !!((acc&0xffff)==0);
				negative = !!(acc&0x8000);
				ctx.mem[addr++] = acc&0xff;
				ctx.mem[addr] = (acc>>8)&0xff;
				ctx.uCycles+=2; // add 2 cycles for 16 bit operations
			}

		} else {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc8 = ctx.a & 0xff;
				acc = acc8>>1;
				carry = !!(acc8 & 0x01);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.a = acc & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg8 = ctx.mem[addr];
				acc = arg8>>1;
				carry = !!(arg8 & 0x01);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.mem[addr] = acc&0xff;
			}
		}
		if (!(theApp.m_global.m_bProc6502==0) && extracycle) ctx.uCycles++; 	
		ctx.set_status_reg_ZNC(zero, negative, carry);
		break;

	case C_ROL:
		carry = ctx.carry;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc = (ctx.a<<1) + (carry ? 1 : 0);
				carry = !!(ctx.a & 0x8000);
				zero = !!(acc==0);
				negative = !!(acc&0x8000);
				ctx.a = acc;
				ctx.b = (acc>>8) & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg = ctx.mem.GetWord(addr);
				acc = (arg<<1) + (carry ? 1 : 0);
				carry = !!(arg & 0x8000);
				zero = !!((acc&0xffff)==0);
				negative = !!(acc&0x8000);
				ctx.mem[addr++] = acc&0xff;
				ctx.mem[addr] = (acc>>8)&0xff;
				ctx.uCycles+=2; // add 2 cycles for 16 bit operations
			}
		} else {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc = ctx.a & 0xff;
				acc = (acc<<1) + (carry ? 1 : 0);
				carry = !!(ctx.a & 0x80);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.a = acc & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg8 = ctx.mem[addr];
				acc = (arg8<<1) + (carry ? 1 : 0);
				carry = !!(acc & 0x100);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.mem[addr] = acc&0xff;
			}
		}
		if (!(theApp.m_global.m_bProc6502==0) && extracycle) ctx.uCycles++; 	
		ctx.set_status_reg_ZNC(zero, negative, carry);
		break;

	case C_ROR:
		carry = ctx.carry;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc = (ctx.a>>1) + (carry ? 0x8000 : 0);;
				carry = !!(ctx.a & 0x01);
				zero = !!(acc==0);
				negative = !!(acc&0x8000);
				ctx.a = acc;
				ctx.b = (acc>>8) & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg = ctx.mem.GetWord(addr);
				acc = (arg>>1) + (carry ? 0x8000 : 0);
				carry = !!(arg & 0x01);
				zero = !!((acc&0xffff)==0);
				negative = !!(acc&0x8000);
				ctx.mem[addr++] = acc&0xff;
				ctx.mem[addr] = (acc>>8)&0xff;
				ctx.uCycles+=2; // add 2 cycles for 16 bit operations
			}
		} else {
			if (m_vCodeToMode[cmd] == A_ACC)	
			{
				inc_prog_counter(); 			
				acc8 = ctx.a & 0xff;
				acc = (acc8>>1) + (carry ? 0x80 : 0);
				carry = !!(acc8 & 0x01);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.a = acc & 0xff;
			}
			else
			{
				addr = get_argument_address(s_bWriteProtectArea);
				arg8 = ctx.mem[addr];
				acc = (arg8>>1) + (carry ? 0x80 : 0);
				carry = !!(arg8 & 0x01);
				zero = !!((acc&0xff)==0);
				negative = !!(acc&0x80);
				ctx.mem[addr] = acc&0xff;
			}
		}
		if (!(theApp.m_global.m_bProc6502==0) && extracycle) ctx.uCycles++; 	
		ctx.set_status_reg_ZNC(zero, negative, carry);
		break;

	case C_AND: 
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = get_argument_value(true);
			ctx.a &= arg;
			ctx.b = (ctx.a>>8)&0xff;
			ctx.set_status_reg16( ctx.a );
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);
			ctx.a &= (arg & 0xff);
			ctx.set_status_reg( ctx.a & 0xff );
		}
		if (extracycle) ctx.uCycles++; 
		break;

	case C_ORA:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = get_argument_value(true);
			ctx.a |= arg;
			ctx.b = (ctx.a>>8)&0xff;
			ctx.set_status_reg16( ctx.a );
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);
			ctx.a = (ctx.a & 0xff)|(arg & 0xff);
			ctx.set_status_reg( ctx.a & 0xff );
		}
		if (extracycle) ctx.uCycles++; 
		break;

	case C_EOR:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = get_argument_value(true);
			ctx.a ^= arg;
			ctx.b = (ctx.a>>8)&0xff;
			ctx.set_status_reg16( ctx.a );
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);
			ctx.a = (ctx.a & 0xff)^(arg & 0xff);
			ctx.set_status_reg( ctx.a & 0xff );
		}
		if (extracycle) ctx.uCycles++; 
		break;

	case C_INC:
		if (m_vCodeToMode[cmd] == A_ACC) {
			inc_prog_counter();
			ctx.a++;
			if (cpu16 && !ctx.emm && !ctx.mem16) {
				ctx.b = (ctx.a>>8)&0xff;
				ctx.set_status_reg16( ctx.a );
			} else
				ctx.set_status_reg( ctx.a & 0xff );
		} else {
			addr = get_argument_address(s_bWriteProtectArea);
			if (cpu16 && !ctx.emm && !ctx.mem16) {
				UINT16 tmp =  ctx.mem.GetWord(addr);
				tmp++;
				ctx.set_status_reg16(tmp);
				ctx.mem[addr] = tmp & 0xff;
				ctx.mem[addr+1] = (tmp>>8) & 0xff;
				ctx.uCycles+=2;   // 16 bit operation adds 2 cycles
			} else {
				ctx.mem[addr]++;
				ctx.set_status_reg( ctx.mem[addr]);
			}
			if (cpu16 && extracycle) ctx.uCycles++; 
		}
		break;

	case C_DEC:
		if (m_vCodeToMode[cmd] == A_ACC) {
			inc_prog_counter();
			ctx.a--;
			if (cpu16 && !ctx.emm && !ctx.mem16) {
				ctx.b = (ctx.a>>8)&0xff;
				ctx.set_status_reg16( ctx.a );
			} else
				ctx.set_status_reg( ctx.a & 0xff );
		} else {
			addr = get_argument_address(s_bWriteProtectArea);
			if (cpu16 && !ctx.emm && !ctx.mem16) {
				UINT16 tmp =  ctx.mem.GetWord(addr);
				tmp--;
				ctx.set_status_reg16(tmp);
				ctx.mem[addr] = tmp & 0xff;
				ctx.mem[addr+1] = (tmp>>8) & 0xff;
				ctx.uCycles+=2;   // 16 bit operation adds 2 cycles
			} else {
				ctx.mem[addr]--;
				ctx.set_status_reg( ctx.mem[addr] );
			}
			if (cpu16 && extracycle) ctx.uCycles++; 
		}
		break;

	case C_INX:
		inc_prog_counter();
		ctx.x++;

		if (cpu16 && !ctx.emm && !ctx.xy16) 
			ctx.set_status_reg16( ctx.x );
		else {
			ctx.set_status_reg( ctx.x & 0xff);
			ctx.x = ctx.x & 0xff;
		}
		break;

	case C_DEX:
		inc_prog_counter();
		ctx.x--;
		if (cpu16 && !ctx.emm && !ctx.xy16) 
			ctx.set_status_reg16( ctx.x );
		else {
			ctx.set_status_reg( ctx.x & 0xff );
			ctx.x = ctx.x & 0xff;
		}
		break;

	case C_INY:
		inc_prog_counter();
		ctx.y++;
		if (cpu16 && !ctx.emm && !ctx.xy16) 
			ctx.set_status_reg16( ctx.y );
		else {
			ctx.set_status_reg( ctx.y & 0xff );
			ctx.y = ctx.y & 0xff;
		}
		break;

	case C_DEY:
		inc_prog_counter();
		ctx.y--;
		if (cpu16 && !ctx.emm && !ctx.xy16) 
			ctx.set_status_reg16( ctx.y );
		else {
			ctx.set_status_reg( ctx.y & 0xff );
			ctx.y = ctx.y & 0xff;
		}
		break;

	case C_TAX:
		inc_prog_counter();
		ctx.x = ctx.a;
		if (cpu16 && !ctx.emm && !ctx.xy16) 
			ctx.set_status_reg16( ctx.x );
		else
			ctx.set_status_reg( ctx.x & 0xff );
		break;

	case C_TXA:
		inc_prog_counter();
		ctx.a = ctx.x;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			ctx.b = (ctx.a>>8)&0xff;
			ctx.set_status_reg16( ctx.a );
		} else
			ctx.set_status_reg( ctx.a & 0xff );
		break;

	case C_TAY:
		inc_prog_counter();
		ctx.y = ctx.a;
		if (cpu16 && !ctx.emm && !ctx.xy16) 
			ctx.set_status_reg16( ctx.y );
		else
			ctx.set_status_reg( ctx.y & 0xff );
		break;

	case C_TYA:
		inc_prog_counter();
		ctx.a = ctx.y;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			ctx.set_status_reg16( ctx.a );
			ctx.b = (ctx.a>>8)&0xff;
		} else
			ctx.set_status_reg( ctx.a & 0xff );
		break;

	case C_TSX: 
		inc_prog_counter();
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			ctx.x = ctx.s;
			ctx.set_status_reg16( ctx.x );
		}else {
			ctx.x = ctx.s & 0xff;
			ctx.set_status_reg( ctx.x & 0xff );
		}
		break;

	case C_TXS:
		inc_prog_counter();
		if (cpu16 && !ctx.emm) { // && !ctx.xy16) {
			ctx.s = ctx.x;
			theApp.m_global.m_bSRef = ctx.s;
		}else {
			ctx.s = ctx.x & 0xff;  
			theApp.m_global.m_bSRef = 0x1ff;
		}

		break;

	case C_STA:
		addr = get_argument_address(s_bWriteProtectArea);
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			if (check_io_write(addr)) {
				io_function(ctx.a & 0xff);
				if (check_io_write(addr+1)) 
					io_function((ctx.a>>8) & 0xff);			
				else						
					ctx.mem[addr+1] = (ctx.a>>8) & 0xff;
			} else {
				ctx.mem[addr] = ctx.a & 0xff;
				if (check_io_write(addr+1)) 
					io_function((ctx.a>>8) & 0xff);			
				else
					ctx.mem[addr+1] = (ctx.a>>8) & 0xff;
			}
			ctx.uCycles++;  // add 1 cycle for 16 bit operation 
		} else {
			if (check_io_write(addr)) 
				io_function(ctx.a & 0xff);
			else
				ctx.mem[addr] = ctx.a & 0xff;
		}
		if (cpu16 && extracycle) ctx.uCycles++; 	
		break;

	case C_STX:
		addr = get_argument_address(s_bWriteProtectArea);
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			if (check_io_write(addr)) {
				io_function(ctx.x & 0xff);
				if (check_io_write(addr+1)) 
					io_function((ctx.x>>8) & 0xff);			
				else						
					ctx.mem[addr+1] = (ctx.x>>8) & 0xff;
			} else {
				ctx.mem[addr] = ctx.x & 0xff;
				if (check_io_write(addr+1)) 
					io_function((ctx.x>>8) & 0xff);			
				else
					ctx.mem[addr+1] = (ctx.x>>8) & 0xff;
			}
			ctx.uCycles++;  // add 1 cycle for 16 bit operation 
		} else {
			if (check_io_write(addr)) 
				io_function(ctx.x & 0xff);
			else
				ctx.mem[addr] = ctx.x & 0xff;
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_STY:
		addr = get_argument_address(s_bWriteProtectArea);
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			if (check_io_write(addr)) {
				io_function(ctx.y & 0xff);
				if (check_io_write(addr+1)) 
					io_function((ctx.y>>8) & 0xff);			
				else						
					ctx.mem[addr+1] = (ctx.y>>8) & 0xff;
			} else {
				ctx.mem[addr] = ctx.y & 0xff;
				if (check_io_write(addr+1)) 
					io_function((ctx.y>>8) & 0xff);			
				else
					ctx.mem[addr+1] = (ctx.y>>8) & 0xff;
			}
			ctx.uCycles++;  // add 1 cycle for 16 bit operation 
		} else {
			if (check_io_write(addr)) 
				io_function(ctx.y & 0xff);
			else
				ctx.mem[addr] = ctx.y & 0xff;
		}
		if (cpu16 && extracycle) ctx.uCycles++; 
		break;

	case C_LDA:   
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			ctx.set_status_reg16( ctx.a = get_argument_value(true) );
			ctx.b = (ctx.a>>8)&0xff;
			ctx.uCycles++;  // add 1 cycle for 16 bit operation 
		} else
			ctx.set_status_reg( (ctx.a  = (get_argument_value(false)&0xff))&0xff);
		if (extracycle) ctx.uCycles++; 
		break;

	case C_LDX:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			ctx.set_status_reg16( ctx.x = get_argument_value(true) );
			ctx.uCycles++;  // add 1 cycle for 16 bit operation 

		} else
			ctx.set_status_reg( (ctx.x = (get_argument_value(false)&0xff))&0xff);
		if (extracycle) ctx.uCycles++; 
		break;

	case C_LDY:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			ctx.set_status_reg16( ctx.y = get_argument_value(true) );
			ctx.uCycles++;  // add 1 cycle for 16 bit operation 
		} else
			ctx.set_status_reg( (ctx.y = (get_argument_value(false)&0xff))&0xff);
		if (extracycle) ctx.uCycles++; 
		break;

	case C_BIT:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = get_argument_value(true);
			ctx.zero = ((ctx.a & arg) == 0);
			ctx.uCycles++;   // 16 bit operation adds 1 cycle
		} else {
			arg = get_argument_value(false);
			ctx.zero = (((ctx.a&0xff) & (arg &0xff)) == 0);
		}
		if (cmd != 0x89) {	// 65C02/816 BIT # only updates Z flag
			if (cpu16 && !ctx.emm && !ctx.mem16) {
				ctx.negative = (arg&0x8000) != 0;
				ctx.overflow = (arg&0x4000) != 0;
			} else {
				ctx.negative = (arg&0x80) != 0;
				ctx.overflow = (arg&0x40) != 0;
			}
		}
		if (extracycle) ctx.uCycles++; 
		break;

	case C_PHA:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			push_addr_on_stack(ctx.a);
			ctx.uCycles++;
		}
		else
			push_on_stack(ctx.a & 0xff);
		inc_prog_counter();
		break;

	case C_PLA:
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			ctx.a = pull_addr_from_stack();
			ctx.b = (ctx.a>>8)&0xff;
			ctx.set_status_reg16(ctx.a);
			ctx.uCycles++;
		} else {
			ctx.a = pull_from_stack();
			ctx.set_status_reg(ctx.a & 0xff);
		}
		inc_prog_counter();
		break;

	case C_PHP:
		inc_prog_counter();		
		if (cpu16 && !ctx.emm) 
			push_on_stack(ctx.get_status_reg());
		else
			push_on_stack( ctx.get_status_reg() | CContext::BREAK | CContext::RESERVED );
		break;

	case C_PLP:
		inc_prog_counter();
		ctx.set_status_reg_bits( pull_from_stack() );
		if (!theApp.m_global.GetProcType())              // not 6502
		{
			ctx.reserved = true;        
			ctx.break_bit = true;
		}
		break;

	case C_JSR:
		addr = get_argument_address(false);
		if (cpu16 && (cmd==0xFC)&& ctx.emm) {
			ctx.emm= false;
			push_addr_on_stack(ctx.pc-1 & 0xFFFF);
			ctx.emm= true;
		} else
			push_addr_on_stack(ctx.pc-1 & 0xFFFF);	
		ctx.pc = addr;
		break;

	case C_JMP:
		addr = get_argument_address(false) & 0xffff;
		ctx.pc = addr;
		break;

	case C_RTS:
		if ((finish == FIN_BY_RTS) && ((ctx.s & 0xff) == 0xFF))                         
			return SYM_FIN;
		ctx.pc = pull_addr_from_stack()+1 & 0xFFFF; 
		break;

	case C_RTI:
		ctx.set_status_reg_bits( pull_from_stack() );
		ctx.pc = pull_addr_from_stack(); 
		if (cpu16 && !ctx.emm){
			ctx.pbr = pull_from_stack();
			theApp.m_global.m_bPBR = ctx.pbr;
			ctx.uCycles++; // native mode takes 1 extra cycle
		}
		break;

	case C_BCC: 
		arg = get_argument_value(false);
		if (!ctx.carry)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BCS:
		arg = get_argument_value(false);
		if (ctx.carry)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BVC:
		arg = get_argument_value(false);
		if (!ctx.overflow)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BVS:
		arg = get_argument_value(false);
		if (ctx.overflow)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BNE:
		arg = get_argument_value(false);
		if (!ctx.zero)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;
	
	case C_BEQ:
		arg = get_argument_value(false);
		if (ctx.zero)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BPL:
		arg = get_argument_value(false);
		if (!ctx.negative)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BMI:
		arg = get_argument_value(false);
		if (ctx.negative)
		{
			AddBranchCycles(arg & 0xff);
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_NOP:
		inc_prog_counter();
		break;

	case C_CLI:
		inc_prog_counter();
		ctx.interrupt = false;
		break;

	case C_SEI:
		inc_prog_counter();
		ctx.interrupt = true;
		break;

	case C_CLD:
		inc_prog_counter();
		ctx.decimal = false;
		break;

	case C_SED:
		inc_prog_counter();
		ctx.decimal = true;
		break;

	case C_CLC:
		inc_prog_counter();
		ctx.carry = false;
		break;

	case C_SEC:
		inc_prog_counter();
		ctx.carry = true;
		break;

	case C_CLV:
		inc_prog_counter();
		ctx.overflow = false;
		break;

	case C_BRK: 
		if (finish == FIN_BY_BRK) 		
			return SYM_FIN;
		inc_prog_counter(2);
		if (cpu16 && !ctx.emm) {
			push_on_stack(ctx.pbr);
			push_addr_on_stack(ctx.pc);
			push_on_stack(ctx.get_status_reg()); 
			ctx.decimal = false; 
			ctx.pc = get_brk_addr16();
			ctx.uCycles++; // native mode takes 1 extra cycle
		} else {
			push_addr_on_stack(ctx.pc);
			ctx.break_bit = true;
			push_on_stack(ctx.get_status_reg() | CContext::RESERVED); 
			if (!(theApp.m_global.m_bProc6502==0)) 
				ctx.decimal = false; 
			ctx.pc = get_irq_addr();
		}
		ctx.interrupt = true; // after pushing status
		ctx.pbr = 0;
		theApp.m_global.m_bPBR = ctx.pbr;
		break;

	//---------- 65c02 --------------------------------------------------------

	case C_PHX:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			push_addr_on_stack(ctx.x);
			ctx.uCycles++;
		}
		else
			push_on_stack(ctx.x & 0xff);
		inc_prog_counter();
		break;

	case C_PLX:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			ctx.x = pull_addr_from_stack();
			ctx.set_status_reg16(ctx.x);
			ctx.uCycles++;
		} else {
			ctx.x = pull_from_stack();
			ctx.set_status_reg(ctx.x & 0xff);
		}
		inc_prog_counter();
		break;	

	case C_PHY:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			push_addr_on_stack(ctx.y);
			ctx.uCycles++;
		}
		else
			push_on_stack(ctx.y & 0xff);
		inc_prog_counter();
		break;

	case C_PLY:
		if (cpu16 && !ctx.emm && !ctx.xy16) {
			ctx.y = pull_addr_from_stack();
			ctx.set_status_reg16(ctx.y);
			ctx.uCycles++;
		} else {
			ctx.y = pull_from_stack();
			ctx.set_status_reg(ctx.y & 0xff);
		}
		inc_prog_counter();
		break;	

	case C_BRA:
		arg = get_argument_value(false);
		AddBranchCycles(arg & 0xff);
		if (arg & 0x80)	
			ctx.pc -= 0x100 - arg;
		else				
			ctx.pc += arg;
		break;

	case C_INA: 
		inc_prog_counter();
		ctx.a++;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			ctx.set_status_reg16( ctx.a );
			ctx.b = (ctx.a>>8)&0xff;
		} else
			ctx.set_status_reg( ctx.a & 0xff );
		break;

	case C_DEA:
		inc_prog_counter();
		ctx.a--;
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			ctx.set_status_reg16( ctx.a );
			ctx.b = (ctx.a>>8)&0xff;
		} else
			ctx.set_status_reg( ctx.a & 0xff );
		break;

	case C_STZ:
		addr = get_argument_address(s_bWriteProtectArea);
		if (check_io_write(addr))
		{
			UINT8 a= 0;
			io_function(a);
		}
		else
			ctx.mem[addr] = 0;

		if (cpu16 && !ctx.emm && !ctx.mem16) {
			addr++;
			if (check_io_write(addr))
			{
				UINT8 a= 0;
				io_function(a);
			}
			else
				ctx.mem[addr] = 0;
		}
		break;

	case C_TRB: 		
		addr = get_argument_address(s_bWriteProtectArea);
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = ctx.mem.GetWord(addr);
			ctx.mem[addr] = (arg & ~ctx.a)&0xff;
			ctx.mem[addr+1] = ((arg & ~ctx.a)>>8)&0xff;
			ctx.zero = (arg & ctx.a) == 0;
			ctx.uCycles+=2;   // 16 bit operation adds 2 cycle
		} else {
			arg = ctx.mem[addr];
			ctx.mem[addr] = (arg&0xff) & ~(ctx.a&0xff);
			ctx.zero = ((arg&0xff) & (ctx.a&0xff)) == 0;
		}
		if (extracycle) ctx.uCycles++; 
		break;

	case C_TSB:		
		addr = get_argument_address(s_bWriteProtectArea);
		if (cpu16 && !ctx.emm && !ctx.mem16) {
			arg = ctx.mem.GetWord(addr);
			ctx.mem[addr] = (arg | ctx.a)&0xff;
			ctx.mem[addr+1] = ((arg | ctx.a)>>8)&0xff;
			ctx.zero = (arg & ctx.a) == 0;
			ctx.uCycles+=2;   // 16 bit operation adds 2 cycle
		} else {
			arg = ctx.mem[addr];
			ctx.mem[addr] = (arg&0xff) | (ctx.a&0xff);
			ctx.zero = ((arg&0xff) & (ctx.a&0xff)) == 0;
		}
		if (extracycle) ctx.uCycles++; 
		break;

	case C_BBR:
		addr = get_argument_address(false);	
		if (!( ctx.mem[addr & 0xFF] & UINT8(1 << ((cmd >> 4) & 0x07)) ))
		{
			arg = addr >> 8;
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			else			
				ctx.pc += arg;
		}
		break;

	case C_BBS:
		addr = get_argument_address(false);	
		if ( ctx.mem[addr & 0xFF] & UINT8(1 << ((cmd >> 4) & 0x07)) )
		{
			arg = addr >> 8;
			if (arg & 0x80) 
				ctx.pc -= 0x100 - arg;
			ctx.pc += arg;
		}
		break;

	case C_RMB:
		addr = get_argument_address(s_bWriteProtectArea);
		ctx.mem[addr] &= UINT8(~(1 << ((cmd >> 4) & 0x07)));
		break;

	case C_SMB:
		addr = get_argument_address(s_bWriteProtectArea);
		ctx.mem[addr] |= UINT8(1 << ((cmd >> 4) & 0x07));
		break;

	//------------65816--------------------------------------------------------
	case C_TXY:
		inc_prog_counter();
		if (cpu16 && !ctx.emm && !ctx.xy16) { 
			ctx.y = ctx.x;
			ctx.set_status_reg16( ctx.y );
		} else {
			ctx.y = ctx.x & 0xff;
			ctx.set_status_reg( ctx.y & 0xff );
		}
		break;

	case C_TYX:
		inc_prog_counter();
        if (cpu16 && !ctx.emm && !ctx.xy16) { 
			ctx.x = ctx.y;
			ctx.set_status_reg16( ctx.x );
		} else {
			ctx.x = ctx.y & 0xff;
			ctx.set_status_reg( ctx.x & 0xff );
		}
		break;
	
	case C_STP:
		inc_prog_counter();
		return SYM_STOP; 

	case C_BRL:
		inc_prog_counter();
		addr = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16));
		inc_prog_counter(2);
		if (addr & 0x8000)	
			ctx.pc = (ctx.pc - (0x10000 - addr))& 0xffff;
		else				
			ctx.pc = (ctx.pc+addr)&0xffff;
		break;

	case C_JSL:
		inc_prog_counter();
		addr = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16));
		inc_prog_counter(2);
		if (ctx.emm && (ctx.s== 0x0100)) {
			ctx.emm=false;
			push_on_stack(ctx.pbr);
			push_addr_on_stack(ctx.pc & 0xFFFF);
			ctx.s = 0x01FD;
			ctx.emm = true;			
		} else {
			push_on_stack(ctx.pbr);
			push_addr_on_stack(ctx.pc & 0xFFFF);
		}
		ctx.pbr = ctx.mem[ctx.pc+(ctx.pbr<<16)];
		theApp.m_global.m_bPBR = ctx.pbr;
		ctx.pc = addr & 0xffff;
		break;

	case C_JML:
		addr = get_argument_address(false);
		ctx.pc = addr & 0xffff;
		ctx.pbr = (addr>>16) & 0xff;
		theApp.m_global.m_bPBR = ctx.pbr;
		break;

	case C_COP:  
		inc_prog_counter(2);
		if (!ctx.emm) {
			push_on_stack(ctx.pbr);
			push_addr_on_stack(ctx.pc);
			push_on_stack(ctx.get_status_reg()); 
			ctx.pc = get_cop_addr16();
			ctx.uCycles++; // native mode takes 1 extra cycle
		} else {
			push_addr_on_stack(ctx.pc);
			push_on_stack(ctx.get_status_reg()); 
			ctx.pc = get_cop_addr();
		}
		ctx.interrupt = true; // after pushing status
		ctx.decimal = false; 
		ctx.pbr = 0;
		break;

	case C_MVN:  {
		ctx.dbr = ctx.mem[ctx.pc+1+(ctx.pbr<<16)];
		 arg = ctx.mem[ctx.pc+2+(ctx.pbr<<16)];
//		ctx.a++;
//		while (ctx.a--){
			// check for write protect error
			UINT32 ptr = (ctx.dbr<<16)+(ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
			if (s_bWriteProtectArea && ptr >= s_uProtectFromAddr && ptr <= s_uProtectToAddr)
				{
					throw SYM_ILL_WRITE;
					break;
				}
			ctx.mem[(ctx.dbr<<16)+(ctx.xy16 ? (ctx.y++ & 0xff) : ctx.y++)] = ctx.mem[(arg<<16)+(ctx.xy16 ? (ctx.x++ & 0xff) : ctx.x++)];
//		}
		if (((--ctx.a) & 0xffff)==0xffff) inc_prog_counter(3);  // repeat opcode until A = 0xffff
		ctx.b = (ctx.a>>8) & 0xff;
		break;
		}
	case C_MVP:  {
		ctx.dbr = ctx.mem[ctx.pc+1+(ctx.pbr<<16)];
		arg = ctx.mem[ctx.pc+2+(ctx.pbr<<16)];
//		ctx.a++;
//		while (ctx.a--){
			// check for write protect error
			UINT32 ptr = (ctx.dbr<<16)+(ctx.xy16 ? (ctx.y & 0xff) : ctx.y);
			if (s_bWriteProtectArea && ptr >= s_uProtectFromAddr && ptr <= s_uProtectToAddr)
				{
					throw SYM_ILL_WRITE;
					break;
				}
			ctx.mem[(ctx.dbr<<16)+(ctx.xy16 ? (ctx.y-- & 0xff) : ctx.y--)] = ctx.mem[(arg<<16)+(ctx.xy16 ? (ctx.x-- & 0xff) : ctx.x--)];
//		}

		if (((--ctx.a) & 0xffff) == 0xffff) inc_prog_counter(3);   // repeat opcode until A = 0xffff
		ctx.b = (ctx.a>>8) & 0xff;
		break;
		}
	case C_PEA:
		inc_prog_counter();
		addr = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16));
		if (ctx.emm && (ctx.s== 0x0100)) {
			ctx.emm=false;
			push_addr_on_stack(addr);
			ctx.s = 0x01FE;
			ctx.emm = true;			
		} else
			push_addr_on_stack(addr);
		inc_prog_counter(2);
		break;

	case C_PEI:  
		inc_prog_counter();
		arg = ctx.mem[ctx.pc+(ctx.pbr<<16)];
		addr = (arg + ctx.dir)&0xffff;
		arg = ctx.mem.GetWord(addr);
		if (ctx.emm && (ctx.s== 0x0100)) {
			ctx.emm=false;
			push_addr_on_stack(arg);
			ctx.s = 0x01FE;
			ctx.emm = true;			
		} else
			push_addr_on_stack(arg);
		if ((ctx.dir & 0xff)!= 0)
			ctx.uCycles++;
		inc_prog_counter();
		break;

	case C_PER:
		inc_prog_counter();
		arg = ctx.mem.GetWord(ctx.pc+(ctx.pbr<<16));
		inc_prog_counter(2);
		if (arg & 0x8000) 
			addr = (ctx.pc - (0x10000-arg)) & 0xffff;
		else
			addr = (ctx.pc + arg)&0xffff;

		if (ctx.emm && (ctx.s== 0x0100)) {
			ctx.emm=false;
			push_addr_on_stack(addr);
			ctx.s = 0x01FE;
			ctx.emm = true;			
		} else
			push_addr_on_stack(addr);
		break;

	case C_PHB:  
		push_on_stack(ctx.dbr);
		inc_prog_counter();
		break;

	case C_PHD:  
		if (ctx.emm && (ctx.s == 0x0100)) {
			ctx.mem[0xff] = ctx.dir & 0xff;
			ctx.mem[0x100] = (ctx.dir>>8)&0xff;
			ctx.s = 0x01fe;
		} else
			push_addr_on_stack(ctx.dir);

		inc_prog_counter();
		break;

	case C_PHK:  
		push_on_stack(ctx.pbr);
		inc_prog_counter();
		break;

	case C_PLB:  
        inc_prog_counter();
		if (ctx.emm && (ctx.s==0x01ff)) {  // special case
			ctx.dbr = ctx.mem[0x200];
			ctx.s = 0x100;
		} else
		  ctx.dbr = pull_from_stack();
		ctx.set_status_reg(ctx.dbr);
		break;

	case C_PLD:  
        inc_prog_counter();
		if (ctx.emm && (ctx.s==0x01ff)) {  // special case
			ctx.dir = ctx.mem.GetWord(0x200);
			ctx.s = 0x0101;
		} else		
			ctx.dir = pull_addr_from_stack();
		ctx.set_status_reg16(ctx.dir);
		break;

	case C_REP:  
        inc_prog_counter();
		arg8 = (ctx.mem[ctx.pc+(ctx.pbr<<16)])^(0xff);    // EOR
		ctx.set_status_reg_bits(ctx.get_status_reg()&arg8);	
        inc_prog_counter();
		break;

	case C_SEP:  
        inc_prog_counter();
		arg8 = ctx.mem[ctx.pc+(ctx.pbr<<16)];
		ctx.set_status_reg_bits(ctx.get_status_reg()|arg8);
		if (ctx.xy16) {
			ctx.x = ctx.x & 0xff;
			ctx.y = ctx.y & 0xff;
		}
        inc_prog_counter();
		break;

	case C_RTL:  
		if (ctx.emm && (ctx.s==0x01ff)) {
			ctx.pc = ctx.mem.GetWord(0x200)+1;
			ctx.pbr = ctx.mem[0x202];
			ctx.s = 0x0102;
		} else {
			ctx.pc = ((pull_addr_from_stack()+1) & 0xFFFF); 
			ctx.pbr = (pull_from_stack() & 0xff);
		}
		theApp.m_global.m_bPBR = ctx.pbr;
		break;

	case C_TCD:  
        inc_prog_counter();
		if (ctx.mem16) 
			ctx.dir = (ctx.a & 0xff) + (ctx.b<<8);
		else
			ctx.dir = ctx.a;
		ctx.set_status_reg16(ctx.dir);
		break;

	case C_TCS:  
		inc_prog_counter();
		if (ctx.mem16) {
			ctx.s = (ctx.a & 0xff) + (ctx.b<<8);
			theApp.m_global.m_bSRef = ctx.s;
		} else {
			ctx.s = ctx.a;
			theApp.m_global.m_bSRef = ctx.s + 0x100;
		}
		break;

	case C_TDC:  
        inc_prog_counter();
		ctx.a = ctx.dir;
		ctx.b = (ctx.a>>8)&0xff;
		ctx.set_status_reg16(ctx.a);
		break;

	case C_TSC:  
        inc_prog_counter();
		ctx.a = ctx.s;
		ctx.b = (ctx.a>>8)&0xff;
		ctx.set_status_reg16(ctx.a);
		break;

	case C_WAI:  
		// don't inc PC so this instruction just repeats until Interrupt or Reset changes it
		// need to modify return addr so after IRQ continues to next instruction
		// this is done in the NMI and IRQ handler routine (see further down)
		waiflag = true;
		break;

	case C_WDM:  // NOP plus sig byte
        inc_prog_counter(2);
		break;

	case C_XBA:  
        inc_prog_counter();
		arg = ctx.b;
		ctx.b = ctx.a & 0xff;
		ctx.a = arg;
		ctx.set_status_reg(ctx.a & 0xff);
		break;
	case C_XCE:  
        inc_prog_counter();
		arg = ctx.emm;
		ctx.emm = ctx.carry;
		ctx.carry = arg;
		if (ctx.emm) {
			ctx.mem16 = true;
			ctx.xy16 = true;
		}
		break;


	//-------------------------------------------------------------------------

	case C_ILL:
		if (finish == FIN_BY_DB && cmd == 0xDB)   // DB is invalid for 6502 and 65C02 - STP for 65816
			return SYM_FIN;
			
		if (!(theApp.m_global.m_bProc6502==0)) //not 6502 mode
		{
			arg = get_argument_value(false);
			extracycle = false;
			break;
		}
		
		return SYM_ILLEGAL_CODE;

	default:
		ASSERT(false);
	}
	
	ctx.uCycles += m_vCodeToCycles[cmd];
  
	CmdInfo ci(pre);
	CmdInfo cj(ctx);
	
	ci.a = cj.a; 
	ci.x = cj.x;
	ci.y = cj.y;
	ci.s = cj.s;
	ci.flags = cj.flags;
	ci.uCycles = cj.uCycles - ci.uCycles;  // this provides cycles used per instruction

	m_Log.Record(ci);
	saveCycles = ctx.uCycles;

	return SYM_OK;
}


CAsm::SymStat CSym6502::skip_cmd()      
{
	inc_prog_counter( mode_to_len[m_vCodeToMode[ctx.mem[ctx.pc]]] );
	return SYM_OK;
}


UINT16 CSym6502::get_irq_addr()
{
	return ( (UINT16)ctx.mem[0xFFFE] | ( (UINT16)ctx.mem[0xFFFF] << 8 ) ); 
}
UINT16 CSym6502::get_irq_addr16()
{
	return ( (UINT16)ctx.mem[0xFFEE] | ( (UINT16)ctx.mem[0xFFEF] << 8 ) ); 
}

UINT16 CSym6502::get_nmi_addr()
{
	return ( (UINT16)ctx.mem[0xFFFA] | ( (UINT16)ctx.mem[0xFFFB] << 8 ) ); 
}
UINT16 CSym6502::get_nmi_addr16()
{
	return ( (UINT16)ctx.mem[0xFFEA] | ( (UINT16)ctx.mem[0xFFEB] << 8 ) ); 
}


UINT16 CSym6502::get_brk_addr16()
{
	return ( (UINT16)ctx.mem[0xFFE6] | ( (UINT16)ctx.mem[0xFFE7] << 8 ) ); 
}

UINT16 CSym6502::get_cop_addr()
{
	return ( (UINT16)ctx.mem[0xFFF4] | ( (UINT16)ctx.mem[0xFFF5] << 8 ) ); 
}
UINT16 CSym6502::get_cop_addr16()
{
	return ( (UINT16)ctx.mem[0xFFE4] | ( (UINT16)ctx.mem[0xFFE5] << 8 ) ); 
}

UINT16 CSym6502::get_abort_addr()
{
	return ( (UINT16)ctx.mem[0xFFF8] | ( (UINT16)ctx.mem[0xFFF9] << 8 ) ); 
}
UINT16 CSym6502::get_abort_addr16()
{
	return ( (UINT16)ctx.mem[0xFFE8] | ( (UINT16)ctx.mem[0xFFE9] << 8 ) ); 
}

UINT16 CSym6502::get_rst_addr()
{
	return ( (UINT16)ctx.mem[0xFFFC] | ( (UINT16)ctx.mem[0xFFFD] << 8 ) ); 
}

//=============================================================================

CAsm::SymStat CSym6502::StepInto()
{
	ASSERT(fin_stat!=SYM_FIN);    

	if (running)
	{
		ASSERT(false);
		return SYM_OK;
	}
	set_translation_tables();
	stop_prog = false;
	running = true;
	old = ctx;            
	fin_stat = perform_cmd();
	running = false;

	Update(fin_stat);

	return fin_stat;
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::StepOver()
{
	ASSERT(fin_stat!=SYM_FIN);    

	if (running)
	{
		ASSERT(false);
		return SYM_OK;
	}
	Update(SYM_RUN);
	old = ctx;            
	stop_prog = false;
	running = true;
	CWinThread *thread = AfxBeginThread(CSym6502::start_step_over_thread, this, SIM_THREAD_PRIORITY, 0, CREATE_SUSPENDED);
	if (thread == NULL)
	{
		running = false;
		AfxMessageBox(IDS_ERR_SYM_THREAD);
	}
	else
	{
		hThread = (HANDLE)*thread;
		ResetPointer();             
		thread->ResumeThread();
	}
	return SYM_OK;
}


UINT CSym6502::start_step_over_thread(LPVOID ptr)
{
	CSym6502 *pSym= (CSym6502 *)ptr;
	pSym->fin_stat = pSym->step_over();
	pSym->running = false;
	AfxGetApp()->GetMainWnd()->PostMessage(WM_USER+9998,pSym->fin_stat,0);        
	return 0;
}


CAsm::SymStat CSym6502::step_over()     
{
	UINT32 addr= ctx.pc;
	if (cpu16) addr += (ctx.pbr<<16);
	bool jsr= false;
	UINT16 stack;

	set_translation_tables();

	switch (m_vCodeToCommand[ctx.mem[addr]])
	{
	case C_JSR:
		stack = ctx.s;
		jsr = true;

	case C_JSL:
		stack = ctx.s;
		jsr = true;

	case C_BRK:
		if (debug && !jsr)
			debug->SetTemporaryExecBreakpoint((addr+2) & ctx.mem_mask);     

		for (;;)
		{
			SymStat stat= perform_step(false);
			if (stat != SYM_OK)
				return stat;

			if (jsr && ctx.s==stack)       
				return SYM_BPT_TEMP;          
		}
		break;

    default:
		return perform_cmd();
	}
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::RunTillRet()
{
	ASSERT(fin_stat!=SYM_FIN);    

	if (running)
	{
		ASSERT(false);
		return SYM_OK;
	}
	Update(SYM_RUN);
	old = ctx;            
	stop_prog = false;
	running = true;
	CWinThread *thread = AfxBeginThread(CSym6502::start_run_till_ret_thread, this, SIM_THREAD_PRIORITY, 0, CREATE_SUSPENDED);
	if (thread == NULL)
	{
		running = false;
		AfxMessageBox(IDS_ERR_SYM_THREAD);
	}
	else
	{
		hThread = (HANDLE)*thread;
		ResetPointer();             
		thread->ResumeThread();
	}
	return SYM_OK;
}


UINT CSym6502::start_run_till_ret_thread(LPVOID ptr)
{
	CSym6502 *pSym= (CSym6502 *)ptr;
	pSym->fin_stat = pSym->run_till_ret();
	pSym->running = false;
	AfxGetApp()->GetMainWnd()->PostMessage(WM_USER+9998, pSym->fin_stat, 0);      
	return 0;
}


CAsm::SymStat CSym6502::run_till_ret()  
{
	set_translation_tables();

	UINT16 stack= ctx.s + 2;
	for (;;)
	{
		SymStat stat= perform_step(false);
		if (stat != SYM_OK)
			return stat;

		if (ctx.s == stack)         
			return SYM_BPT_TEMP;      
	}
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::Run()
{
	ASSERT(fin_stat!=SYM_FIN);    

	if (running)
	{
		ASSERT(false);
		return SYM_OK;
	}
	Update(SYM_RUN);
	old = ctx;            
	stop_prog = false;
	running = true;
	CWinThread *thread = AfxBeginThread(CSym6502::start_run_thread, this, SIM_THREAD_PRIORITY, 0, CREATE_SUSPENDED);
	if (thread == NULL)
	{
		running = false;
		AfxMessageBox(IDS_ERR_SYM_THREAD);
	}
	else
	{
		hThread = (HANDLE)*thread;
		ResetPointer();             
		thread->ResumeThread();
	}
	return SYM_OK;
}


UINT CSym6502::start_run_thread(LPVOID ptr)
{
	CSym6502 *pSym= (CSym6502 *)ptr;
	pSym->fin_stat = pSym->run();
	pSym->running = false;
	AfxGetApp()->GetMainWnd()->PostMessage(WM_USER+9998,pSym->fin_stat,0);        
	return 0;
}


CAsm::SymStat CSym6502::perform_step(bool animate)
{
	if (stop_prog)						// stop executing?
		return SYM_STOP;

	if (m_nInterruptTrigger != NONE)	// interrupt requested?
		interrupt(m_nInterruptTrigger);

	SymStat stat= perform_cmd();
	if (stat != SYM_OK)
		return stat;

	Breakpoint bp;
	UINT32 addr = ctx.pc;;
	if (cpu16) addr += (ctx.pbr<<16);
	if (debug && (bp=debug->GetBreakpoint(addr)) != BPT_NONE)
	{
		if (bp & BPT_EXECUTE)
			return SYM_BPT_EXECUTE;
		if (bp & BPT_TEMP_EXEC)
			return SYM_BPT_TEMP;
	}

	if (animate)
	{
		eventRedraw.ResetEvent(); 
		AfxGetApp()->GetMainWnd()->PostMessage(WM_USER+9998, SYM_RUN, 1); 
		eventRedraw.Lock();       
	}

	return SYM_OK;
}


CAsm::SymStat CSym6502::run(bool animate)
{
	set_translation_tables();

	for (;;)
	{
		SymStat stat= perform_step(animate);
		if (stat != SYM_OK)
			return stat;
	}
}


void CSym6502::interrupt(int& nInterrupt)	// interrupt requested: load pc ***
{
	ASSERT(running);

	if (nInterrupt & RST)
	{
		ctx.interrupt = false;
		if (!(theApp.m_global.m_bProc6502==0))
			ctx.decimal = false;	
		ctx.pc = get_rst_addr();
		nInterrupt = NONE;
		ctx.emm = true;
		ctx.mem16 = true;
		ctx.xy16 = true;
		ctx.dbr = 0x00;
		ctx.pbr = 0x00;
		ctx.dir = 0x0000;
		waiflag= false;
		ctx.uCycles+= 7;	
	}
	else if (nInterrupt & NMI)
	{
		if (waiflag) {
			inc_prog_counter();
			waiflag = false;
		}
		if (cpu16 && !ctx.emm) {
			push_on_stack(ctx.pbr);
			push_addr_on_stack(ctx.pc);
			ctx.pc = get_nmi_addr16();
		}else {
			push_addr_on_stack(ctx.pc);
			ctx.pc = get_nmi_addr();
		}
		ctx.break_bit = false;
		push_on_stack(ctx.get_status_reg());
		if (!(theApp.m_global.m_bProc6502==0))
			ctx.decimal = false;	
		ctx.break_bit = true; 
		nInterrupt &= ~NMI;
		ctx.uCycles+= 7;	
	}
	else if (nInterrupt & IRQ)
	{
		nInterrupt &= ~IRQ;		 
		if (waiflag) {
			inc_prog_counter();
			waiflag = false;
		}
		if (ctx.interrupt)
			return;				
		if (cpu16 && !ctx.emm) {
			push_on_stack(ctx.pbr);
			push_addr_on_stack(ctx.pc);
			ctx.pc = get_irq_addr16();
		}else {
			push_addr_on_stack(ctx.pc);
			ctx.pc = get_irq_addr();
		}
		ctx.break_bit = false;
		push_on_stack(ctx.get_status_reg());
		ctx.break_bit = true; 
		ctx.interrupt = true;
		if (!(theApp.m_global.m_bProc6502==0)) 
			ctx.decimal = false;  
		ctx.uCycles+= 7;	
	}
	else
	{
		ASSERT(false);
	}
}

//-----------------------------------------------------------------------------

CAsm::SymStat CSym6502::Animate()
{
	ASSERT(fin_stat!=SYM_FIN);   

	if (running)
	{
		ASSERT(false);
		return SYM_OK;
	}
	Update(SYM_RUN);
	old = ctx;            
	stop_prog = false;
	running = true;
	CWinThread *thread = AfxBeginThread(CSym6502::start_animate_thread, this, THREAD_PRIORITY_IDLE, 0, CREATE_SUSPENDED);
	if (thread == NULL)
	{
		running = false;
		AfxMessageBox(IDS_ERR_SYM_THREAD);
	}
	else
	{
		hThread = (HANDLE)*thread;
		thread->ResumeThread();
	}
	return SYM_OK;
}


UINT CSym6502::start_animate_thread(LPVOID ptr)
{

	CSym6502 *pSym= (CSym6502 *)ptr;
	pSym->fin_stat = pSym->run(true);
	pSym->running = false;
	AfxGetApp()->GetMainWnd()->PostMessage(WM_USER+9998, pSym->fin_stat, 0);     
	return 0;
}

//-----------------------------------------------------------------------------

void CSym6502::SkipToAddr(UINT16 addr)
{
	ASSERT(fin_stat != SYM_FIN);  

	if (running)
		return;             
	
	ctx.pc = addr; 
	Update(SYM_OK);
}


CAsm::SymStat CSym6502::SkipInstr()
{
	ASSERT(fin_stat!=SYM_FIN);    

	if (running)
		return SYM_OK;      

	fin_stat = skip_cmd();

	Update(fin_stat);

	return fin_stat;
}

//-----------------------------------------------------------------------------

void CSym6502::AbortProg()
{
	stop_prog = true;
	while (running)
	{
		MSG msg; 
		if (!::GetMessage(&msg, NULL, NULL, NULL))
			break;
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	ASSERT(!running);
}

void CSym6502::ExitSym()
{
	ASSERT(running == false);
	ResetPointer();               
	CMainFrame* pMain = (CMainFrame*) AfxGetApp()->m_pMainWnd;
}

//-----------------------------------------------------------------------------

CString CSym6502::GetLastStatMsg()
{
	return GetStatMsg(fin_stat);
}


CString CSym6502::GetStatMsg(SymStat stat)
{
	CString msg;
	switch (stat)
	{
	case SYM_OK:
	case SYM_BPT_TEMP:
		msg.LoadString(IDS_SYM_STAT_OK);
		break;
	case SYM_BPT_EXECUTE:
		msg.LoadString(IDS_SYM_STAT_BPX);
		break;
	case SYM_BPT_READ:
		msg.LoadString(IDS_SYM_STAT_BPR);
		break;
	case SYM_BPT_WRITE:
		msg.LoadString(IDS_SYM_STAT_BPW);
		break;
	case SYM_ILLEGAL_CODE:
		msg.LoadString(IDS_SYM_STAT_ILL);
		break;
	case SYM_STOP:
		msg.LoadString(IDS_SYM_STAT_STOP);
		break;
	case SYM_FIN:
		msg.LoadString(IDS_SYM_STAT_FIN);
		break;
	case SYM_RUN:
		msg.LoadString(IDS_SYM_STAT_RUN);
		break;
	case SYM_INP_WAIT:
		msg.LoadString(IDS_SYM_STAT_INP_WAIT);
		break;
	case SYM_ILL_WRITE:
		msg.LoadString(IDS_SYM_ILL_WRITE);
		break;
	default:
		ASSERT(false);
	}
	return msg;
}

//-----------------------------------------------------------------------------

void CSym6502::Update(SymStat stat, bool no_ok /*=false*/)
{
	CMainFrame* pMain = (CMainFrame*) AfxGetApp()->m_pMainWnd;

	CString reg= GetStatMsg(stat);
	if (debug)
	{
		if (fin_stat == SYM_BPT_TEMP)
			debug->RemoveTemporaryExecBreakpoint();
		CDebugLine dl;
		if (cpu16)
			debug->GetLine(dl,ctx.pc+(ctx.pbr<<16));
		else
			debug->GetLine(dl,ctx.pc);
		if (fin_stat == SYM_FIN)
			ResetPointer();
		else
			if (cpu16) 
				SetPointer(dl.line,ctx.pc+(ctx.pbr<<16));       
			else
				SetPointer(dl.line,ctx.pc);       
	}
	pMain->m_wndRegisterBar.SendMessage(CBroadcast::WM_USER_UPDATE_REG_WND, (WPARAM)&reg, (LPARAM)&ctx);
	if (stat==SYM_OK && !no_ok)
		pMain->m_wndStatusBar.SetPaneText(0, reg);
	if (running)
		eventRedraw.SetEvent();     
}

//-----------------------------------------------------------------------------

void CSym6502::Restart(const COutputMem &mem)
{
	ctx.Reset(mem);
	old = ctx;
	fin_stat = SYM_OK;
	m_Log.Clear();
	saveCycles = 0;
	ctx.set_status_reg_bits(0); 
}


void CSym6502::SymStart(UINT32 org)
{
	ctx.pc = org;
	ctx.s = 0x01FF;
	theApp.m_global.m_bSRef = ctx.s;
	saveCycles = 0;
	ctx.set_status_reg_bits(0); 

	if (debug)
	{
		CDebugLine dl;
		debug->GetLine(dl,org);
		SetPointer(dl.line,org);  
	}
}


void CSym6502::SetPointer(const CLine &line, UINT32 addr) 
{
	POSITION posDoc= theApp.m_pDocDeasmTemplate->GetFirstDocPosition();
	while (posDoc != NULL)        
	{
		CDocument *pDoc= theApp.m_pDocDeasmTemplate->GetNextDoc(posDoc);
		ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDeasm6502Doc)));
		((CDeasm6502Doc*)pDoc)->SetPointer(addr,true);
	}

	CSrc6502View* pView= FindDocView(line.file);  
	if (m_fuidLastView != line.file && ::IsWindow(m_hwndLastView))        
	{
		if (CSrc6502View* pView= dynamic_cast<CSrc6502View*>(CWnd::FromHandlePermanent(m_hwndLastView)))
			SetPointer(pView, -1, false);              
		m_hwndLastView = 0;
	}
	if (!pView && debug)
	{
		if (const TCHAR* path= debug->GetFilePath(line.file))
		{                                           
			C6502App* pApp= static_cast<C6502App*>( AfxGetApp() );
			pApp->m_bDoNotAddToRecentFileList = true;
			CDocument* pDoc= pApp->OpenDocumentFile(path);
			pApp->m_bDoNotAddToRecentFileList = false;
			if (CSrc6502Doc* pSrcDoc= dynamic_cast<CSrc6502Doc*>(pDoc))
			{
				POSITION pos = pSrcDoc->GetFirstViewPosition();
				if (pos != NULL)
					pView = dynamic_cast<CSrc6502View*>(pSrcDoc->GetNextView(pos));
			}
		}
	}
	if (!pView)
	{
		return;             // nie ma okna dokumentu zawieraj¹cego aktualny wiersz
	}

	SetPointer(pView, line.ln, true);  // wymuszenie przesuniêcia zawartoœci okna, jeœli potrzeba
	m_fuidLastView = line.file;
	m_hwndLastView = pView->m_hWnd;
}

void CSym6502::SetPointer(CSrc6502View* pView, int nLine, bool bScroll)
{
	if (pView == 0)
		return;

	CDocument* pDoc= pView->GetDocument();
	POSITION pos= pDoc->GetFirstViewPosition();
	while (pos != NULL)
	{
		if (CSrc6502View* pSrcView= dynamic_cast<CSrc6502View*>(pDoc->GetNextView(pos)))
			pSrcView->SetPointer(nLine, bScroll && pSrcView == pView);
	}
}


void CSym6502::ResetPointer()   // schowanie strza³ki
{
	POSITION posDoc= theApp.m_pDocDeasmTemplate->GetFirstDocPosition();
	while (posDoc != NULL)        // s¹ okna z deasemblera?
	{
		if (CDeasm6502Doc* pDoc= dynamic_cast<CDeasm6502Doc*>( theApp.m_pDocDeasmTemplate->GetNextDoc(posDoc) ))
			pDoc->SetPointer(-1,true);
	}

	if (m_fuidLastView)
	{
		if (CSrc6502View* pView= FindDocView(m_fuidLastView))
			SetPointer(pView, -1, false);    // zmazanie strza³ki
	}
	m_fuidLastView = 0;
}


CSrc6502View *CSym6502::FindDocView(FileUID fuid)
{
	if (debug == NULL)
		return NULL;
	if (CFrameWnd* pFrame= dynamic_cast<CFrameWnd*>(AfxGetMainWnd()))
		if (CFrameWnd* pActive= pFrame->GetActiveFrame())
			if (CSrc6502View* pView= dynamic_cast<CSrc6502View*>(pActive->GetActiveView()))
				if (debug->GetFileUID(pView->GetDocument()->GetPathName()) == fuid)
					return pView;

	return 0;
}


//=============================================================================

CWnd *CSym6502::io_open_window()  // otwarcie okna terminala
{
	AfxGetMainWnd()->SendMessage(WM_COMMAND,ID_VIEW_IO_WINDOW);
	return io_window();
}


CWnd *CSym6502::io_window()     // odszukanie okna terminala
{
	static CString name;
	static bool loaded= false;
	if (!loaded)
	{
		name.LoadString(IDS_IO_WINDOW);
		loaded = true;
	}
	return CWnd::FindWindow(NULL,name);
}


UINT8 CSym6502::io_function()
{
	CWnd *terminal= io_window();
	if (terminal == NULL)
		terminal = io_open_window();
	if (terminal == 0 || !::IsWindow(terminal->m_hWnd))
	{
		io_func = IO_NONE;
		return 0;
	}

	int arg= 0;

	if (io_func == TERMINAL_IN)
	{
		arg = terminal->SendMessage(CIOWindow::CMD_IN);
	}
	else
	{
		if (io_func == TERMINAL_GET_X_POS || io_func == TERMINAL_GET_Y_POS)
			arg = terminal->SendMessage(CIOWindow::CMD_POSITION, io_func == TERMINAL_GET_X_POS ? 0x3 : 0x2);
		else
			arg = 0;
	}

	io_func = IO_NONE;

	if (arg == -1)  // break?
	{
		Break();
		return 0;
	}

	return UINT8(arg);
}

CAsm::SymStat CSym6502::io_function(UINT8 arg)
{
	CWnd *terminal= io_window();
	if (terminal == NULL)
		terminal = io_open_window();

	switch (io_func)
	{
	case TERMINAL_OUT:
		if (terminal)
			terminal->SendMessage(CIOWindow::CMD_PUTC, arg, 0);
		break;

	case TERMINAL_OUT_CHR:
		if (terminal)
			terminal->SendMessage(CIOWindow::CMD_PUTC, arg, 1);
		break;

	case TERMINAL_OUT_HEX:
		if (terminal)
			terminal->SendMessage(CIOWindow::CMD_PUTC, arg, 2);
		break;

	case TERMINAL_CLS:
		if (terminal)
			terminal->SendMessage(CIOWindow::CMD_CLS);
		break;

	case TERMINAL_IN:
		ASSERT(false);
		break;

	case TERMINAL_SET_X_POS:
	case TERMINAL_SET_Y_POS:
		if (terminal)
			terminal->SendMessage(CIOWindow::CMD_POSITION, io_func == TERMINAL_SET_X_POS ? 0x1 : 0x0, arg);
		break;

	default:
		ASSERT(false);            // nierozpoznana funkcja
	}

	io_func = IO_NONE;
	return SYM_OK;
}


void CSym6502::ClearCyclesCounter()
{
	ASSERT(running == false);
	ctx.uCycles = 0;
}


void CSym6502::AddBranchCycles(UINT8 arg)
{
	ctx.uCycles++;       // branch taken

	if ((!cpu16) || (cpu16 && ctx.emm)) {
		if (arg & 0x80)       // skok do ty³u
		{
			if (ctx.pc >> 8 != UINT16(ctx.pc - (0x100 - arg)) >> 8)
				ctx.uCycles++;     // zmiana strony pamiêci -> dodatkowy cykl
		}
		else          // skok do przodu
		{
			if (ctx.pc >> 8 != UINT16(ctx.pc + arg) >> 8)
				ctx.uCycles++;     // zmiana strony pamiêci -> dodatkowy cykl
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

bool CSym6502::check_io_write(UINT32 addr)
{
	if (io_enabled && (addr >= io_addr) && (addr < (io_addr + IO_LAST_FUNC)))
	{
		io_func = IOFunc(addr - io_addr);
		if (io_func == TERMINAL_GET_X_POS)
			io_func = TERMINAL_SET_X_POS;
		else if (io_func == TERMINAL_GET_Y_POS)
			io_func = TERMINAL_SET_Y_POS;
		return true;
	}
	else
		return false;
}

bool CSym6502::check_io_read(UINT32 addr)
{
	if (io_enabled && (addr >= io_addr) && (addr < (io_addr + IO_LAST_FUNC)))
	{
		io_func = IOFunc(addr - io_addr);
		return true;
	}
	else
		return false;
}


///////////////////////////////////////////////////////////////////////////////

CSym6502::SymStat CSym6502::Interrupt(IntType eInt)
{
	m_nInterruptTrigger |= eInt;
	return SYM_OK;

}


///////////////////////////////////////////////////////////////////////////////

void CSym6502::init()
{
	running = false;
	m_fuidLastView = 0;
	finish = FIN_BY_BRK;
	fin_stat = SYM_OK;
	hThread = 0;
	io_func = IO_NONE;
	m_nInterruptTrigger = NONE;
	m_vCodeToCommand = 0;
	m_vCodeToCycles = 0;
	m_vCodeToMode = 0;
	ctx.set_status_reg_bits(0); 
	if (theApp.m_global.m_bProc6502==2)
		ctx.mem_mask = UINT32((1 <<(24)) - 1); 
	else
		ctx.mem_mask = UINT32((1 <<(16)) - 1); 
}


void CSym6502::set_translation_tables()
{
	m_vCodeToCommand = CodeToCommand();
	m_vCodeToCycles = CodeToCycles();
	m_vCodeToMode = CodeToMode();
}


///////////////////////////////////////////////////////////////////////////////

CString CmdInfo::Asm() const
{
	CDeasm deasm;
	CAsm::DeasmFmt fmt= CAsm::DeasmFmt(CAsm::DF_ADDRESS | CAsm::DF_CODE_BYTES| CAsm::DF_USE_BRK);  //% bug Fix 1.2.13.18 - show BRK vs. .DB $00

	CString strLine= deasm.DeasmInstr(*this, fmt);
	const char* pcszLine= strLine;

	CString strBuf;
	// * indicates RST, IRQ, or NMI have occurred

	if (theApp.m_global.m_bProc6502==2)
		if (intFlag)
			strBuf.Format("%-36s A:%04x X:%04x Y:%04x F:%02x S:%04x  Cycles=%u *", pcszLine, int(a), int(x), int(y), int(flags), int(s), uCycles);
		else
			strBuf.Format("%-36s A:%04x X:%04x Y:%04x F:%02x S:%04x  Cycles=%u ", pcszLine, int(a), int(x), int(y), int(flags), int(s), uCycles);
	else
		if (intFlag)
			strBuf.Format("%-36s A:%02x X:%02x Y:%02x F:%02x S:%04x  Cycles=%u *", pcszLine, int(a), int(x), int(y), int(flags), int((s&0xff) + 0x100), uCycles);
		else
			strBuf.Format("%-36s A:%02x X:%02x Y:%02x F:%02x S:%04x  Cycles=%u ", pcszLine, int(a), int(x), int(y), int(flags), int((s&0xff) + 0x100), uCycles);

	return strBuf;
}
