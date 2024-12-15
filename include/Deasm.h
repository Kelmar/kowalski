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

class CDeasm
{
private:
    static const char mnemonics[];

    std::string SetMemZPGInfo(uint8_t addr, uint8_t val);   // Cell description of page zero of memory
    std::string SetMemInfo(uint32_t addr, uint8_t val);     // Memory cell description
    std::string SetValInfo(uint8_t val);                    // Value description 'val'
    std::string SetWordInfo(uint16_t val);                  // Value description 'val'

    std::shared_ptr<CSym6502> m_sim;

public:
    /* constructor */ CDeasm(std::shared_ptr<CSym6502> sim)
        : m_sim(sim)
    {
    }

    virtual ~CDeasm()
    {
    }

    std::string DeasmInstr(CAsm::DeasmFmt flags, int32_t &ptr);
    std::string DeasmInstr(const CmdInfo &ci, CAsm::DeasmFmt flags);
    std::string ArgumentValue(uint32_t ptr = CAsm::INVALID_ADDRESS);

    std::string Mnemonic(uint8_t code, ProcessorType procType, bool bUseBrk = false) const;
    std::string Argument(uint8_t cmd, CAsm::CodeAdr mode, uint32_t addr, uint8_t arg1, uint8_t arg2, uint8_t arg3, bool bLabel = false, bool bHelp = false) const;

    int FindPrevAddr(uint32_t &addr, int cnt = 1);
    int FindNextAddr(uint32_t &addr, int cnt = 1);
    int FindDelta(uint32_t &addr, uint32_t dest, int max_lines);
};
