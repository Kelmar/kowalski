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

#ifndef _asm_h_
#define _asm_h_

/**
 * @brief Type of CPU we're emulating.
 * @remark This is temporary until we refactor the simulator into a more generic
 * system where the CPU type is more flexable.
 */
enum class ProcessorType
{
    M6502 = 0,    // Basic 6502
    WDC65C02 = 1, // 65C02, 6501
    WDC65816 = 2  // 65816
};

// Being used more like a namespace than a class? -- B.Simonds (April 25, 2024)

class CAsm	  // base class - type definitions
{
public:

    enum Stat
    {
        STAT_INCLUDE = -999,
        STAT_REPEAT,
        STAT_ENDR,
        STAT_MACRO,
        STAT_ENDM,
        STAT_EXITM,
        STAT_IF_TRUE,
        STAT_IF_FALSE,
        STAT_IF_UNDETERMINED,
        STAT_ELSE,
        STAT_ENDIF,
        STAT_ASM,
        STAT_SKIP,
        STAT_USER_DEF_ERR,			// user error (.ERROR)
        STAT_FIN,
        
        OK = 0,

        ERR_DAT,                    // Unexpected data occurrence (just a comment here)
        ERR_UNEXP_DAT,              // Unrecognized string (at the beginning of the line)
        ERR_OUT_OF_MEM,
        ERR_FILE_READ,
        ERR_NUM_LONG,               // Expected number max $FFFF
        ERR_NUM_NOT_BYTE,           // Expected number max $FF
        ERR_NUM_NEGATIVE,           // Expected non-negative value
        ERR_INSTR_OR_NULL_EXPECTED, // Expected statement, comment or CR
        ERR_IDX_REG_EXPECTED,       // Expected index register (X or Y)
        ERR_IDX_REG_X_EXPECTED,     // Expected X index register
        ERR_IDX_REG_Y_EXPECTED,     // Expected Y index register
        ERR_COMMA_OR_BRACKET_EXPECTED,// Expected comma or bracket ')'
        ERR_BRACKET_R_EXPECTED,     // Expected bracket ')'
        ERR_BRACKET_L_EXPECTED,     // Expected bracket '('
        ERR_DIV_BY_ZERO,            // Divide by zero in the expression
        ERR_EXPR_BRACKET_R_EXPECTED,// Missing bracket ']' ending the expression
        ERR_CONST_EXPECTED,         // Expected constant (number or ident)
        ERR_LABEL_REDEF,            // Label already defined
        ERR_UNDEF_EXPR,             // Undefined expression value
        ERR_PC_WRAPED,              // "scroll" the command counter
        ERR_UNDEF_LABEL,            // Undefined label
        ERR_PHASE,                  // Phase error -inconsistent label values ​​between runs
        ERR_REL_OUT_OF_RNG,         // Range exceeded for relative addressing
        ERR_MODE_NOT_ALLOWED,       // Addressing mode not allowed
        ERR_STR_EXPECTED,           // Expected string
        ERR_SPURIOUS_ENDIF,         // An occurrence of .ENDIF without a corresponding .IF
        ERR_SPURIOUS_ELSE,          // An instance of .ELSE without a corresponding .IF
        ERR_ENDIF_REQUIRED,         // No .ENDIF directive
        ERR_LOCAL_LABEL_NOT_ALLOWED,// A global label is required
        ERR_LABEL_EXPECTED,         // Required label
        ERR_USER_ABORT,             // The user has interrupted the assembly
        ERR_UNDEF_ORIGIN,           // No .ORG directive
        ERR_MACRONAME_REQUIRED,     // No label naming the macrodefinition
        ERR_PARAM_ID_REDEF,         // Parameter name already defined
        ERR_NESTED_MACRO,           // Macro definition within a macro definition is prohibited
        ERR_ENDM_REQUIRED,          // Missing .ENDM directive
        ERR_UNKNOWN_INSTR,          // Unrecognized macro/instruction/directive name
        ERR_PARAM_REQUIRED,         // Missing the required number of macro calling parameters
        ERR_SPURIOUS_ENDM,          // An occurrence of .ENDM without a corresponding .MACRO
        ERR_SPURIOUS_EXITM,         // .EXIT outside the macro definition is not allowed
        ERR_STR_NOT_ALLOWED,        // Expression characters not allowed
        ERR_NOT_STR_PARAM,          // The parameter called from '$' does not have a text type value
        ERR_EMPTY_PARAM,            // The requested parameter does not exist (too large reference number: %num)
        ERR_UNDEF_PARAM_NUMBER,     // The parameter number in the "%number" call is undefined
        ERR_BAD_MACRONAME,          // Macro name cannot start with '.'
        ERR_PARAM_NUMBER_EXPECTED,  // Expected macro parameter number
        ERR_LABEL_NOT_ALLOWED,      // Illegal label occurrence (before directive)
        ERR_BAD_REPT_NUM,           // Invalid number of repeats (allowed from 0 to 0xFFFF)
        ERR_SPURIOUS_ENDR,          // An occurrence of .ENDR without a corresponding .REPEAT
        ERR_INCLUDE_NOT_ALLOWED,    // The .INCLUDE directive cannot appear in marks and repeats
        ERR_STRING_TOO_LONG,        // String too long (in .STR)
        ERR_NOT_BIT_NUM,            // Expected number from 0 to 7 (bit number)
        ERR_OPT_NAME_REQUIRED,      // No option name
        ERR_OPT_NAME_UNKNOWN,       // Unrecognized option name
        ERR_LINE_TO_LONG,           // Source line too long
        ERR_PARAM_DEF_REQUIRED,     // Required macro parameter name
        ERR_INDIRECT_BYTE_EXPECTED,	// Indirect postindexed addressing mode requires byte operand
        ERR_CONST_LABEL_REDEF,		// Constant label (predefined) cannot be redefined
        ERR_NO_RANGE,				// Expected valid range: first value has to be less than or equal to snd value
        ERR_NUM_ZERO,				// Expected nonzero value
        ERR_INVALID_BANK_CROSSING,  // Branch, subroutine, jump, or execution crossed a bank boundary
        ERR_MACRO_PARAM_COUNT,		// Macro parameter count exceeds expected count
        ERR_DEBUG,					// Debugging error
//		ERR_PARAMTYPE_NOT_IN_MACRO,	// .PARAMTYPE can only be used inside macro

        ERR_LAST					// Last value of type Stat -no error
    };

    /// @brief Processor instruction type
    enum OpCode		
    {
        C_LDA, C_LDX, C_LDY,
        C_STA, C_STX, C_STY, C_STZ,
        C_TAX, C_TXA, C_TAY, C_TYA, C_TXS, C_TSX,
        C_ADC, C_SBC, C_CMP, C_CPX, C_CPY,
        C_INC, C_DEC, C_INA, C_DEA, C_INX, C_DEX, C_INY, C_DEY,
        C_ASL, C_LSR, C_ROL, C_ROR,
        C_AND, C_ORA, C_EOR,
        C_BIT, C_TSB, C_TRB,
        C_JMP, C_JSR, C_BRK,
        C_BRA, C_BPL, C_BMI, C_BVC, C_BVS, C_BCC, C_BCS, C_BNE, C_BEQ,
        C_RTS, C_RTI,
        C_PHA, C_PLA, C_PHX, C_PLX, C_PHY, C_PLY, C_PHP, C_PLP,
        C_CLC, C_SEC, C_CLV, C_CLD, C_SED, C_CLI, C_SEI,
        C_NOP,
        
        // New op codes from 6501
        C_BBR, //C_BBR1, C_BBR2, C_BBR3, C_BBR4, C_BBR5, C_BBR6, C_BBR7,
        C_BBS, //C_BBS1, C_BBS2, C_BBS3, C_BBS4, C_BBS5, C_BBS6, C_BBS7,
        C_RMB, //C_RMB1, C_RMB2, C_RMB3, C_RMB4, C_RMB5, C_RMB6, C_RMB7,
        C_SMB, //C_SMB1, C_SMB2, C_SMB3, C_SMB4, C_SMB5, C_SMB6, C_SMB7,

        // 65816
        C_BRL,
        C_COP,
        C_JML,
        C_JSL,
        C_MVN,
        C_MVP,
        C_PEA,
        C_PEI,
        C_PER,
        C_PHB,
        C_PHD,
        C_PHK,
        C_PLB,
        C_PLD,
        C_REP,
        C_RTL,
        C_SEP,
        C_STP,
        C_TCD,
        C_TCS,
        C_TDC,
        C_TSC,
        C_TXY,
        C_TYX,
        C_WAI,
        C_WDM,
        C_XBA,
        C_XCE,

        C_ILL   // Value for marking illegal commands in the simulator (ILLEGAL)
        // Also the maximum value for the OpCode type
    };

    /// @brief Addressing modes
    enum CodeAdr
    {
        A_IMP,      // implied
        A_ACC,      // accumulator
        A_IMM,      // immediate
        A_ZPG,      // zero page
        A_ABS,      // absolute
        A_ABS_X,    // absolute indexed X
        A_ABS_Y,    // absolute indexed Y
        A_ZPG_X,    // zero page indexed X
        A_ZPG_Y,    // zero page indexed Y
        A_REL,      // relative
        A_ZPGI,     // zero page indirect
        A_ZPGI_X,   // zero page indirect, indexed X
        A_ZPGI_Y,   // zero page indirect, indexed Y
        A_ABSI,     // absolute indirect
        A_ABSI_X,   // absolute indirect, indexed X
        A_ZREL,     // zero page / relative -> BBS i BBR z 6501
        A_ZPG2,     // zero page for RMB SMB commands from 6501
        A_ABSL,
        A_ABSL_X,
        A_ZPIL,
        A_ZPIL_Y,
        A_INDL,     // Absolute Indirect Long (JMP or JML)
        A_SR,
        A_SRI_Y,
        A_RELL,
        A_XYC,
        A_IMM2,
        A_NO_OF_MODES, // Number of addressing modes

        A_ABS_OR_ZPG = A_NO_OF_MODES,   // Undetermined addressing mode
        A_ABSX_OR_ZPGX,
        A_ABSY_OR_ZPGY,
        A_ABSI_OR_ZPGI,
        A_IMP_OR_ACC,
        A_ABSIX_OR_ZPGIX,

        A_IMP2, // Implied for the BRK command

        A_ILL   // value for marking illegal commands in the simulator (ILLEGAL)
    };

    enum InstrType	// rodzaj dyrektywy asemblera
    {
        I_ORG,		// origin
        I_DB,		// def byte
        I_DW,		// def word
        I_DD,		// def double byte
        I_DX,		// def 24 bit number
        I_DDW,		// def 32 bit number
        I_DS,		// def string
        I_LS,		// def long string
        I_ASCIS,	// ascii + $80
        I_DCB,		// declare block
        I_RS,		// reserve space
        I_END,		// zako�cz asemblacj�
        I_ERROR,	// zg�oszenie b��du
        I_INCLUDE,	// w��czenie pliku
        I_IF,		// asemblacja warunkowa
        I_ELSE,
        I_ENDIF,
        I_MACRO,	// makrodefinicja
        I_EXITM,
        I_ENDM,
        I_START,	// pocz�tek programu dla symulatora
        I_SET,		// przypisanie warto�ci
        I_REPEAT,	// powt�rka
        I_ENDR,
        I_OPT,		// opcje asemblera
        I_ROM_AREA,	// protected memory area
        I_IO_WND,	// size of I/O window (columns, rows)
        I_DATE,		// date insert
        I_TIME		// time insert
    };

    enum OperType		// typ operatora
    {
        O_HI, O_LO,
        O_B_AND, O_B_OR, O_B_XOR, O_B_NOT,
        O_PLUS, O_MINUS, O_DIV, O_MUL, O_MOD,
        O_AND, O_OR, O_NOT,
        O_EQ, O_NE, O_GT, O_LT, O_GTE, O_LTE,
        O_SHL, O_SHR
    };

    static const char LOCAL_LABEL_CHAR; // Sign that starts the local tag
    static const char MULTIPARAM[];     // Ellipsis - Any number of parameters

#if 0
    // TODO: Replace with wx events. -- B.Simonds (April 25, 2024)

    enum Msg
    {
        WM_USER_GET_NEXT_LINE = WM_USER + 4000, // Get/share the next line
        WM_USER_GET_LINE_NO,    // Get the current line number
        WM_USER_FIN,            // Informing about the completion of assembly
        WM_USER_GET_TITLE,      // Read document name
        WM_USER_NEXT_PASS,      // Next assembly pass
        WM_USER_ABORT_ASM	    // Interruption of assembly
    };
#endif

    /// @brief Simulator Status
    enum SymStat
    {
        SYM_OK = 0,
        SYM_BPT_EXECUTE,  // Interrupt during execution
        SYM_BPT_READ,     // Interrupt on reading
        SYM_BPT_WRITE,    // Interrupt on write
        SYM_BPT_TEMP,     // Interrupt during execution
        SYM_ILLEGAL_CODE, // Illegal statement encountered
        SYM_STOP,         // Program stopped by user
        SYM_FIN,          // Program finished
        SYM_RUN,          // Program started
        SYM_INP_WAIT,     // Waiting for data input
        SYM_ILL_WRITE     // Protected area writing attempt detected
    };

    enum Breakpoint
    {
        BPT_NONE      = 0x00, // there is no interrupt
        BPT_EXECUTE   = 0x01, // interrupt during execution
        BPT_READ      = 0x02, // interrupt on reading
        BPT_WRITE     = 0x04, // interrupt on write
        BPT_MASK      = 0x07,
        BPT_NO_CODE   = 0x08, // line contains no code -interrupt cannot be set
        BPT_TEMP_EXEC = 0x10, // temporary interrupt to stop the program
        BPT_DISABLED  = 0x80  // interrupt disabled
    };

    enum Finish // How to end the program
    {
        FIN_BY_BRK, // When a BRK instruction is encountered
        FIN_BY_RTS, // When RTS is encountered with an empty stack
        FIN_BY_DB   // when $BB is encountered
    };

    /// @brief Disassembly format
    enum DeasmFmt
    {
        DF_NONE         = 0x00,
        DF_ADDRESS      = 0x01, // Address before the statement
        DF_CODE_BYTES   = 0x02, // Before the instruction its code and arguments
        DF_BRANCH_INFO  = 0x04, // After the conditional jump commands, info whether the jump is active
        DF_LABELS       = 0x08, // Argument addresses preceded by the letter 'a' or 'z'
        DF_HELP         = 0x10, // generate output suitable for dynamic help
        DF_USE_BRK      = 0x20, // generate BRK instead of .DB 0
        DF_65C02        = 0x40, // use 65c02 instead of current settings
        DF_65816        = 0x80  // use 65816 instead of current settings
    };

    //............................... debug info ................................

    typedef uint16_t FileUID;

    enum DbgFlag
    {
        DBG_EMPTY	= 0,
        DBG_CODE	= 1,
        DBG_DATA	= 2,
        DBG_MACRO	= 4,
    };

    //---------------------------------------------------------------------------
protected:
    static const uint8_t code_to_mode[];                 // Converting the command code to the addressing mode
    static const uint8_t code_to_mode_c[];               // As above for 65c02
    static const uint8_t code_to_mode_8[];               // As above for 65816
    static const uint8_t code_to_command[];              // Replacing the command code with its number (OpCode type)
    static const uint8_t code_to_command_c[];            // As above for 65c02
    static const uint8_t code_to_command_8[];            // As above for 65816
    static const uint8_t trans_table[][A_NO_OF_MODES];   // Converting an instruction in a given addressing mode into a code (65XX)
    static const uint8_t trans_table_c[][A_NO_OF_MODES]; // Converting an instruction in a given addressing mode into a code (65C02)
    static const uint8_t trans_table_8[][A_NO_OF_MODES]; // Converting an instruction in a given addressing mode into a code (65816)
    static const uint8_t code_cycles[];                  // How many cycles for instruction
    static const uint8_t code_cycles_c[];                // As above for 65c02
    static const uint8_t code_cycles_8[];                // As above for 65816

public:
    static const uint8_t mode_to_len[]; // Changing the addressing mode to the length of the instruction and arguments

    const uint8_t (&TransformTable(const ProcessorType procType))[C_ILL][A_NO_OF_MODES];

    const uint8_t (&CodeToCommand(const ProcessorType procType))[0x100];
    const uint8_t (&CodeToCommand())[0x100];

    const uint8_t (&CodeToMode(const ProcessorType procType))[0x100];
    const uint8_t (&CodeToMode())[0x100];

    const uint8_t (&CodeToCycles(const ProcessorType procType))[0x100];
    const uint8_t (&CodeToCycles())[0x100];

    ProcessorType ProcType();
};

CAsm::DeasmFmt inline operator | (CAsm::DeasmFmt f1, CAsm::DeasmFmt f2)
{
    return static_cast<CAsm::DeasmFmt>(uint32_t(f1) | uint32_t(f2));
}

#endif
