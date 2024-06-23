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

typedef unsigned int UINT32;		// liczba ca�kowita 32-bitowa bez znaku
typedef signed int SINT32;		// liczba ca�kowita 32-bitowa ze znakiem
typedef unsigned char UINT8;		// liczba ca�kowita 8-bitowa bez znaku
typedef unsigned short int UINT16;	// liczba ca�kowita 16-bitowa bez znaku

enum class ProcessorType
{
    M6502 = 0, // Basic 6502
    WDC65C02 = 1, // 65C02, 6501
    WDC65816 = 2, // 65816
};

class CAsm	  // klasa bazowa - definicje typ�w
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
        STAT_USER_DEF_ERR,			// b��d u�ytkownika (.ERROR)
        STAT_FIN,
        OK = 0,
        ERR_DAT,					// nieooczekiwane wyst�pienie danych (tu tylko komentarz)
        ERR_UNEXP_DAT,				// nierozpoznany napis (na pocz�tku wiersza)
        ERR_OUT_OF_MEM,
        ERR_FILE_READ,
        ERR_NUM_LONG,				// oczekiwana liczba max $FFFF
        ERR_NUM_NOT_BYTE,			// oczekiwana liczba max $FF
        ERR_NUM_NEGATIVE,			// oczekiwana warto�� nieujemna
        ERR_INSTR_OR_NULL_EXPECTED,	// oczekiwana instrukcja, komentarz lub CR
        ERR_IDX_REG_EXPECTED,		// oczekiwany rejestr indeksowy (X lub Y)
        ERR_IDX_REG_X_EXPECTED,		// oczekiwany rejestr indeksowy X
        ERR_IDX_REG_Y_EXPECTED,		// oczekiwany rejestr indeksowy Y
        ERR_COMMA_OR_BRACKET_EXPECTED,	// oczekiwany przecinek lub nawias ')'
        ERR_BRACKET_R_EXPECTED,		// oczekiwany nawias ')'
        ERR_BRACKET_L_EXPECTED,		// oczekiwany nawias '('
        ERR_DIV_BY_ZERO,			// dzielenie przez zero w wyra�eniu
        ERR_EXPR_BRACKET_R_EXPECTED,// brak nawiasu ']' zamykaj�cego wyra�enie
        ERR_CONST_EXPECTED,			// oczekiwana sta�a (liczba lub ident)
        ERR_LABEL_REDEF,			// etykieta ju� zdefiniowana
        ERR_UNDEF_EXPR,				// nieokre�lona warto�� wyra�enia
        ERR_PC_WRAPED,				// "przewini�cie si�" licznika rozkaz�w
        ERR_UNDEF_LABEL,			// niezdefiniowana etykieta
        ERR_PHASE,					// b��d fazy - niezgodne warto�ci etykiety mi�dzy przebiegami
        ERR_REL_OUT_OF_RNG,			// przekroczenie zakresu dla adresowania wzgl�dnego
        ERR_MODE_NOT_ALLOWED,		// niedozwolony tryb adresowania
        ERR_STR_EXPECTED,			// oczekiwany �a�cuch znak�w
        ERR_SPURIOUS_ENDIF,			// wyst�pienie .ENDIF bez odpowiadaj�cego mu .IF
        ERR_SPURIOUS_ELSE,			// wyst�pienie .ELSE bez odpowiadaj�cego mu .IF
        ERR_ENDIF_REQUIRED,			// brak dyrektywy .ENDIF
        ERR_LOCAL_LABEL_NOT_ALLOWED,// wymagane jest wyst�pienie etykiety globalnej
        ERR_LABEL_EXPECTED,			// wymagana etykieta
        ERR_USER_ABORT,				// u�ytkownik przerwa� asemblacj�
        ERR_UNDEF_ORIGIN,			// brak dyrektywy .ORG
        ERR_MACRONAME_REQUIRED,		// brak etykiety nazywaj�cej makrodefinicj�
        ERR_PARAM_ID_REDEF,			// nazwa parametru ju� zdefiniowana
        ERR_NESTED_MACRO,			// definicja makra w makrodefinicji jest zabroniona
        ERR_ENDM_REQUIRED,			// brak dyrektywy .ENDM
        ERR_UNKNOWN_INSTR,			// nierozpoznana nazwa makra/instrukcji/dyrektywy
        ERR_PARAM_REQUIRED,			// brak wymaganej ilo�ci parametr�w wywo�ania makra
        ERR_SPURIOUS_ENDM,			// wyst�pienie .ENDM bez odpowiadaj�cego mu .MACRO
        ERR_SPURIOUS_EXITM,			// wyst�pienie .EXIT poza makrodefinicj� jest niedozwolone
        ERR_STR_NOT_ALLOWED,		// wyra�enie znakowe niedozwolone
        ERR_NOT_STR_PARAM,			// parametr wo�any z '$' nie posiada warto�ci typu tekstowego
        ERR_EMPTY_PARAM,			// wo�any parametr nie istnieje (za du�y nr przy odwo�aniu: %num)
        ERR_UNDEF_PARAM_NUMBER,		// numer parametru w wywo�aniu "%number" jest niezdefiniowany
        ERR_BAD_MACRONAME,			// nazwa makra nie mo�e zaczyna� si� od znaku '.'
        ERR_PARAM_NUMBER_EXPECTED,	// oczekiwany numer parametru makra
        ERR_LABEL_NOT_ALLOWED,		// niedozwolone wyst�pienie etykiety (przed dyrektyw�)
        ERR_BAD_REPT_NUM,			// b��dna ilo�� powt�rze� (dozwolone od 0 do 0xFFFF)
        ERR_SPURIOUS_ENDR,			// wyst�pienie .ENDR bez odpowiadaj�cego mu .REPEAT
        ERR_INCLUDE_NOT_ALLOWED,	// dyrektywa .INCLUDE nie mo�e wyst�powa� w makrach i powt�rkach
        ERR_STRING_TOO_LONG,		// za d�ugi �a�cuch (w .STR)
        ERR_NOT_BIT_NUM,			// oczekiwana liczba od 0 do 7 (numer biru)
        ERR_OPT_NAME_REQUIRED,		// brak nazwy opcji
        ERR_OPT_NAME_UNKNOWN,		// nierozpoznana nazwa opcji
        ERR_LINE_TO_LONG,			// za d�ugi wiersz �r�d�owy
        ERR_PARAM_DEF_REQUIRED,		// wymagana nazwa parametru makra
        ERR_INDIRECT_BYTE_EXPECTED,	// Indirect postindexed addressing mode requires byte operand
        ERR_CONST_LABEL_REDEF,		// Constant label (predefined) cannot be redefined
        ERR_NO_RANGE,				// expected valid range: first value has to be less than or equal to snd value
        ERR_NUM_ZERO,				// expected nonzero value
        ERR_INVALID_BANK_CROSSING,  // branch, subroutine, jump, or execution crossed a bank boundary
        ERR_MACRO_PARAM_COUNT,		// Macro parameter count exceeds expected count
        ERR_DEBUG,					// debugging error
        //		ERR_PARAMTYPE_NOT_IN_MACRO,	// .PARAMTYPE can only be used inside macro
        ERR_LAST					// ostatnia warto�� typu Stat - nie b��d
    };

    enum OpCode		// rodzaj instrukcji procesora
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
        // nowe rozkazy z 6501
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
        C_ILL		// warto�� do oznaczania nielegalnych rozkaz�w w symulatorze (ILLEGAL)
        // jednocze�nie warto�� maksymalna dla typu OpCode
    };

    enum CodeAdr	// tryby adresowania
    {
        A_IMP,		// implied
        A_ACC,		// accumulator
        A_IMM,		// immediate
        A_ZPG,		// zero page
        A_ABS,		// absolute
        A_ABS_X,	// absolute indexed X
        A_ABS_Y,	// absolute indexed Y
        A_ZPG_X,	// zero page indexed X
        A_ZPG_Y,	// zero page indexed Y
        A_REL,		// relative
        A_ZPGI,		// zero page indirect
        A_ZPGI_X,	// zero page indirect, indexed X
        A_ZPGI_Y,	// zero page indirect, indexed Y
        A_ABSI,		// absolute indirect
        A_ABSI_X,	// absolute indirect, indexed X
        A_ZREL,		// zero page / relative -> BBS i BBR z 6501
        A_ZPG2,		// zero page dla rozkaz�w RMB SMB z 6501
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
        A_NO_OF_MODES,	// ilo�� tryb�w adresowania

        A_ABS_OR_ZPG = A_NO_OF_MODES,	// niezdeterminowany tryb adresowania
        A_ABSX_OR_ZPGX,
        A_ABSY_OR_ZPGY,
        A_ABSI_OR_ZPGI,
        A_IMP_OR_ACC,
        A_ABSIX_OR_ZPGIX,

        A_IMP2,	// implied dla rozkazu BRK

        A_ILL		// warto�� do oznaczania nielegalnych rozkaz�w w symulatorze (ILLEGAL)
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

    static const TCHAR LOCAL_LABEL_CHAR;	// znak rozpoczynaj�cy etykiet� lokaln�
    static const TCHAR MULTIPARAM[];		// wielokropek - dowolna ilo�� parametr�w

    enum Msg
    {
        WM_USER_GET_NEXT_LINE = WM_USER + 4000, // pobranie/udost�pnienie kolejnego wiersza
        WM_USER_GET_LINE_NO,		// pobranie numeru aktualnego wiersza
        WM_USER_FIN,				// poinformowanie o zako�czeniu asemblacji
        WM_USER_GET_TITLE,			// odczyt nazwy dokumentu
        WM_USER_NEXT_PASS,			// nast�pny przebieg asemblacji
        WM_USER_ABORT_ASM			// przerwanie asemblacji
    };

    enum SymStat
    {
        SYM_OK = 0,
        SYM_BPT_EXECUTE,		// przerwanie przy wykonaniu
        SYM_BPT_READ,			// przerwanie przy odczycie
        SYM_BPT_WRITE,			// przerwanie przy zapisie
        SYM_BPT_TEMP,			// przerwanie przy wykonaniu
        SYM_ILLEGAL_CODE,		// napotkana nielegalna instrukcja
        SYM_STOP,				// program zatrzymany przez u�ytkownika
        SYM_FIN,				// program zako�czony
        SYM_RUN,				// program uruchomiony
        SYM_INP_WAIT,			// waiting for data input
        SYM_ILL_WRITE			// protected area writing attempt detected
    };

    enum Breakpoint
    {
        BPT_NONE = 0x00,		// nie ma przerwania
        BPT_EXECUTE = 0x01,		// przerwanie przy wykonaniu
        BPT_READ = 0x02,		// przerwanie przy odczycie
        BPT_WRITE = 0x04,		// przerwanie przy zapisie
        BPT_MASK = 0x07,
        BPT_NO_CODE = 0x08,		// wiersz nie zawiera kodu - przerwanie nie mo�e by� ustawione
        BPT_TEMP_EXEC = 0x10,		// przerwanie tymczasowe do zatrzymania programu
        BPT_DISABLED = 0x80		// przerwanie wy��czone
    };

    enum Finish				// spos�b zako�czenia programu
    {
        FIN_BY_BRK,			// po napotkaniu instrukcji BRK
        FIN_BY_RTS,			// po napotkaniu RTS przy pustym stosie
        FIN_BY_DB			// po napotkaniu kodu $BB
    };

    enum DeasmFmt				// format deasemblacji
    {
        DF_NONE = 0x00,
        DF_ADDRESS = 0x01,	// adres przed instrukcj�
        DF_CODE_BYTES = 0x02,	// przed instrukcj� jej kod i argumenty
        DF_BRANCH_INFO = 0x04,	// za rozkazami skok�w warunkowych info czy skok aktywny
        DF_LABELS = 0x08,	// adresy argument�w poprzedzone liter� 'a' lub 'z'
        DF_HELP = 0x10,	// generate output suitable for dynamic help
        DF_USE_BRK = 0x20,	// generate BRK instead of .DB 0
        DF_65C02 = 0x40,	// use 65c02 instead of current settings
        DF_65816 = 0x80    // use 65816 instead of current settings
    };

    //............................... debug info ................................

    typedef UINT16 FileUID;

    enum DbgFlag
    {
        DBG_EMPTY = 0,
        DBG_CODE = 1,
        DBG_DATA = 2,
        DBG_MACRO = 4,
    };

    //---------------------------------------------------------------------------
protected:
    static const UINT8 code_to_mode[];			// zamiana kodu rozkazu na tryb adresowania
    static const UINT8 code_to_mode_c[];		// j.w. dla 65c02
    static const UINT8 code_to_mode_8[];		// j.w. dla 65c02
    static const UINT8 code_to_command[];		// zamiana kodu rozkazu na jego numer (typu OpCode)
    static const UINT8 code_to_command_c[];		// j.w. dla 65c02
    static const UINT8 code_to_command_8[];		// j.w. dla 65c02
    static const UINT8 trans_table[][A_NO_OF_MODES];	// zamiana rozkazu w danym trybie adresowania na kod (65XX)
    static const UINT8 trans_table_c[][A_NO_OF_MODES];	// zamiana rozkazu w danym trybie adresowania na kod (65C02)
    static const UINT8 trans_table_8[][A_NO_OF_MODES];	// zamiana rozkazu w danym trybie adresowania na kod (65C02)
    static const UINT8 code_cycles[];			// ilo�� cykli dla rozkazu procesora
    static const UINT8 code_cycles_c[];			// j.w. dla 65c02
    static const UINT8 code_cycles_8[];			// j.w. dla 65c02

public:
    static const UINT8 mode_to_len[];	// zamiana trybu adresowania na d�ugo�� rozkazu i argument�w

    static const UINT8(&TransformTable(const ProcessorType procType))[C_ILL][A_NO_OF_MODES];

    static const UINT8(&CodeToCommand(const ProcessorType procType))[0x100];
    static const UINT8(&CodeToCommand())[0x100];

    static const UINT8(&CodeToMode(const ProcessorType procType))[0x100];
    static const UINT8(&CodeToMode())[0x100];

    static const UINT8(&CodeToCycles(const ProcessorType procType))[0x100];
    static const UINT8(&CodeToCycles())[0x100];

    static ProcessorType ProcType();
};


CAsm::DeasmFmt inline operator | (CAsm::DeasmFmt f1, CAsm::DeasmFmt f2)
{
    return static_cast<CAsm::DeasmFmt>(DWORD(f1) | DWORD(f2));
}


#endif
