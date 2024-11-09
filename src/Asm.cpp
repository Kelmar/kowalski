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
#include "Asm.h"
#include "6502.h" // TODO: Remove this reference.

//-----------------------------------------------------------------------------

const char CAsm::LOCAL_LABEL_CHAR = '.'; // Character that starts the local label
const char CAsm::MULTIPARAM[] = "...";   // Ellipsis -any number of parameters

static const uint8_t NA = 0x42; // Invalid in 6502 and 65C02 and WDM in 65816

//-----------------------------------------------------------------------------

// Converting an instruction in a given addressing mode into a code (65XX)
const uint8_t CAsm::trans_table[C_ILL][A_NO_OF_MODES] =
{
// IMP   ACC   IMM   ZPG   ABS  ABS_X ABS_Y ZPG_X ZPG_Y  REL  ZPGI ZPGI_X ZPGI_Y  ABSI ABSI_X ZREL ZPG2 ABSL ABSL_X ZPIL  ZPIL_Y INDL   SR  SRI_Y A_RELL A_XYC A_IMM2
    NA,   NA, 0xA9, 0xA5, 0xAD, 0xBD, 0xB9, 0xB5, NA, NA, NA, 0xA1,  0xB1,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                               // C_LDA
    NA, NA, 0xA2, 0xA6, 0xAE, NA, 0xBE, NA, 0xB6, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                      // C_LDX
    NA, NA, 0xA0, 0xA4, 0xAC, 0xBC, NA, 0xB4, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                      // C_LDY
    NA, NA, NA, 0x85, 0x8D, 0x9D, 0x99, 0x95, NA, NA, NA, 0x81,  0x91,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                  // C_STA
    NA, NA, NA, 0x86, 0x8E, NA, NA, NA, 0x96, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                          // C_STX
    NA, NA, NA, 0x84, 0x8C, NA, NA, 0x94, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                          // C_STY
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_STZ
    0xAA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_TAX
    0x8A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_TXA
    0xA8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_TAY
    0x98, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_TYA
    0x9A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_TXS
    0xBA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_TSX
    NA, NA, 0x69, 0x65, 0x6D, 0x7D, 0x79, 0x75, NA, NA, NA, 0x61,  0x71,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                // C_ADC
    NA, NA, 0xE9, 0xE5, 0xED, 0xFD, 0xF9, 0xF5, NA, NA, NA, 0xE1,  0xF1,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                // C_SBC
    NA, NA, 0xC9, 0xC5, 0xCD, 0xDD, 0xD9, 0xD5, NA, NA, NA, 0xC1,  0xD1,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                // C_CMP
    NA, NA, 0xE0, 0xE4, 0xEC, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                          // C_CPX
    NA, NA, 0xC0, 0xC4, 0xCC, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                          // C_CPY
    NA, NA, NA, 0xE6, 0xEE, 0xFE, NA, 0xF6, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                        // C_INC
    NA, NA, NA, 0xC6, 0xCE, 0xDE, NA, 0xD6, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                        // C_DEC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_INA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_DEA
    0xE8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_INX
    0xCA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_DEX
    0xC8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_INY
    0x88, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_DEY
    NA, 0x0A, NA, 0x06, 0x0E, 0x1E, NA, 0x16, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                      // C_ASL
    NA, 0x4A, NA, 0x46, 0x4E, 0x5E, NA, 0x56, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                      // C_LSR
    NA, 0x2A, NA, 0x26, 0x2E, 0x3E, NA, 0x36, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                      // C_ROL
    NA, 0x6A, NA, 0x66, 0x6E, 0x7E, NA, 0x76, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                      // C_ROR
    NA, NA, 0x29, 0x25, 0x2D, 0x3D, 0x39, 0x35, NA, NA, NA, 0x21,  0x31,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                // C_AND
    NA, NA, 0x09, 0x05, 0x0D, 0x1D, 0x19, 0x15, NA, NA, NA, 0x01,  0x11,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                // C_ORA
    NA, NA, 0x49, 0x45, 0x4D, 0x5D, 0x59, 0x55, NA, NA, NA, 0x41,  0x51,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                // C_EOR
    NA, NA, NA, 0x24, 0x2C, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                            // C_BIT
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TSB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TRB
    NA, NA, NA, NA, 0x4C, NA, NA, NA, NA, NA, NA, NA,  NA,  0x6C, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                            // C_JMP
    NA, NA, NA, NA, 0x20, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_JSR
    0x00, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BRK
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_BRA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x10, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BPL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x30, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BMI
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x50, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BVC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x70, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BVS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x90, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BCC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xB0, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BCS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xD0, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BNE
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xF0, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_BEQ
    0x60, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_RTS
    0x40, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_RTI
    0x48, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_PHA
    0x68, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_PLA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PHX
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PLX
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PHY
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PLY
    0x08, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_PHP
    0x28, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_PLP
    0x18, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_CLC
    0x38, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_SEC
    0xB8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_CLV
    0xD8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_CLD
    0xF8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_SED
    0x58, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_CLI
    0x78, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_SEI
    0xEA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                              // C_NOP
    NA,	NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_BBR
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_BBS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_RMB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_SMB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_BRL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_COP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_JML
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_JSL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_MVN
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_MVP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PEA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PEI
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PER
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PHB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PHD
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PHK
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PLB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_PLD
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_REP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_RTL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_SEP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_STP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TCD
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TCS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TDC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TSC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TXY
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_TYX
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_WAI
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_WDM
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA,                                                // C_XBA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,  NA                                                 // C_XCE
};

//-----------------------------------------------------------------------------

// Converting an instruction in a given addressing mode into a code (65C02)
const uint8_t CAsm::trans_table_c[C_ILL][A_NO_OF_MODES] =
{
// IMP   ACC   IMM   ZPG   ABS  ABS_X ABS_Y ZPG_X ZPG_Y  REL  ZPGI ZPGI_X ZPGI_Y  ABSI ABSI_X ZREL ZPG2 ABSL ABSL_X ZPIL  ZPIL_Y INDL  SR  SRI_Y A_RELL A_XYC A_IMM2
    NA, NA, 0xA9, 0xA5, 0xAD, 0xBD, 0xB9, 0xB5, NA, NA, 0xB2, 0xA1,  0xB1,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_LDA
    NA, NA, 0xA2, 0xA6, 0xAE, NA, 0xBE, NA, 0xB6, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_LDX
    NA, NA, 0xA0, 0xA4, 0xAC, 0xBC, NA, 0xB4, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_LDY
    NA, NA, NA, 0x85, 0x8D, 0x9D, 0x99, 0x95, NA, NA, 0x92, 0x81,  0x91,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                // C_STA
    NA, NA, NA, 0x86, 0x8E, NA, NA, NA, 0x96, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                          // C_STX
    NA, NA, NA, 0x84, 0x8C, NA, NA, 0x94, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                          // C_STY
    NA, NA, NA, 0x64, 0x9C, 0x9E, NA, 0x74, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                        // C_STZ
    0xAA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_TAX
    0x8A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_TXA
    0xA8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_TAY
    0x98, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_TYA
    0x9A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_TXS
    0xBA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_TSX
    NA, NA, 0x69, 0x65, 0x6D, 0x7D, 0x79, 0x75, NA, NA, 0x72, 0x61,  0x71,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_ADC
    NA, NA, 0xE9, 0xE5, 0xED, 0xFD, 0xF9, 0xF5, NA, NA, 0xF2, 0xE1,  0xF1,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_SBC
    NA, NA, 0xC9, 0xC5, 0xCD, 0xDD, 0xD9, 0xD5, NA, NA, 0xD2, 0xC1,  0xD1,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_CMP
    NA, NA, 0xE0, 0xE4, 0xEC, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                          // C_CPX
    NA, NA, 0xC0, 0xC4, 0xCC, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                          // C_CPY
    NA, 0x1A, NA, 0xE6, 0xEE, 0xFE, NA, 0xF6, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_INC
    NA, 0x3A, NA, 0xC6, 0xCE, 0xDE, NA, 0xD6, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_DEC
    NA, 0x1A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_INA
    NA, 0x3A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_DEA
    0xE8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_INX
    0xCA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_DEX
    0xC8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_INY
    0x88, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_DEY
    NA, 0x0A, NA, 0x06, 0x0E, 0x1E, NA, 0x16, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_ASL
    NA, 0x4A, NA, 0x46, 0x4E, 0x5E, NA, 0x56, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_LSR
    NA, 0x2A, NA, 0x26, 0x2E, 0x3E, NA, 0x36, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_ROL
    NA, 0x6A, NA, 0x66, 0x6E, 0x7E, NA, 0x76, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_ROR
    NA, NA, 0x29, 0x25, 0x2D, 0x3D, 0x39, 0x35, NA, NA, 0x32, 0x21,  0x31,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_AND
    NA, NA, 0x09, 0x05, 0x0D, 0x1D, 0x19, 0x15, NA, NA, 0x12, 0x01,  0x11,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_ORA
    NA, NA, 0x49, 0x45, 0x4D, 0x5D, 0x59, 0x55, NA, NA, 0x52, 0x41,  0x51,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                              // C_EOR
    NA, NA, 0x89, 0x24, 0x2C, 0x3C, NA, 0x34, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                      // C_BIT
    NA, NA, NA, 0x04, 0x0C, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                            // C_TSB
    NA, NA, NA, 0x14, 0x1C, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                            // C_TRB
    NA, NA, NA, NA, 0x4C, NA, NA, NA, NA, NA, NA, NA,  NA,  0x6C, 0x7C, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                          // C_JMP
    NA, NA, NA, NA, 0x20, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_JSR
    0x00, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BRK
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x80, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BRA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x10, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BPL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x30, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BMI
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x50, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BVC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x70, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BVS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0x90, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BCC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xB0, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BCS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xD0, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BNE
    NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xF0, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_BEQ
    0x60, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_RTS
    0x40, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_RTI
    0x48, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PHA
    0x68, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PLA
    0xDA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PHX
    0xFA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PLX
    0x5A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PHY
    0x7A, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PLY
    0x08, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PHP
    0x28, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_PLP
    0x18, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_CLC
    0x38, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_SEC
    0xB8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_CLV
    0xD8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_CLD
    0xF8, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_SED
    0x58, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_CLI
    0x78, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_SEI
    0xEA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_NOP
    NA,	NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, 0x0F,NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                               // C_BBR
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, 0x8F,NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                               // C_BBS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, 0x07, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_RMB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, 0x87, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                              // C_SMB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_BRL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_COP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_JML
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_JSL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_MVN
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_MVP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PEA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PEI
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PER
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PHB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PHD
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PHK
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PLB
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_PLD
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_REP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_RTL
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_SEP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_STP
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_TCD
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_TCS
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_TDC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_TSC
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_TXY
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_TYX
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_WAI
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_WDM
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA,                                                // C_XBA
    NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA, NA, NA,  NA,  NA, NA, NA, NA,   NA, NA,   NA                                                 // C_XCE
};

const uint8_t CAsm::trans_table_8[C_ILL][A_NO_OF_MODES] =
{
// IMP   ACC   IMM   ZPG   ABS  ABS_X ABS_Y ZPG_X ZPG_Y  REL ZPGI ZPGI_X ZPGI_Y ABSI ABSI_X ZREL  ZPG2  ABSL ABSL_X ZPIL ZPIL_Y INDL  SR SRI_Y, A_RELL, A_XYC  A_IMM2
    NA,  NA, 0xA9, 0xA5, 0xAD, 0xBD, 0xB9, 0xB5,  NA,  NA, 0xB2, 0xA1, 0xB1,  NA,  NA,  NA,  NA, 0xAF, 0xBF, 0xA7, 0xB7, NA, 0xA3, 0xB3,  NA,  NA,  0xA9,             // C_LDA A9
    NA,  NA, 0xA2, 0xA6, 0xAE,  NA, 0xBE,  NA, 0xB6,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,  0xA2,                       // C_LDX
    NA,  NA, 0xA0, 0xA4, 0xAC, 0xBC,  NA, 0xB4,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,  0xA0,                       // C_LDY
    NA,  NA,  NA, 0x85, 0x8D, 0x9D, 0x99, 0x95,  NA,  NA, 0x92, 0x81, 0x91,  NA,  NA,  NA,  NA, 0x8F, 0x9F, 0x87, 0x97, NA, 0x83, 0x93,  NA,  NA,    NA,              // C_STA
    NA,  NA,  NA, 0x86, 0x8E,  NA,  NA,  NA, 0x96,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                         // C_STX
    NA,  NA,  NA, 0x84, 0x8C,  NA,  NA, 0x94,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                         // C_STY
    NA,  NA,  NA, 0x64, 0x9C, 0x9E,  NA, 0x74,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                        // C_STZ
    0xAA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TAX
    0x8A,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TXA
    0xA8,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TAY
    0x98,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TYA
    0x9A,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TXS
    0xBA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TSX
    NA,  NA, 0x69, 0x65, 0x6D, 0x7D, 0x79, 0x75,  NA,  NA, 0x72, 0x61, 0x71,  NA,  NA,  NA,  NA, 0x6F, 0x7F, 0x67, 0x77, NA, 0x63, 0x73,  NA,  NA,  0x69,             // C_ADC
    NA,  NA, 0xE9, 0xE5, 0xED, 0xFD, 0xF9, 0xF5,  NA,  NA, 0xF2, 0xE1, 0xF1,  NA,  NA,  NA,  NA, 0xEF, 0xFF, 0xE7, 0xF7, NA, 0xE3, 0xF3,  NA,  NA,  0xE9,             // C_SBC
    NA,  NA, 0xC9, 0xC5, 0xCD, 0xDD, 0xD9, 0xD5,  NA,  NA, 0xD2, 0xC1, 0xD1,  NA,  NA,  NA,  NA, 0xCF, 0xDF, 0xC7, 0xD7, NA, 0xC3, 0xD3,  NA,  NA,  0xC9,             // C_CMP
    NA,  NA, 0xE0, 0xE4, 0xEC,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,  0xE0,                         // C_CPX
    NA,  NA, 0xC0, 0xC4, 0xCC,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,  0xC0,                         // C_CPY
    NA, 0x1A,  NA, 0xE6, 0xEE, 0xFE,  NA, 0xF6,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                       // C_INC
    NA, 0x3A,  NA, 0xC6, 0xCE, 0xDE,  NA, 0xD6,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                       // C_DEC
    NA, 0x1A,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_INA
    NA, 0x3A,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_DEA
    0xE8,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_INX
    0xCA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_DEX
    0xC8,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_INY
    0x88,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_DEY
    NA, 0x0A,  NA, 0x06, 0x0E, 0x1E,  NA, 0x16,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                       // C_ASL
    NA, 0x4A,  NA, 0x46, 0x4E, 0x5E,  NA, 0x56,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                       // C_LSR
    NA, 0x2A,  NA, 0x26, 0x2E, 0x3E,  NA, 0x36,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                       // C_ROL
    NA, 0x6A,  NA, 0x66, 0x6E, 0x7E,  NA, 0x76,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                       // C_ROR
    NA,  NA, 0x29, 0x25, 0x2D, 0x3D, 0x39, 0x35,  NA,  NA, 0x32, 0x21, 0x31,  NA,  NA,  NA,  NA, 0x2F, 0x3F, 0x27, 0x37, NA, 0x23, 0x33,  NA,  NA,  0x29,             // C_AND
    NA,  NA, 0x09, 0x05, 0x0D, 0x1D, 0x19, 0x15,  NA,  NA, 0x12, 0x01, 0x11,  NA,  NA,  NA,  NA, 0x0F, 0x1F, 0x07, 0x17, NA, 0x03, 0x13,  NA,  NA,  0x09,             // C_ORA
    NA,  NA, 0x49, 0x45, 0x4D, 0x5D, 0x59, 0x55,  NA,  NA, 0x52, 0x41, 0x51,  NA,  NA,  NA,  NA, 0x4F, 0x5F, 0x47, 0x57, NA, 0x43, 0x53,  NA,  NA,  0x49,             // C_EOR
    NA,  NA, 0x89, 0x24, 0x2C, 0x3C,  NA, 0x34,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,  0x89,                       // C_BIT
    NA,  NA,  NA, 0x04, 0x0C,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                          // C_TSB
    NA,  NA,  NA, 0x14, 0x1C,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                          // C_TRB
    NA,  NA,  NA,  NA, 0x4C,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x6C, 0x7C,  NA,  NA,  NA,  NA,  NA,  NA, 0XDC,  NA,  NA,  NA,  NA,    NA,                       // C_JMP
    NA,  NA,  NA,  NA, 0x20,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0xFC,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                          // C_JSR
    0x00,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BRK
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x80,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BRA
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x10,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BPL
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x30,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BMI
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x50,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BVC
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x70,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BVS
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x90,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BCC
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0xB0,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BCS
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0xD0,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BNE
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0xF0,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_BEQ
    0x60,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_RTS
    0x40,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_RTI
    0x48,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHA
    0x68,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PLA
    0xDA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHX
    0xFA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PLX
    0x5A,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHY
    0x7A,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PLY
    0x08,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHP
    0x28,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PLP
    0x18,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_CLC
    0x38,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_SEC
    0xB8,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_CLV
    0xD8,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_CLD
    0xF8,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_SED
    0x58,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_CLI
    0x78,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_SEI
    0xEA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_NOP
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                            // C_BBR
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                            // C_BBS
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                            // C_RMB
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                            // C_SMB
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA, 0x82,  NA,    NA,                           // C_BRL
    NA,  NA, 0x02,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_COP
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x5C,  NA,  NA,  NA, 0XDC,  NA,  NA,  NA,  NA,    NA,                         // C_JML
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, 0x22,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_JSL
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA, 0x54,    NA,                           // C_MVN
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA, 0x44,    NA,                           // C_MVP
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,  0xF4,                            // C_PEA
    NA,  NA,  NA, 0xD4,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PEI
    NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA, 0x62,  NA,    NA,                           // C_PER
    0x8B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHB
    0x0B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHD
    0x4B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PHK
    0xAB,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PLB
    0x2B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_PLD
    NA,  NA, 0xC2,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_REP
    0x6B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_RTL
    NA,  NA, 0xE2,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_SEP
    0xDB,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_STP
    0x5B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TCD
    0x1B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TCS
    0x7B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TDC
    0x3B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TSC
    0x9B,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TXY
    0xBB,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_TYX
    0xCB,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_WAI
    NA,  NA, 0x42,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_WDM
    0xEB,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA,                           // C_XBA
    0xFB,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA, NA,  NA,  NA,  NA,  NA,    NA                            // C_XCE
};

//-----------------------------------------------------------------------------

// Transformation table of the instruction code to the corresponding addressing mode 6502
const uint8_t CAsm::code_to_mode[] =
{
    // x0     x1	     x2     x3     x4      x5       x6       x7     x8     x9       xA     xB     xC      xD 	   xE	    xF
    A_IMP2,A_ZPGI_X, A_ILL, A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ILL,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
    A_ABS, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
    A_IMP, A_ZPGI_X, A_ILL, A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
    A_IMP, A_ZPGI_X, A_ILL, A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABSI, A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
    A_ILL, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_ILL,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_Y, A_ILL, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ILL,  A_ABS_X, A_ILL,   A_ILL,
    A_IMM, A_ZPGI_X, A_IMM, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_Y, A_ILL, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ABS_X,A_ABS_X, A_ABS_Y, A_ILL,
    A_IMM, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
    A_IMM, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
    A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL
};

//% bug fix 1.2.13.2 - 65c02 code to mode table - added unused opcode addressing modes to allow simulation
const uint8_t CAsm::code_to_mode_c[] =
{
    // x0     x1	     x2     x3     x4      x5       x6       x7      x8     x9       xA     xB     xC       xD 	     xE       xF
    A_IMP2,A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // 0x
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG,   A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_ACC, A_IMP, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL,  // 1x
    A_ABS, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // 2x
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_ACC, A_IMP, A_ABS_X, A_ABS_X, A_ABS_X, A_ZREL,  // 3x
    A_IMP, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // 4x
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL,  // 5x
    A_IMP, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_IMP, A_ABSI,  A_ABS,   A_ABS,   A_ZREL,  // 6x
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABSI_X,A_ABS_X, A_ABS_X, A_ZREL,  // 7x
    A_REL, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // 8x
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_Y, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL,  // 9x
    A_IMM, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // Ax
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_Y, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS_X, A_ABS_X, A_ABS_Y, A_ZREL,  // Bx
    A_IMM, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // Cx
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL,  // Dx
    A_IMM, A_ZPGI_X, A_IMM,  A_IMP, A_ZPG,   A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,   A_ABS,   A_ABS,   A_ZREL,  // Ex
    A_REL, A_ZPGI_Y, A_ZPGI, A_IMP, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL   // Fx
};

const uint8_t CAsm::code_to_mode_8[] =
{
    // x0     x1	     x2      x3       x4       x5       x6       x7        x8     x9       xA     xB     xC        xD 	    xE       xF
    A_IMP2, A_ZPGI_X, A_IMM,  A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_ACC, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // 0x
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_ZPG,   A_ZPG_X, A_ZPG_X, A_ZPIL_Y, A_IMP, A_ABS_Y, A_ACC, A_IMP, A_ABS,    A_ABS_X, A_ABS_X, A_ABSL_X,  // 1x
    A_ABS, A_ZPGI_X, A_ABSL, A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_ACC, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // 2x
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPIL_Y, A_IMP, A_ABS_Y, A_ACC, A_IMP, A_ABS_X,  A_ABS_X, A_ABS_X, A_ABSL_X,  // 3x
    A_IMP, A_ZPGI_X, A_ACC,  A_SR,    A_XYC,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_ACC, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // 4x
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_XYC,   A_ZPG_X, A_ZPG_X, A_ZPIL_Y, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABSL,   A_ABS_X, A_ABS_X, A_ABSL_X,  // 5x
    A_IMP, A_ZPGI_X, A_RELL, A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_ACC, A_IMP, A_ABSI,   A_ABS,   A_ABS,   A_ABSL,    // 6x
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_ZPG_X, A_ZPG_X, A_ZPG_X, A_ZPIL_Y, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABSI_X, A_ABS_X, A_ABS_X, A_ABSL_X,  // 7x
    A_REL, A_ZPGI_X, A_RELL, A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // 8x
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_ZPG_X, A_ZPG_X, A_ZPG_Y, A_ZPIL_Y, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS,    A_ABS_X, A_ABS_X, A_ABSL_X,  // 9x
    A_IMM, A_ZPGI_X, A_IMM,  A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // Ax
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_ZPG_X, A_ZPG_X, A_ZPG_Y, A_ZPIL_Y, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABS_X,  A_ABS_X, A_ABS_Y, A_ABSL_X,  // Bx
    A_IMM, A_ZPGI_X, A_IMM,  A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // Cx
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_ZPG,   A_ZPG_X, A_ZPG_X, A_ZPIL_Y, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_INDL,   A_ABS_X, A_ABS_X, A_ABSL_X,  // Dx
    A_IMM, A_ZPGI_X, A_IMM,  A_SR,    A_ZPG,   A_ZPG,   A_ZPG,   A_ZPIL,   A_IMP, A_IMM,   A_IMP, A_IMP, A_ABS,    A_ABS,   A_ABS,   A_ABSL,    // Ex
    A_REL, A_ZPGI_Y, A_ZPGI, A_SRI_Y, A_IMM2,  A_ZPG_X, A_ZPG_X, A_ZPIL_Y, A_IMP, A_ABS_Y, A_IMP, A_IMP, A_ABSI_X, A_ABS_X, A_ABS_X, A_ABSL_X   // Fx
};


// Transformation table of the instruction code into the corresponding instruction (action) of the 6502 processor
const uint8_t CAsm::code_to_command[] =
{
    // x0     x1	  x2     x3     x4     x5     x6     x7     x8     x9     xA     xB     xC     xD     xE     xF
    C_BRK, C_ORA, C_ILL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, C_PHP, C_ORA, C_ASL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, // 0
    C_BPL, C_ORA, C_ILL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, C_CLC, C_ORA, C_ILL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, // 1
    C_JSR, C_AND, C_ILL, C_ILL, C_BIT, C_AND, C_ROL, C_ILL, C_PLP, C_AND, C_ROL, C_ILL, C_BIT, C_AND, C_ROL, C_ILL, // 2
    C_BMI, C_AND, C_ILL, C_ILL, C_ILL, C_AND, C_ROL, C_ILL, C_SEC, C_AND, C_ILL, C_ILL, C_ILL, C_AND, C_ROL, C_ILL, // 3
    C_RTI, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_ILL, C_PHA, C_EOR, C_LSR, C_ILL, C_JMP, C_EOR, C_LSR, C_ILL, // 4
    C_BVC, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_ILL, C_CLI, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_ILL, // 5
    C_RTS, C_ADC, C_ILL, C_ILL, C_ILL, C_ADC, C_ROR, C_ILL, C_PLA, C_ADC, C_ROR, C_ILL, C_JMP, C_ADC, C_ROR, C_ILL, // 6
    C_BVS, C_ADC, C_ILL, C_ILL, C_ILL, C_ADC, C_ROR, C_ILL, C_SEI, C_ADC, C_ILL, C_ILL, C_ILL, C_ADC, C_ROR, C_ILL, // 7

    C_ILL, C_STA, C_ILL, C_ILL, C_STY, C_STA, C_STX, C_ILL, C_DEY, C_ILL, C_TXA, C_ILL, C_STY, C_STA, C_STX, C_ILL, // 8
    C_BCC, C_STA, C_ILL, C_ILL, C_STY, C_STA, C_STX, C_ILL, C_TYA, C_STA, C_TXS, C_ILL, C_ILL, C_STA, C_ILL, C_ILL, // 9
    C_LDY, C_LDA, C_LDX, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, C_TAY, C_LDA, C_TAX, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, // A
    C_BCS, C_LDA, C_ILL, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, C_CLV, C_LDA, C_TSX, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, // B
    C_CPY, C_CMP, C_ILL, C_ILL, C_CPY, C_CMP, C_DEC, C_ILL, C_INY, C_CMP, C_DEX, C_ILL, C_CPY, C_CMP, C_DEC, C_ILL, // C
    C_BNE, C_CMP, C_ILL, C_ILL, C_ILL, C_CMP, C_DEC, C_ILL, C_CLD, C_CMP, C_ILL, C_ILL, C_ILL, C_CMP, C_DEC, C_ILL, // D
    C_CPX, C_SBC, C_ILL, C_ILL, C_CPX, C_SBC, C_INC, C_ILL, C_INX, C_SBC, C_NOP, C_ILL, C_CPX, C_SBC, C_INC, C_ILL, // E
    C_BEQ, C_SBC, C_ILL, C_ILL, C_ILL, C_SBC, C_INC, C_ILL, C_SED, C_SBC, C_ILL, C_ILL, C_ILL, C_SBC, C_INC, C_ILL  // F
};

// As above for 65c02
const uint8_t CAsm::code_to_command_c[] =
{
    // x0     x1	  x2     x3     x4     x5     x6     x7     x8     x9     xA     xB     xC     xD     xE     xF
    C_BRK, C_ORA, C_ILL, C_ILL, C_TSB, C_ORA, C_ASL, C_RMB, C_PHP, C_ORA, C_ASL, C_ILL, C_TSB, C_ORA, C_ASL, C_BBR,
    C_BPL, C_ORA, C_ORA, C_ILL, C_TRB, C_ORA, C_ASL, C_RMB, C_CLC, C_ORA, C_INC, C_ILL, C_TRB, C_ORA, C_ASL, C_BBR,
    C_JSR, C_AND, C_ILL, C_ILL, C_BIT, C_AND, C_ROL, C_RMB, C_PLP, C_AND, C_ROL, C_ILL, C_BIT, C_AND, C_ROL, C_BBR,
    C_BMI, C_AND, C_AND, C_ILL, C_BIT, C_AND, C_ROL, C_RMB, C_SEC, C_AND, C_DEC, C_ILL, C_BIT, C_AND, C_ROL, C_BBR,
    C_RTI, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_RMB, C_PHA, C_EOR, C_LSR, C_ILL, C_JMP, C_EOR, C_LSR, C_BBR,
    C_BVC, C_EOR, C_EOR, C_ILL, C_ILL, C_EOR, C_LSR, C_RMB, C_CLI, C_EOR, C_PHY, C_ILL, C_ILL, C_EOR, C_LSR, C_BBR,
    C_RTS, C_ADC, C_ILL, C_ILL, C_STZ, C_ADC, C_ROR, C_RMB, C_PLA, C_ADC, C_ROR, C_ILL, C_JMP, C_ADC, C_ROR, C_BBR,
    C_BVS, C_ADC, C_ADC, C_ILL, C_STZ, C_ADC, C_ROR, C_RMB, C_SEI, C_ADC, C_PLY, C_ILL, C_JMP, C_ADC, C_ROR, C_BBR,

    C_BRA, C_STA, C_ILL, C_ILL, C_STY, C_STA, C_STX, C_SMB, C_DEY, C_BIT, C_TXA, C_ILL, C_STY, C_STA, C_STX, C_BBS,
    C_BCC, C_STA, C_STA, C_ILL, C_STY, C_STA, C_STX, C_SMB, C_TYA, C_STA, C_TXS, C_ILL, C_STZ, C_STA, C_STZ, C_BBS,
    C_LDY, C_LDA, C_LDX, C_ILL, C_LDY, C_LDA, C_LDX, C_SMB, C_TAY, C_LDA, C_TAX, C_ILL, C_LDY, C_LDA, C_LDX, C_BBS,
    C_BCS, C_LDA, C_LDA, C_ILL, C_LDY, C_LDA, C_LDX, C_SMB, C_CLV, C_LDA, C_TSX, C_ILL, C_LDY, C_LDA, C_LDX, C_BBS,
    C_CPY, C_CMP, C_ILL, C_ILL, C_CPY, C_CMP, C_DEC, C_SMB, C_INY, C_CMP, C_DEX, C_ILL, C_CPY, C_CMP, C_DEC, C_BBS,
    C_BNE, C_CMP, C_CMP, C_ILL, C_ILL, C_CMP, C_DEC, C_SMB, C_CLD, C_CMP, C_PHX, C_ILL, C_ILL, C_CMP, C_DEC, C_BBS,
    C_CPX, C_SBC, C_ILL, C_ILL, C_CPX, C_SBC, C_INC, C_SMB, C_INX, C_SBC, C_NOP, C_ILL, C_CPX, C_SBC, C_INC, C_BBS,
    C_BEQ, C_SBC, C_SBC, C_ILL, C_ILL, C_SBC, C_INC, C_SMB, C_SED, C_SBC, C_PLX, C_ILL, C_ILL, C_SBC, C_INC, C_BBS
};

const uint8_t CAsm::code_to_command_8[] =
{
    // x0     x1	  x2     x3     x4     x5     x6     x7     x8     x9     xA     xB     xC     xD     xE     xF
    C_BRK, C_ORA, C_COP, C_ORA, C_TSB, C_ORA, C_ASL, C_ORA, C_PHP, C_ORA, C_ASL, C_PHD, C_TSB, C_ORA, C_ASL, C_ORA,	// 0x
    C_BPL, C_ORA, C_ORA, C_ORA, C_TRB, C_ORA, C_ASL, C_ORA, C_CLC, C_ORA, C_INC, C_TCS, C_TRB, C_ORA, C_ASL, C_ORA,	// 1x
    C_JSR, C_AND, C_JSL, C_AND, C_BIT, C_AND, C_ROL, C_AND, C_PLP, C_AND, C_ROL, C_PLD, C_BIT, C_AND, C_ROL, C_AND,	// 2x
    C_BMI, C_AND, C_AND, C_AND, C_BIT, C_AND, C_ROL, C_AND, C_SEC, C_AND, C_DEC, C_TSC, C_BIT, C_AND, C_ROL, C_AND,	// 3x
    C_RTI, C_EOR, C_WDM, C_EOR, C_MVP, C_EOR, C_LSR, C_EOR, C_PHA, C_EOR, C_LSR, C_PHK, C_JMP, C_EOR, C_LSR, C_EOR,	// 4x
    C_BVC, C_EOR, C_EOR, C_EOR, C_MVN, C_EOR, C_LSR, C_EOR, C_CLI, C_EOR, C_PHY, C_TCD, C_JML, C_EOR, C_LSR, C_EOR,	// 5x
    C_RTS, C_ADC, C_PER, C_ADC, C_STZ, C_ADC, C_ROR, C_ADC, C_PLA, C_ADC, C_ROR, C_RTL, C_JMP, C_ADC, C_ROR, C_ADC,	// 6x
    C_BVS, C_ADC, C_ADC, C_ADC, C_STZ, C_ADC, C_ROR, C_ADC, C_SEI, C_ADC, C_PLY, C_TDC, C_JMP, C_ADC, C_ROR, C_ADC,	// 7x
    C_BRA, C_STA, C_BRL, C_STA, C_STY, C_STA, C_STX, C_STA, C_DEY, C_BIT, C_TXA, C_PHB, C_STY, C_STA, C_STX, C_STA,	// 8x
    C_BCC, C_STA, C_STA, C_STA, C_STY, C_STA, C_STX, C_STA, C_TYA, C_STA, C_TXS, C_TXY, C_STZ, C_STA, C_STZ, C_STA,	// 9x
    C_LDY, C_LDA, C_LDX, C_LDA, C_LDY, C_LDA, C_LDX, C_LDA, C_TAY, C_LDA, C_TAX, C_PLB, C_LDY, C_LDA, C_LDX, C_LDA,	// Ax
    C_BCS, C_LDA, C_LDA, C_LDA, C_LDY, C_LDA, C_LDX, C_LDA, C_CLV, C_LDA, C_TSX, C_TYX, C_LDY, C_LDA, C_LDX, C_LDA,	// Bx
    C_CPY, C_CMP, C_REP, C_CMP, C_CPY, C_CMP, C_DEC, C_CMP, C_INY, C_CMP, C_DEX, C_WAI, C_CPY, C_CMP, C_DEC, C_CMP,	// Cx
    C_BNE, C_CMP, C_CMP, C_CMP, C_PEI, C_CMP, C_DEC, C_CMP, C_CLD, C_CMP, C_PHX, C_STP, C_JML, C_CMP, C_DEC, C_CMP,	// Dx
    C_CPX, C_SBC, C_SEP, C_SBC, C_CPX, C_SBC, C_INC, C_SBC, C_INX, C_SBC, C_NOP, C_XBA, C_CPX, C_SBC, C_INC, C_SBC,	// Ex
    C_BEQ, C_SBC, C_SBC, C_SBC, C_PEA, C_SBC, C_INC, C_SBC, C_SED, C_SBC, C_PLX, C_XCE, C_JSR, C_SBC, C_INC, C_SBC	// Fx
};


const uint8_t CAsm::mode_to_len[] = // Changing the addressing mode to the length of the instruction and arguments
{
    1,	// A_IMP,	 implied
    1,	// A_ACC,	 accumulator
    2,	// A_IMM,	 immediate
    2,	// A_ZPG,	 zero page
    3,	// A_ABS,	 absolute
    3,	// A_ABS_X,	 absolute indexed X
    3,	// A_ABS_Y,	 absolute indexed Y
    2,	// A_ZPG_X,	 zero page indexed X
    2,	// A_ZPG_Y,	 zero page indexed Y
    2,	// A_REL,	 relative
    2,	// A_ZPGI,	 zero page indirect
    2,	// A_ZPGI_X,	 zero page indirect, indexed X
    2,	// A_ZPGI_Y,	 zero page indirect, indexed Y
    3,	// A_ABSI,	 absolute indirect
    3,	// A_ABSI_X,	 absolute indirect, indexed X
    3,	// A_ZREL,	 zero page / relative -> BBS i BBR z 6501
    2,	// A_ZPG2,	 zero page for RMB SMB commands from 6501
    4,	// A_ABSL,
    4,	// A_ABSL_X,
    2,	// A_ZPIL,
    2,	// A_ZPIL_Y,
    2,  // A_INDL
    2,	// A_SR,       A_RELL,  A_XYC
    2,	// A_SRI_Y	(d,s),Y
    3,	// A_RELL	rel long
    3,	// A_XYC		MVN,MVP
    3,  // A_IMM2 - 16 bit immediate
    0,  // A_NO_OF_MODES, number of addressing modes
    //A_ABS_OR_ZPG= A_NO_OF_MODES, undetermined addressing mode
    0,	// A_ABSX_OR_ZPGX,
    0,	// A_ABSY_OR_ZPGY,
    0,	// A_ABSI_OR_ZPGI,
    0,	// A_IMP_OR_ACC,
    0,	// A_ABSIX_OR_ZPGIX,
    2,	// A_IMP2,	 implied for the BRK command
    1	// A_ILL	 value for marking illegal commands in the simulator (ILLEGAL)
};


//% bug fix 1.2.13.1 - This timing table updated by Daryl Rictor
const uint8_t CAsm::code_cycles[256] =
{
//0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,   // 0x
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,   // 1x
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,   // 2x
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,   // 3x
    6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,   // 4x
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,   // 5x
    6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,   // 6x
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,   // 7x
    0, 6, 0, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,   // 8x
    2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,   // 9x
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,   // ax
    2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,   // bx
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,   // cx
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,   // dx
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,   // ex
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0    // fx
};

//% bug fix 1.2.13.1 - This timing table updated by Daryl Rictor
const uint8_t CAsm::code_cycles_c[256] =
{
//0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    7, 6, 2, 1, 5, 3, 5, 5, 3, 2, 2, 1, 6, 4, 6, 5, // 0x
    2, 5, 5, 1, 5, 4, 6, 5, 2, 4, 2, 1, 6, 4, 6, 5, // 1x
    6, 6, 2, 1, 3, 3, 5, 5, 4, 2, 2, 1, 4, 4, 6, 5, // 2x
    2, 5, 5, 1, 4, 4, 6, 5, 2, 4, 2, 1, 4, 4, 6, 5, // 3x
    6, 6, 2, 1, 3, 3, 5, 5, 3, 2, 2, 1, 3, 4, 6, 5, // 4x
    2, 5, 5, 1, 4, 4, 6, 5, 2, 4, 3, 1, 8, 4, 6, 5, // 5x
    6, 6, 2, 1, 3, 3, 5, 5, 4, 2, 2, 1, 6, 4, 6, 5, // 6x
    2, 5, 5, 1, 4, 4, 6, 5, 2, 4, 4, 1, 6, 4, 6, 5, // 7x
    2, 6, 2, 1, 3, 3, 3, 5, 2, 2, 2, 1, 4, 4, 4, 5, // 8x
    2, 6, 5, 1, 4, 4, 4, 5, 2, 5, 2, 1, 4, 5, 5, 5, // 9x
    2, 6, 2, 1, 3, 3, 3, 5, 2, 2, 2, 1, 4, 4, 4, 5, // ax
    2, 5, 5, 1, 4, 4, 4, 5, 2, 4, 2, 1, 4, 4, 4, 5, // bx
    2, 6, 2, 1, 3, 3, 5, 5, 2, 2, 2, 1, 4, 4, 6, 5, // cx
    2, 5, 5, 1, 4, 4, 6, 5, 2, 4, 3, 1, 4, 4, 7, 5, // dx
    2, 6, 2, 1, 3, 3, 5, 5, 2, 2, 2, 1, 4, 4, 6, 5, // ex
    2, 5, 5, 1, 4, 4, 6, 5, 2, 4, 4, 1, 4, 4, 7, 5 // fx
};

const uint8_t CAsm::code_cycles_8[256] =
{
  //0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    7, 6, 7, 4, 5, 3, 5, 6, 3, 2, 2, 4, 6, 4, 6, 5, // 0x
    2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 2, 2, 6, 4, 7, 5, // 1x
    6, 6, 8, 4, 3, 3, 5, 6, 4, 2, 2, 5, 5, 4, 6, 5, // 2x
    2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 2, 2, 4, 4, 7, 5, // 3x
    7, 6, 2, 4, 7, 3, 5, 6, 3, 2, 2, 3, 3, 4, 6, 5, // 4x
    2, 5, 5, 7, 7, 4, 6, 6, 2, 4, 3, 2, 4, 4, 7, 5, // 5x
    6, 6, 6, 4, 3, 3, 5, 6, 4, 2, 2, 6, 5, 4, 6, 5, // 6x
    2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5, // 7x
    2, 6, 4, 4, 3, 3, 3, 2, 2, 2, 2, 3, 4, 4, 4, 5, // 8x
    2, 6, 5, 7, 4, 4, 4, 6, 2, 5, 2, 3, 4, 5, 5, 5, // 9x
    2, 6, 2, 4, 3, 3, 3, 6, 2, 2, 2, 4, 4, 4, 4, 5, // ax
    2, 5, 5, 7, 4, 4, 4, 6, 2, 4, 2, 2, 4, 4, 4, 5, // bx
    2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5, // cx
    2, 5, 5, 7, 6, 4, 6, 6, 2, 4, 3, 3, 6, 4, 7, 5, // dx
    2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5, // ex
    2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 4, 2, 8, 4, 7, 5  // fx
};

const uint8_t (&CAsm::TransformTable(const ProcessorType procType))[C_ILL][A_NO_OF_MODES]
{
    switch (procType)
    {
    case ProcessorType::M6502:
        return trans_table;

    case ProcessorType::WDC65C02:
        return trans_table_c;

    case ProcessorType::WDC65816:
        return trans_table_8;

    default:
        throw 0; // TODO: Throw real exception later.
    }
}

const uint8_t (&CAsm::CodeToCommand())[0x100]
{
    return CodeToCommand(wxGetApp().m_global.m_procType);
}

const uint8_t (&CAsm::CodeToCommand(const ProcessorType procType))[0x100]
{
    switch (procType)
    {
    case ProcessorType::M6502:
        return code_to_command;
    
    case ProcessorType::WDC65C02:
        return code_to_command_c;

    case ProcessorType::WDC65816:
        return code_to_command_8;

    default:
        throw 0; // TODO: Throw real exception later.
    }
}

const uint8_t (&CAsm::CodeToMode())[0x100]
{
    return CodeToMode(wxGetApp().m_global.m_procType);
}

const uint8_t (&CAsm::CodeToMode(const ProcessorType procType))[0x100]
{
    switch (procType)
    {
    case ProcessorType::M6502:
        return code_to_mode;

    case ProcessorType::WDC65C02:
        return code_to_mode_c;

    case ProcessorType::WDC65816:
        return code_to_mode_8;

    default:
        throw 0; // TODO: Throw real exception later.
    }
}

inline ProcessorType CAsm::ProcType()
{
    return wxGetApp().m_global.m_procType;
}

const uint8_t (&CAsm::CodeToCycles())[0x100]
{
    return CodeToCycles(wxGetApp().m_global.m_procType);
}

const uint8_t (&CAsm::CodeToCycles(const ProcessorType procType))[0x100]
{
    switch (procType)
    {
    case ProcessorType::M6502:
        return code_cycles;

    case ProcessorType::WDC65C02:
        return code_cycles_c;

    case ProcessorType::WDC65816:
        return code_cycles_8;

    default:
        throw 0; // TODO: Throw real exception later.
    }
}
