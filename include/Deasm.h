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

class CDeasm : public CAsm
{
    static const char mnemonics[];

    std::string SetMemZPGInfo(uint8_t addr, uint8_t val);   // Cell description of page zero of memory
    std::string SetMemInfo(uint32_t addr, uint8_t val);     // Memory cell description
    std::string SetValInfo(uint8_t val);                    // Value description 'val'

public:
    /* constructor */ CDeasm()
    { }

    virtual ~CDeasm()
    { }

    std::string DeasmInstr(const CContext& ctx, DeasmFmt flags, int& ptr);
    std::string DeasmInstr(const CmdInfo& ci, DeasmFmt flags);
    std::string ArgumentValue(const CContext &ctx, int ptr= -1);

    std::string Mnemonic(uint8_t code, ProcessorType procType, bool bUseBrk = false);
    std::string Argument(uint8_t cmd, CodeAdr mode, uint32_t addr, uint8_t arg1, uint8_t arg2, uint8_t arg3, bool bLabel = false, bool bHelp = false);
    std::string Binary(uint8_t val);

    int FindPrevAddr(uint32_t &addr, const CContext &ctx, int cnt = 1);
    int FindNextAddr(uint32_t &addr, const CContext &ctx, int cnt = 1);
    int FindDelta(uint32_t &addr, uint32_t dest, const CContext &ctx, int max_lines);
};
