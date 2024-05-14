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
#include "6502View.h"
#include "Deasm.h"

static const char * s_vpszInstructionList[] =
{
    "LDA",
    "LDX",
    "LDY",
    "STA",
    "STX",
    "STY",
    "STZ",
    "TAX",
    "TXA",
    "TAY",
    "TYA",
    "TXS",
    "TSX",
    "ADC",
    "SBC",
    "CMP",
    "CPX",
    "CPY",
    "INC",
    "DEC",
    "INA",
    "DEA",
    "INX",
    "DEX",
    "INY",
    "DEY",
    "ASL",
    "LSR",
    "ROL",
    "ROR",
    "AND",
    "ORA",
    "EOR",
    "BIT",
    "TSB",
    "TRB",
    "JMP",
    "JSR",
    "BRK",
    "BRA",
    "BPL",
    "BMI",
    "BVC",
    "BVS",
    "BCC",
    "BCS",
    "BNE",
    "BEQ",
    "RTS",
    "RTI",
    "PHA",
    "PLA",
    "PHX",
    "PLX",
    "PHY",
    "PLY",
    "PHP",
    "PLP",
    "CLC",
    "SEC",
    "CLV",
    "CLD",
    "SED",
    "CLI",
    "SEI",
    "NOP",
    "BBR",
    "BBS",
    "RMB",
    "SMB",
    "BRL",
    "COP",
    "JML",
    "JSL",
    "MVN",
    "MVP",
    "PEA",
    "PEI",
    "PER",
    "PHB",
    "PHD",
    "PHK",
    "PLB",
    "PLD",
    "REP",
    "RTL",
    "SEP",
    "STP",
    "TCD",
    "TCS",
    "TDC",
    "TSC",
    "TXY",
    "TYX",
    "WAI",
    "WDM",
    "XBA",
    "XCE",
    NULL
};

//% Bug Fix 1.2.13.18 - .commands commented out
static const char * s_vpszDirectiveList[]=
{
    ".ASCII",
    ".ASCIS",
    ".BYTE",
    ".DATE",
    ".DB",
    ".DBYTE",
    ".DCB",
    ".DD",
    ".DDW",
    ".DS",
    ".DW",
    ".uint32_t",
    ".DX",
    ".ELSE",
    ".END",
    ".ENDIF",
    ".ENDM",
    ".ENDR",
    ".ERROR",
    ".EXITM",
    ".IF",
    ".INCLUDE",
    ".IO_WND",
    ".LSTR",
    ".LSTRING",
    ".MACRO",
    ".OPT",
    ".ORG",
    ".REPEAT",
    ".REPT",
    ".ROM_AREA",
    ".RS",
    ".SET",
    ".START",
    ".STR",
    ".STRING",
    ".TIME",
    ".WORD",
    ".XWORD",
    ".PARAMTYPE",
    ".STRLEN",
    ".REF",
    ".DEF",
    ".PASSDEF",
    NULL
};

static const char * s_vpszConstantList[]=
{
    "ORG",
    "IO_AREA",
    0
};

std::string makeUpper(const std::string &str)
{
    if (str.empty())
        return "";

    std::string rval;
    rval.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i)
        rval += std::toupper(str[i]);

    return rval;
}

bool IsMnemonic(const std::string &chars)
{
    if (chars.size() != 3)
        return false;

    std::string charsUpper = makeUpper(chars);
    
    for (int i = 0; s_vpszInstructionList[i] != NULL; ++i)
    {
        if (charsUpper == s_vpszInstructionList[i])
            return true;
    }

    return false;
}

bool IsDirective(const std::string &chars)
{
    if (chars.size() < 2)
        return false;

    std::string charsUpper = makeUpper(chars);
    
    if (chars[0] != '.')
    {
        for (int i = 0; s_vpszConstantList[i] != NULL; ++i)
        {
            if (charsUpper == s_vpszConstantList[i])
                return true;
        }

        return false;
    }

    for (int i = 0; s_vpszDirectiveList[i] != NULL; ++i)
    {
        if (charsUpper == s_vpszDirectiveList[i])
            return true;
    }

    return false;
}

#if 0
static bool Is6502Number(const std::string &chars)
{
    if (chars.size() > 1) // && (chars[0] == '$' || chars[0] == '@'))
    {
        if (chars[0] == '$')
        {
            for (int i = 1; i < chars.size(); ++i)
            {
                if (isdigit(chars[i]) || (chars[i] >= 'A' && chars[i] <= 'F') ||
                    (chars[i] >= 'a' && chars[i] <= 'f'))
                {
                    continue;
                }
                return false;
            }
        }
        else if (chars[0] == '@')	// binary?
        {
            for (int i = 1; i < chars.size(); ++i)
            {
                if (chars[i] == '0' || chars[i] == '1')
                    continue;
                return false;
            }
        }
        else
        {
            for (int i = 0; i < chars.size(); ++i)
            {
                if (isdigit(chars[i]))
                    continue;
                return false;
            }
        }
        return true;
    }

    if (isdigit(chars[0]))
        return true;

    return false;
}
#endif

#define DEFINE_BLOCK(pos, colorindex)	\
	ASSERT((pos) >= 0 && (pos) <= nLength);\
	if (pBuf != NULL)\
	{\
		if (nActualItems == 0 || pBuf[nActualItems - 1].m_nCharPos <= (pos)){\
		pBuf[nActualItems].m_nCharPos = (pos);\
		pBuf[nActualItems].m_nColorIndex = (colorindex);\
		nActualItems++;}\
	}

#define COOKIE_COMMENT      0x0001
#define COOKIE_PREPROCESSOR 0x0002
#define COOKIE_EXT_COMMENT  0x0004
#define COOKIE_STRING       0x0008
#define COOKIE_CHAR         0x0010

#if REWRITE_TO_WX_WIDGET

extern uint32_t ParseLine(uint32_t cookie, const std::string &chars, int nLength, TEXTBLOCK* pBuf, int& nActualItems)
{
    bool bFirstChar = (cookie & ~COOKIE_EXT_COMMENT) == 0;
    bool bRedefineBlock = true;
    bool bDecIndex = false;
    int nIdentBegin = -1;
    bool bSkipChar = false;

    int i = 0;
    for (i = 0; ; i++)
    {
        if (bRedefineBlock)
        {
            int nPos = i;

            if (bDecIndex)
                nPos--;

            if (cookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT))
            {
                DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
            }
            else if (cookie & (COOKIE_CHAR | COOKIE_STRING))
            {
                DEFINE_BLOCK(nPos, COLORINDEX_STRING);
            }
            else if (cookie & COOKIE_PREPROCESSOR)
            {
                DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
            }
            else
            {
                DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
            }

            bRedefineBlock = false;
            bDecIndex = false;
        }

        if (i == nLength)
            break;

        if (bSkipChar)
        {
            bSkipChar = false;
            continue;
        }

        if (cookie & COOKIE_COMMENT)
        {
            DEFINE_BLOCK(I, COLORINDEX_COMMENT);
            cookie |= COOKIE_COMMENT;
            break;
        }

        //	String constant "...."
        if (cookie & COOKIE_STRING)
        {
            if (chars[i] == '"' && (i == 0 || chars[i - 1] != '\\'))
            {
                cookie &= ~COOKIE_STRING;
                bRedefineBlock = TRUE;
            }
            continue;
        }

        //	Char constant '..'
        if (cookie & COOKIE_CHAR)
        {
            if (chars[i] == '\'' && (I == 0 || chars[i - 1] != '\\'))
            {
                cookie &= ~COOKIE_CHAR;
                bRedefineBlock = TRUE;
            }
            continue;
        }

        //	Extended comment /*....*/
        /*		if (cookie & COOKIE_EXT_COMMENT)
        		{
        			if (i > 0 && chars[i] == '/' && chars[i - 1] == '*')
        			{
        				cookie &= ~COOKIE_EXT_COMMENT;
        				bRedefineBlock = TRUE;
        			}
        			continue;
        		}
        */
        if (chars[i] == ';')
        {
            DEFINE_BLOCK(i, COLORINDEX_COMMENT);
            cookie |= COOKIE_COMMENT;
            continue;
        }
        /*
        		if (i > 0 && chars[I] == '/' && chars[i - 1] == '/')
        		{
        			DEFINE_BLOCK(i - 1, COLORINDEX_COMMENT);
        			cookie |= COOKIE_COMMENT;
        			continue;
        //			break;
        		}
        */
        /*		//	Preprocessor directive #....
        		if (cookie & COOKIE_PREPROCESSOR)
        		{
        			if (I > 0 && chars[I] == '*' && chars[I - 1] == '/')
        			{
        				DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
        				cookie |= COOKIE_EXT_COMMENT;

        				// MiK
        				bSkipChar = true;
        			}
        			continue;
        		}
        */
        //	Normal text
        if (chars[i] == '"')
        {
            DEFINE_BLOCK(i, COLORINDEX_STRING);
            cookie |= COOKIE_STRING;
            continue;
        }
        if (chars[i] == '\'')
        {
            DEFINE_BLOCK(i, COLORINDEX_STRING);
            cookie |= COOKIE_CHAR;
            continue;
        }
        /*		if (i > 0 && chars[I] == '*' && chars[i - 1] == '/')
        		{
        			DEFINE_BLOCK(i - 1, COLORINDEX_COMMENT);
        			cookie |= COOKIE_EXT_COMMENT;

        			// MiK
        			bSkipChar = true;

        			continue;
        		}
        */
        /*		if (bFirstChar)
        		{
        			if (chars[i] == '#')
        			{
        				DEFINE_BLOCK(I, COLORINDEX_PREPROCESSOR);
        				cookie |= COOKIE_PREPROCESSOR;
        				continue;
        			}
        			if (!isspace(chars[i]))
        				bFirstChar = FALSE;
        		} */

        /*		if (chars[i] == '+' || chars[i] == '-' || chars[i] == '*' || chars[i] == '/' ||
        			chars[i] == '#' || chars[i] == '>' || chars[i] == '<' || chars[i] == '(' || chars[i] == ')')
        		{
        			DEFINE_BLOCK(i, COLORINDEX_OPERATOR);
        			continue;
        		} */

        if (pBuf == NULL)
            continue; // We don't need to extract keywords,
        // for faster parsing skip the rest of loop

        if (isalnum(chars[i]) || chars[y] == '_' || chars[i] == '.' || chars[i] == '$' || chars[i] == '@')
        {
            if (nIdentBegin == -1)
                nIdentBegin = i;
        }
        else
        {
            if (nIdentBegin >= 0)
            {
                std::string sect = chars.substr(nIdentBegin, i - nIdentBegin);

                if (nIdentBegin > 0 && IsMnemonic(sect))
                {
                    DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
                }
                else if (IsDirective(sect))
                {
                    if (nIdentBegin > 0 || chars[0] != '.')
                    {
                        DEFINE_BLOCK(nIdentBegin, COLORINDEX_PREPROCESSOR);
                    }
                }
                else if (Is6502Number(sect))
                {
                    DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
                }
                bRedefineBlock = true;
                bDecIndex = true;
                nIdentBegin = -1;
            }
        }
    } // for (i = 0; ; i++)

    if (nIdentBegin >= 0)
    {
        std::string sect = chars.substr(nIdentBegin, i - nIdentBegin);

        if (nIdentBegin > 0 && IsMnemonic(sect))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
        }
        else if (nIdentBegin > 0 && IsDirective(sect))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_PREPROCESSOR);
        }
        else if (Is6502Number(sect))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
        }
    }

//	if (chars[nLength - 1] != '\\')
//		cookie &= COOKIE_EXT_COMMENT;
    return cookie;
}

uint32_t CSrc6502View::ParseLine(uint32_t cookie, int nLineIndex, TEXTBLOCK* pBuf, int& nActualItems)
{
    cookie = 0; // no history

    int nLength = GetLineLength(nLineIndex);
    if (nLength <= 0)
    {
        ClearCollapsibleBlockMark(nLineIndex);
        return 0; //cookie & COOKIE_EXT_COMMENT;
    }

    /*
    	if (chars[0] > ' ' && chars[0] != '.' && chars[0] != ';')
    	{
    		MarkCollapsibleBlockLine(nLineIndex, true);
    //		if (nLineIndex > 0)
    //			MarkCollapsibleBlockLine(nLineIndex - 1, false);
    	}
    	else
    		ClearCollapsibleBlockMark(nLineIndex);
    */

    std::string chars = GetLineChars(nLineIndex);

    return ::ParseLine(cookie, chars, nLength, pBuf, nActualItems);
}

#endif

///////////////////////////////////////////////////////////////////////////////

extern int MatchingDirectives(const std::string& strWord, std::string& strOut)
{
    UNUSED(strWord);
    UNUSED(strOut);

#if REWRITE_TO_WX_WIDGET
    strOut.clear();

    if (strWord.empty())
        return 0;

    int matchCnt = 0;

    if (strWord[0] == '.')
    {
        for (int i = 0; s_vpszDirectiveList[i] != 0; ++i)
        {
            if (_tcsnicmp(, strWord, strWord.GetLength()) == 0)
            {
                // whole name matches?
                if (strWord.size() == strlen(s_vpszDirectiveList[i]))
                {
                    strOut = s_vpszDirectiveList[i];
                    return 1;
                }

                strOut += s_vpszDirectiveList[i];
                strOut += "\n";
                ++matchCnt;
            }
        }
    }
    else
    {
        for (int i = 0; s_vpszConstantList[i] != 0; ++i)
        {
            if (_tcsnicmp(s_vpszConstantList[i], strWord, strWord.GetLength()) == 0)
            {
                // whole name matches?
                if (strWord.size() == strlen(s_vpszConstantList[i]))
                {
                    strOut = s_vpszConstantList[i];
                    return 1;
                }

                strOut += s_vpszConstantList[i];
                strOut += "\n";
                ++matchCnt;
            }
        }
    }

    if (matchCnt == 1)
        strOut.resize(strOut.size() - 1);

    return matchCnt;
#endif
    return 0;
}

extern std::string GetDirectiveDesc(const std::string& strDirective)
{
    std::string dirUpper;
    dirUpper.reserve(strDirective.size());

    for (size_t i = 0; i < strDirective.size(); ++i)
        dirUpper += std::toupper(strDirective[i]);

    if (dirUpper == ".ASCIS")
        return "#title#.ASCIS#text#\nDirective defining values of single bytes using string argument."
               " Last byte has toggled (XOR-ed) most significant bit to mark end of string."
               "#syntax#[<label>[:]] .ASCIS <expr> | txtexpr { , <expr> | txtexpr }.\n"
               "#exmpl#alpha: .ASCIS \"ABC\" ; generates bytes $41, $42, $C3.\n"
               "beta:  .ASCIS \"Stop\",$D ; generates bytes �S�, �t�, �o�, �p�, $8D\n"
               "#desc#.ASCIS directive is helpful to generate string with it's end marked by toggling most significant bit."
               " Printing subroutine for example can use this information to detect the end of string.\n";

    else if (dirUpper == ".ASCII" ||
             dirUpper == ".BYTE" ||
             dirUpper == ".DB")
        return "#title#.BYTE .DB .ASCII#text#\nDirectives defining values of single bytes using passed arguments."
               "#syntax#"
               "[<label>[:]] .DB <expr> | txtexpr { , <expr> | txtexpr }\n"
               "[<label>[:]] .BYTE <expr> | txtexpr { , <expr> | txtexpr }\n"
               "[<label>[:]] .ASCII <expr> | txtexpr { , <expr> | txtexpr }\n"
               "#exmpl#"
               "alpha: .DB \"ABC\", 0 ; generates bytes �A�, �B�, �C�, 0\n"
               "beta:  .DB %1, %1$  ; macro params; string length and string itself\n"
               " .BYTE <[alpha-1], >[alpha-1]\n"
               " .ASCII \"Text\"\n"
               "#desc#"
               ".BYTE (.DB, .ASCII) directives generates and defines single byte values. Input data might be entered in numerical or string form. Numerical expressions are also accepted.\n";

    else if (dirUpper == ".WORD" ||
             dirUpper == ".DW")
        return "#title#.WORD .DW#text#\nDirectives defining 16-bit word values using passed arguments."
               " Words are written according to low-endian 6502 convention: low byte first, high byte follows"
               "#syntax#"
               "[<label>[:]] .DW expr { , expr }\n"
               "[<label>[:]] .WORD expr { , expr }\n"
               "#exmpl#"
               "alpha: .DW $1234, 0 ; generates sequence $34, $12, 0, 0\n"
               "beta:  .WORD alpha\n"
               "       .WORD alpha-1, beta\n";

    else if (dirUpper == ".DBYTE" ||
             dirUpper == ".DD")
        return "#title#.DBYTE .DD#text#\nDirective defining double byte values."
               " Double bytes numbers are written according to big-endian convention: high byte first, low byte follows."
               "#syntax#"
               "[<label>[:]] .DD expr { , expr }\n"
               "[<label>[:]] .DBYTE expr { , expr }\n"
               "#exmpl#"
               "alpha: .DD $1234, 0 ; generates sequence $12, $34, 0, 0\n"
               "beta:  .DBYTE alpha\n"
               "       .DBYTE alpha-1, beta\n";

    else if (dirUpper == ".LSTRING" ||
             dirUpper == ".LSTR")
        return "#title#.LSTR .LSTRING#text#\nDirectives defining byte values using passed string argument."
               " First generated word contains string length in little-edian format. Remaining bytes are verbatim copies "
               "of string's characters. String length is limited to 1016 characters."
               "#syntax#"
               "[<label>[:]] .LSTR expr { , expr }\n"
               "[<label>[:]] .LSTRING expr { , expr }\n"
               "#exmpl#"
               "alpha: .LSTRING \"ABC\", $D ; generates sequence 4,0, �A�, �B�, �C�, $0D\n"
               "beta:  .LSTR \"Test string\", $D, $A ; generates sequence 13, 0, �T�,�e�,�s�,�t�,� �,�s�,�t�,�r�,�i�,�n�,�g�, $0D, $0A\n"
               "       .LSTR \"AB\", \"CD\", 13 ; generates: 5, 0, �A�, �B�, �C�, �D�, $0D\n";

    else if (dirUpper == ".STRING" ||
             dirUpper == ".STR")
        return "#title#.STR .STRING#text#\nDirectives defining byte values using passed string argument."
               " First generated byte contains string length. Remaining bytes are verbatim copies of string's characters. String length is limited to 255 characters."
               "#syntax#"
               "[<label>[:]] .STR expr { , expr }\n"
               "[<label>[:]] .STRING expr { , expr }\n"
               "#exmpl#"
               "alpha: .STRING \"ABC\", $D ; generates sequence 4, �A�, �B�, �C�, $0D\n"
               "beta:  .STR \"Test string\", $D, $A ; generates sequence 13, �T�,�e�,�s�,�t�,� �,�s�,�t�,�r�,�i�,�n�,�g�, $0D, $0A\n"
               "       .STR \"AB\", \"CD\", 13 ; generates: 5, �A�, �B�, �C�, �D�, $0D\n";

    else if (dirUpper == ".DCB")
        return "#title#.DCB#text#\nDirective reserving memory area with initialization of reserved memory by given value."
               "#syntax#"
               "[<label>[:]] .DCB count_bytes [ , init_val ]\n"
               "#exmpl#"
               "buf: .DCB $20,$FF  ; next $20 bytes is reserved and set to $FF\n"
               "#desc#"
               ".DCB directive reserves �count_bytes� and initializes them with �init_val� (if it's given).\n";

    else if (dirUpper == ".RS" ||
             dirUpper == ".DS")
        return "#title#.RS .DS#text#\nDirectives reserving memory area by adding given value to the pointer of current location. "
               "The memory area reserved is filled with the value of $00."
               "#syntax#"
               "[<label>[:]] .RS expr\n"
               "#exmpl#"
               "buf: .RS $100 ; reserve $100 bytes\n"
               "     .RS size ; same as *= * + size\n"
               "#desc#"
               ".RS and .DS move current origin (�*�, ORG) by specified amount of bytes.\n";

    else if (dirUpper == ".OPT")
        return "#title#.OPT#text#\nDirective setting assembly options. Available options are:\n\n"
               "\\sa100"
               "Proc6502\t select basic command set of 6502 microprocessor\n"
               "Proc65c02\t select extended command set of 65c02, 6501 and other microprocessors\n"
               "Proc6501\t ditto\n"
               "Proc65816\t select 65816 command set\n"
               "CaseSensitive\t treat lowercase and uppercase letters in label names as different\n"
               "CaseInsensitive\t treat lowercase and uppercase letters in label names as same\n"
               "SwapBin\t swap binary radix @ and modulo % operators - % now becomes binary radix and @ becomes modulo."
               "#syntax#"
               "  .OPT option_name { , option_name }\n"
               "#exmpl#"
               "  .OPT Proc65c02, CaseInsensitive\n";

    else if (dirUpper == ".ORG" ||
             dirUpper == "*=")
        return "#title#.ORG  *=#text#\nDirectives setting code generation location."
               "#syntax#"
               "[<label>[:]] .ORG expr\n"
               "#exmpl#"
               "    *= $1000 ; code location: $1000\n"
               "buf: .ORG * + $10 ; offset code location by $10 bytes,\n"
               " ; label �buf� will be set to previous code location address\n"
               "#desc#"
               "Use .ORG directive to designate beginning of an area where assembled code will be generated.\n";

    else if (dirUpper == ".START")
        return "#title#.START#text#\nDirective setting simulator entry (start point address)."
               "#syntax#"
               "  .START expr\n"
               "#exmpl#"
               "  .START Start ; start program execution from �Start� address\n"
               "  .START $A100 ; start program execution from $A100 address\n"
               "#desc#"
               ".START directive selects simulator entry point. Simulator will attempt to launch program using given address."
               " If there is no .START directive used, simulator tries to start execution using address specified by first .ORG directive."
               " .START directive allows using forward referencing (unlike .ORG).\n";

    else if (dirUpper == ".END")
        return "#title#.END#text#\nDirective finishing source code assembly."
               "#syntax#"
               "[<label>[:]] .END\n"
               "#exmpl#"
               "fin: .END  ; rest of source code in current file won't be assembled\n"
               "#desc#"
               ".END directive finishes assembly process of the file it was placed in. Used in main source file finishes assembly at the line it is used in.";

    else if (dirUpper == ".INCLUDE")
        return "#title#.INCLUDE#text#\nDirective including source file to assembly."
               "#syntax#"
               "  .INCLUDE file_name\n"
               "#exmpl#"
               "  .INCLUDE \"c:\\\\asm6502\\\\const_vals.65s\"\n"
               "  .INCLUDE \".\\\\macros\\\\macros\"\n"
               "#desc#"
               ".INCLUDE directive includes given source file. It's useful to include predefined labels or macros.";

    else if (dirUpper == ".MACRO")
        return "#title#.MACRO#text#\nDirective opening macro definition."
               "#syntax#"
               "<label>[:] .MACRO [param {, param} [, ...]] | [...]"
               "#exmpl#"
               "PushX  .MACRO      ; parameterless macro\n"
               "Print: .MACRO ...  ; macro accepting any number of params\n"
               "Put:   .MACRO chr  ; macro accepting exactly one parameter\n"
               ""
               "#desc#"
               "\\sa80"
               ".MACRO directive defines block of code (macro definitions). Label placed before .MACRO becomes macro definition name and is placed in macro name dictionary (which is separate from label names dictionary).\n"
               "After .MACRO directive one can place macro parameters and/or ellipsis (...). Parameter name can then be used in macro definition block. Defined parameters will be required when macro is used later in source code. To pass any number of parameters (also none) one can use ellipsis (...). If there are no params defined macro can be invoked without params only.\n"
               "To use params inside macro definition one can use their names or consecutive numbers (starting from 1) preceded by �%� character. Param number zero (%0) has special meaning--it contains number of actual parameters macro was invoked with. Instead of numbers numeric expression can be used if they are enclosed in square brackets (for example %[.cnt+1]).\n"
               "Note: To access string parameters append �$� sign: param$, %1$, etc. Without dollar sign string parameters return string length.\n"
               "In macro invocation actual parameters are placed after macro name. Param expressions are separated by commas. All those expression are assembly time expressions. They get interpreted and calculated and result values are passed to the macro definition.\n"
               "All labels starting with dot (.) are local to a macro definition block and are not accessible nor visible from the outside code using macrodefinition. All the other labels are global. Macrodefinition code can use local labels (from the place it was invoked), global labels, as well as it's own local labels.\n"
               "Macro definition parameters could be referenced with �$� suffix. If given param was passed as string it is still accessible as string using dollar sign. Accessing it without �$� suffix returns string length. Param 0$ has special meaning: macro name.\n"
               "#exmpl#"
               "Put:	.MACRO chr		; print single character\n"
               "	LDA #chr		; load value of parameter �chr�\n"
               "	JSR sys_put_char\n"
               "	.ENDM\n"
               "; invocation:\n"
               "	Put �A�\n"
               "\n"
               "Print:	.MACRO ...	; printing\n"
               ".cnt	.= 0			; param counter\n"
               "	.REPEAT %0			; for each parameter\n"
               ".cnt	.= .cnt + 1\n"
               "	.IF .PARAMTYPE(%.cnt) == 2	; text param?\n"
               "	  JSR sys_print_text	; string is placed *after* procedure call\n"
               "	  .BYTE .STRLEN(%.cnt$), %.cnt$\n"
               "	.ELSE					; numerical param -> address of string\n"
               "	  LDA #>%.cnt		; high address byte\n"
               "	  LDX #<%.cnt		; low address byte\n"
               "	  JSR sys_print\n"
               "	.ENDIF\n"
               "	.ENDR\n"
               "	.ENDM\n";

    else if (dirUpper == ".ENDM")
        return "#title#.ENDM#text#\nDirective closing macro definition block. Check also .MACRO."
               "#syntax#"
               "  .ENDM"
               "#exmpl#"
               "  .ENDM  ; end of macro definition\n";

    else if (dirUpper == ".EXITM")
        return "#title#.EXITM#text#\nDirective to stop macro expansion."
               "#syntax#"
               "  .EXITM"
               "#exmpl#"
               "  .EXITM  ; remaining macro code won't be inserted in the place it was invoked from\n"
               "#desc#"
               ".EXITM directive is useful when used in conjunction with .IF directive to conditionally stop macro expansion";

    else if (dirUpper == ".IF")
        return "#title#.IF#text#\nDirective opening conditional assembly block."
               "#syntax#"
               "  .IF expr"
               "#exmpl#"
               "  .IF .REF(alpha) ; if 'alpha' label was referenced\n"
               "  .IF a==5  ; if label 'a' equals 5\n"
               "  .IF b     ; if label 'b' has non-zero value\n"
               "  .IF %0>2  ; if macro invoked with more then two params\n";

    else if (dirUpper == ".ELSE")
        return "#title#.ELSE#text#\nDirective of conditional assembly."
               "#syntax#"
               "  .ELSE"
               "#exmpl#"
               "  .IF b\n"
               "    RTS\n"
               "  .ELSE ; if b wasn't <> 0\n"
               "   ; then following lines will be assembled\n"
               "   ; ...\n"
               "  .ENDIF\n";

    else if (dirUpper == ".ENDIF")
        return "#title#.ENDIF#text#\nDirective closing conditional assembly block."
               "#syntax#"
               "  .ENDIF"
               "#exmpl#"
               "  .ENDIF ; end of conditional assembly block\n";

    else if (dirUpper == ".ERROR")
        return "#title#.ERROR#text#\nDirective generating user assembly error."
               "#syntax#"
               "  .ERROR [textexpr]"
               "#exmpl#"
               "  .ERROR \"Required text parameter missing in macro \" + %0$\n";

    else if (dirUpper == ".REPEAT" ||
             dirUpper == ".REPT")
        return "#title#.REPEAT .REPT#text#\nDirective opening block of source text to be repeated given number of times."
               "#syntax#"
               "[<label>[:]] .REPEAT expr"
               "#exmpl#"
               "  .REPEAT 10  ; repeat 10 times\n\n"
               "  .REPEAT %0  ; repeat as many times as number of macro params\n\n"
               "  .REPEAT 4\n"
               "    LSR\n"
               "  .ENDR\n";

    else if (dirUpper == ".ENDR")
        return "#title#.ENDR#text#\nDirective closing block of source text to repeat."
               "#syntax#"
               "[<label>[:]] .ENDR"
               "#exmpl#"
               "  .ENDR\n";

    else if (dirUpper == ".SET" ||
             dirUpper == ".=")
        return "#title#.SET .=#text#\nDirective .SET assigns value to the label. This value can be changed (reassigned)."
               "#syntax#"
               "<label>[:] .SET expr\n"
               "#exmpl#"
               ".cnt .SET .cnt+1 ; increment .cnt\n";

    else if (dirUpper == ".ROM_AREA")
        return "#title#.ROM_AREA#text#\nDirective establishing memory protection area."
               "#syntax#"
               "[<label>[:]] .ROM_AREA addr_from_expr, addr_to_expr\n"
               "#exmpl#"
               " .ROM_AREA $a000, $afff\n"
               " .ROM_AREA Start, * ; from 'Start' to here\n"
               "#desc#"
               ".ROM_AREA turns on memory protection for a given range of addresses. Any attempt to write to this area"
               " will stop program execution. Write attempts to the EPROM usually indicate a bug and memory protection"
               " can facilitate locating such bugs. Specifying same start and end address turns protection off.\n";

    else if (dirUpper == ".IO_WND")
        return "#title#.IO_WND#text#\nDirective setting terminal window size."
               "#syntax#"
               "[<label>[:]] .IO_WND cols_expr, rows_expr\n"
               "#exmpl#"
               " .IO_WND 40, 20; 40 columns, 20 rows\n"
               "#desc#"
               ".IO_WND directive sets size of terminal window. It requires two parameters: number of columns and rows."
               " Both columns and rows are limited to 1..255 range.\n";

    else if (dirUpper == "IO_AREA")
        return "#title#IO_AREA#text#\nLabel representing beginning of simulator I/O area."
               "#syntax#"
               "IO_AREA = addr_expr ; set I/O area\n"
               "  <instruction> IO_AREA ; use I/O area\n"
               "#exmpl#"
               "IO_Cls = IO_AREA + 0 ; clear window port\n"
               "  STA IO_AREA+1 ; put char\n"
               "#desc#"
               "IO_AREA label represents beginning of simulator I/O area. Simulator can detect read and write attempts"
               " from/to its I/O area. Starting from IO_AREA address consecutive bytes are treated as virtual ports."
               " Following ports are defined:\n"
               "IO_AREA+0: TERMINAL_CLS (w)\n"
               "IO_AREA+1: TERMINAL_OUT (w)\n"
               "IO_AREA+2: TERMINAL_OUT_CHR (w)\n"
               "IO_AREA+3: TERMINAL_OUT_HEX (w)\n"
               "IO_AREA+4: TERMINAL_IN (r)\n"
               "IO_AREA+5: TERMINAL_X_POS (r/w)\n"
               "IO_AREA+6: TERMINAL_Y_POS (r/w)\n"
               "(w) means write only port, (r) read only, (r/w) read/write.\n\n"
               "TERMINAL_CLS - clear terminal window, set cursor at (0,0) position.\n"
               "TERMINAL_OUT - output single character interpreting control characters.\n"
               " Terminal can only recognize those characters:\n"
               " $d char (caret) moving cursor to the beginning of line,\n"
               " $a char (line feed) moving cursor to the next line and scrolling window if necessary,\n"
               " 8 char (backspace) moving one position to the left and erasing char below cursor.\n"
               "TERMINAL_OUT_CHR - outputs single character; control chars are being output just like regular characters.\n"
               "TERMINAL_OUT_HEX - outputs single byte as a two-digit hexadecimal number.\n"
               "TERMINAL_IN - input single byte, returns 0 if there's no characters available in terminal's buffer;"
               " when I/O terminal window is active it can accept keyboard input; press <Ins> key to paste clipboard's contents into terminal.\n"
               "TERMINAL_X_POS - cursor X position (column).\n"
               "TERMINAL_Y_POS - cursor Y position (row).\n";

    else if (dirUpper == ".XWORD" ||
             dirUpper == ".DX")
        return "#title#.XWORD .DX#text#\nDirectives defining 24-bit word values using passed arguments."
               " Long Words are written according to little-endian 6502 convention: low byte first, higher bytes follow"
               "#syntax#"
               "[<label>[:]] .DX expr { , expr }\n"
               "[<label>[:]] .XWORD expr { , expr }\n"
               "#exmpl#"
               "alpha: .DX $123456, 0 ; generates sequence $56, $34, $12, 0, 0, 0\n"
               "beta:  .XWORD alpha\n"
               "       .XWORD alpha-1, beta\n";

    else if (dirUpper == ".uint32_t" ||
             dirUpper == ".DDW")
        return "#title#.uint32_t .DDW#text#\nDirectives defining 32-bit word values using passed arguments."
               " Words are written according to little-endian 6502 convention: low byte first, higher bytes follow"
               "#syntax#"
               "[<label>[:]] .DDW expr { , expr }\n"
               "[<label>[:]] .uint32_t expr { , expr }\n"
               "#exmpl#"
               "alpha: .DDW $12345678, 0 ; generates sequence $78, $56, $34, $12, 0, 0, 0, 0\n"
               "beta:  .uint32_t alpha\n"
               "       .uint32_t alpha-1, beta\n";

    else if (dirUpper == ".DATE")
        return "#title#.DATE#text#\nDirective that inserts an ASCII representation of the local date in 3 user selected formats"
               "#syntax#"
               "[<label>[:]] .DATE    ; Default YYYY-MM-DD\n"
               "[<label>[:]] .DATE 0  ; none    YYYYMMDD\n"
               "[<label>[:]] .DATE '/'; user    YYYY/MM/DD\n"
               "#exmpl#"
               "alpha: .DATE '.' ;    YYYY.MM.DD\n";

    else if (dirUpper == ".TIME")
        return "#title#.TIME#text#\nDirective that inserts an ASCII representation of the local time in the 24 hour format"
               " in 3 user selected formats"
               "#syntax#"
               "[<label>[:]] .TIME    ; default  HH:MM:SS\n"
               "[<label>[:]] .TIME 0  ; none     HHMMSS\n"
               "[<label>[:]] .TIME '-'; user     HH-MM-SS\n"
               "#exmpl#"
               "beta: .TIME '.';   HH.MM.SS\n";

    else if (dirUpper == ".PARAMTYPE")
        return "#title#.PARAMTYPE#text#\nDirective that returns the type of parameter referenced: 1= number, 2=string"
               "#syntax#"
               "  .paramtype(label) ; use paramenter name\n"
               "  .paramtype(%1)    ; use parameter number from value\n"
               "  .paramtype(%.cnt) ; use parameter number from local variable\n"
               "\n"
               "#exmpl#"
               "     .if .paramtype(input) == 2  ; type= string\n"
               "        ...do something...\n"
               "     .endif\n";

    else if (dirUpper == ".STRLEN")
        return "#title#.STRLEN#text#\nDirective that returns the length of the reference string."
               "#syntax#"
               "[<label>[:]] .STRLEN(\"hello world\")    ; returns 11\n"
               "\n"
               "\n"
               "#exmpl#"
               "beta: .STRLEN(\"hello world\")    ; returns 11\n\n";

    else if (dirUpper == ".DEF")
        return "#title#.DEF#text#\nDirective that returns 1 if a label has been defined and 0 if not."
               " "
               "#syntax#"
               "[<label>[:]] .BYTE .DEF(test)  ; \n"
               "[<label>[:]] .IF .DEF(test)    ; \n"
               "\n"
               "#exmpl#"
               "beta:  .if .def(test) \n"
               "         .byte $AA    \n"
               "       .endif    \n";

    else if (dirUpper == ".REF")
        return "#title#.REF#text#\nDirective that returns 1 if a label has been referenced and 0 if not."
               " "
               "#syntax#"
               "[<label>[:]] .BYTE .REF(test)  ; \n"
               "[<label>[:]] .IF .REF(test)    ; \n"
               "\n"
               "#exmpl#"
               "beta:  .if .ref(test) \n"
               "         .byte $AA    \n"
               "       .endif    \n";

    else if (dirUpper == ".PASSDEF")
        return "#title#.PASSDEF#text# Needs further testing to determine function.\n"
               " "
               "#syntax#"
               "[<label>[:]] .BYTE .PASSDEF(test)  ; \n"
               "[<label>[:]] .IF .PASSDEF(test)    ; \n"
               "\n"
               "#exmpl#"
               "beta:  .if .passdef(test) \n"
               "         .byte $AA    \n"
               "       .endif    \n";

    return "";
}


extern int MatchingInstructions(const std::string& strWord, std::string& strResult)
{
    strResult.clear();

    if (strWord.empty())
        return 0;

    int nMatch = 0;

    for (int i = 0; s_vpszInstructionList[i] != 0; ++i)
    {
        if (strWord == s_vpszInstructionList[i])
        {
            strResult += s_vpszInstructionList[i];
            strResult += "\n";
            ++nMatch;
        }
    }

    if (nMatch == 1)
        strResult.resize(strResult.size() - 1);

    return nMatch;
}


class Instructions : CDeasm
{
    std::string AddMode(uint8_t cmd, CAsm::OpCode inst, CAsm::CodeAdr mode, ProcessorType procType);

public:

    std::string GetModes(CAsm::OpCode inst);
    std::string GetBranchInfo(bool bConditional = true);
};

std::string Instructions::AddMode(uint8_t cmd, CAsm::OpCode inst, CAsm::CodeAdr mode, ProcessorType procType)
{
    UNUSED(inst);

    std::string cs;
    cs = CDeasm::Mnemonic(cmd, procType, false);
    cs += CDeasm::Argument(cmd, mode, 0x8000, 0x34, 0x12, 0x00, false, true);
    cs += "\n";
    return cs;
}

std::string Instructions::GetBranchInfo(bool bConditional/*= true*/)
{
    std::string str = "All branch instructions are relative to the PC (current location--program counter)."
                      " They can jump forward or backward but are limited to local range (+/- 128 bytes).";

    if (bConditional)
        str += " Jump is effective if corresponding flag in status register is set/clear.";
    else
        str += " Jump is always effective regardless of flags set in status register.";

    return str;
}

std::string Instructions::GetModes(CAsm::OpCode inst)
{
    const uint8_t NA = 0x42;  //WDM on 65816
    std::string strModes;
    bool bExt1 = false;
    bool bExt2 = false;
    bool bExt3 = false;

    uint8_t codeC;

    for (int i = 0; i < CAsm::A_NO_OF_MODES; ++i)
    {
        uint8_t code = CAsm::trans_table[inst][i];
        if (code != NA)
            strModes += "<pre> " + AddMode(code, inst, CAsm::CodeAdr(i), ProcessorType::M6502);
        else
        {
            codeC = CAsm::trans_table_c[inst][i];
            if (codeC != NA)
            {
                code = CAsm::trans_table_8[inst][i];
                if (code != NA)
                {
                    bExt3 = true;
                    strModes += "<pre>�" + AddMode(code, inst, CAsm::CodeAdr(i), ProcessorType::WDC65816);
                }
                else
                {
                    strModes += "<pre>�" + AddMode(codeC, inst, CAsm::CodeAdr(i), ProcessorType::WDC65C02);
                    bExt1 = true;
                }
            }
            else
            {
                code = CAsm::trans_table_8[inst][i];
                if (code != NA)
                {
                    strModes += "<pre>�" + AddMode(code, inst, CAsm::CodeAdr(i), ProcessorType::WDC65816);
                    bExt2 = true;
                }
            }
        }
    }

    if (bExt3)
        strModes += "\n<pre>�<small> (65c02 and 65816 opcode)";

    if (bExt1)
        strModes += "\n<pre>�<small> (65c02 opcode)";

    if (bExt2)
        strModes += "\n<pre>�<small> (65816 opcode)";

    if (bExt1 || bExt2 || bExt3)
        strModes += "\n";

    return strModes;
}

//	desc += GetInstructionModes(C_ADC);

extern std::string GetInstructionDesc(const std::string& instruction)
{
    Instructions inst;
    std::string desc;

    std::string instUpper;
    instUpper.reserve(instruction.size());

    for (size_t i = 0; i < instruction.size(); ++i)
        instUpper += std::toupper(instruction[i]);

    if (instUpper == "ADC")
    {
        desc = "#title#ADC#text#\nAdd Memory to Accumulator with Carry."
               "#flags#NVZC#modes#";
        desc += inst.GetModes(CAsm::C_ADC);
        desc += "#desc#ADC adds memory to the accumulator. If D (decimal) flag bit is set ADC operates"
                " in BCD (packed Binary Coded Decimal) mode, where only decimal digits are allowed. If D flag is clear"
                " ADC operates in binary two's complement mode.\n"
                "#exmpl#"
                " ; add 5 to 'data' word\n"
                " CLD        ; binary mode\n"
                " CLC        ; clear carry\n"
                " LDA data   ; load data\n"
                " ADC #5     ; add 5\n"
                " STA data   ; store low byte\n"
                " BCC .skip  ; no carry over?\n"
                " INC data+1 ; inc hi byte\n"
                ".skip:\n"
                "\n"
                " ; add $0395 to 'data' word\n"
                " CLC        ; clear carry\n"
                " LDA data   ; load data\n"
                " ADC #$95\n"
                " STA data   ; store low byte\n"
                " LDA data+1\n"
                " ADC #$03   ; add with carry\n"
                " STA data+1 ; store hi byte\n"
                "\n<small> (in BCD mode flags N & Z are only set by 65c02 and undefined in case of 6502)\n";
    }
    else if (instUpper == "AND")
    {
        desc = "#title#AND#text#\n\"AND\" Memory with Accumulator."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_AND);
//		desc += "#desc#opis blah blah";
        desc +=
            "#exmpl#"
            " ; extract bits 0-3\n"
            " LDA data,X\n"
            " AND #$0F ; mask four bits\n";
    }
    else if (instUpper == "ASL")
    {
        desc = "#title#ASL#text#\nShift One Bit Left."
               "#flags#NZC#modes#";
        desc += inst.GetModes(CAsm::C_ASL);
        desc += "#desc#ASL shifts all bits left one position. Bit 0 is cleared and original bit 7 is moved into the Carry.\n"
                "#exmpl#"
                " ; extract bits 4-7\n"
                " LDA data,X\n"
                " ASL\n"
                " ASL\n"
                " ASL\n"
                " ASL\n"
                " ; bits 4-7 are in 0-3 position\n";
    }
    else if (instUpper == "BBR")
    {
        desc = "#title#BBR#text#\nBranch on Bit Reset."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BBR);
//		desc += "#desc#opis blah blah";
    }
    else if (instUpper == "BBS")
    {
        desc = "#title#BBS#text#\nBranch on Bit Set."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BBS);
    }
    else if (instUpper == "BCC")
    {
        desc = "#title#BCC#text#\nBranch on Carry Clear."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BCC);
        desc += "#exmpl#"
                " LDA data  ; load data\n"
                " CMP #10\n"
                " BCC .less ; jump if data < 10\n";
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BCS")
    {
        desc = "#title#BCS#text#\nBranch on Carry Set."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BCS);
        desc += "#exmpl#"
                " LDA data  ; load data\n"
                " CMP #10\n"
                " BCS .gt_eq ; jump if data >= 10\n";
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BEQ")
    {
        desc = "#title#BEQ#text#\nBranch on Result Zero."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BEQ);
        desc += "#exmpl#"
                " LDA flag  ; load data\n"
                " BEQ .zero ; jump if flag == 0\n"
                " CMP #5\n"
                " BEQ .five ; jump if flag == 5\n";
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BIT")
    {
        desc = "#title#BIT#text#\nTest Memory Bits with Accumulator."
               "#flags#NVZ#modes#";
        desc += inst.GetModes(CAsm::C_BIT);
        desc += "#desc#BIT performs \"AND\" operation on its argument and accumulator."
                " Result is not stored but Z(ero) flag is set accordingly. Flags N and V become"
                " copies of 7-th (oldest) and 6-th bits of BIT argument.\n"
                "#exmpl#"
                " LDA #@1010 ; bits to test\n"
                " BIT port   ; test port's bits\n"
                " BEQ .zero  ; both bits clear\n";
    }
    else if (instUpper == "BMI")
    {
        desc = "#title#BMI#text#\nBranch on Result Minus."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BMI);
        desc += "#exmpl#"
                " BIT flag  ; flag to test\n"
                " BMI .neg  ; jump if flag negative\n";
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BNE")
    {
        desc = "#title#BNE#text#\nBranch on Result Not Zero."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BNE);
        desc += "#exmpl#"
                " LDA flag      ; load data\n"
                " BNE .not_zero ; jump if flag != 0\n"
                " CMP #2\n"
                " BNE .not_two  ; jump if flag != 2\n";
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BPL")
    {
        desc = "#title#BPL#text#\nBranch on Result Plus."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BPL);
        desc += "#exmpl#"
                " LDX #10    ; load counter\n"
                ".delay:\n"
                " DEX\n"
                " BPL .delay ; jump if X >= 0\n";
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BRA")
    {
        desc = "#title#BRA#text#\nBranch Always."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BRA);
        desc += "#desc#" + inst.GetBranchInfo(false);
    }
    else if (instUpper == "BRK")
    {
        desc = "#title#BRK#text#\nForce Break."
               "#flags#B�DI#modes#";
        desc += inst.GetModes(CAsm::C_BRK);
        desc += "#desc#BRK forces interrupt. CPU fetches interrupt vector ($FFFE/F)"
                " and jumps to the interrupt handler routine. Bits I and B are set.\n"
                "Simulator can use this instruction to stop execution of your program.\n"
                "\n�<small> (D flag cleared only by 65c02 CPU)\n";
    }
    else if (instUpper == "BRL")
    {
        desc = "#title#BRL#text#\nBranch Long."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BRL);
        desc += "#desc#BRL jumps relative to the PC (current location--program counter)."
                " It can jump forward or backward but are limited to local range (+/- 32767 bytes)."
                " Jump is always effective regardless of flags set in status register."
                " It does not cross bank boundaries.";
    }
    else if (instUpper == "BVC")
    {
        desc = "#title#BVC#text#\nBranch on Overflow Clear."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BVC);
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "BVS")
    {
        desc = "#title#BVS#text#\nBranch on Overflow Set."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_BVS);
        desc += "#desc#" + inst.GetBranchInfo();
    }
    else if (instUpper == "CLC")
    {
        desc = "#title#CLC#text#\nClear Carry Flag."
               "#flags#C#modes#";
        desc += inst.GetModes(CAsm::C_CLC);
    }
    else if (instUpper == "CLD")
    {
        desc = "#title#CLD#text#\nClear Decimal Mode."
               "#flags#D#modes#";
        desc += inst.GetModes(CAsm::C_CLD);
    }
    else if (instUpper == "CLI")
    {
        desc = "#title#CLI#text#\nClear Interrupt Disable Bit."
               "#flags#I#modes#";
        desc += inst.GetModes(CAsm::C_CLI);
    }
    else if (instUpper == "CLV")
    {
        desc = "#title#CLV#text#\nClear Overflow Flag."
               "#flags#V#modes#";
        desc += inst.GetModes(CAsm::C_CLV);
    }
    else if (instUpper == "CMP")
    {
        desc = "#title#CMP#text#\nCompare Memory and Accumulator."
               "#flags#NZC#modes#";
        desc += inst.GetModes(CAsm::C_CMP);
    }
    else if (instUpper == "COP")
    {
        desc = "#title#COP#text#\nCo-processor interrupt."
               "#flags#NZC#modes#";
        desc += inst.GetModes(CAsm::C_COP);
        desc += "#desc#COP forces an interrupt. CPU fetches COP vector ($FFF4/$FFE4)"
                " and jumps to the COP handler routine. Bits I and D in the flag register are set."
                "Like BRK, the return address from COP is 2 bytes after the COP opcode. There is "
                "a signature byte following the COP opcode\n";
    }
    else if (instUpper == "CPX")
    {
        desc = "#title#CPX#text#\nCompare Memory and Index X."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_CPX);
    }
    else if (instUpper == "CPY")
    {
        desc = "#title#CPY#text#\nCompare Memory and Index Y."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_CPY);
    }
    else if (instUpper == "DEA")
    {
        desc = "#title#DEA#text#\n.Decrement Accumulator by One"
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_DEA);
    }
    else if (instUpper == "DEC")
    {
        desc = "#title#DEC#text#\nDecrement by One."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_DEC);
        desc += "#exmpl#"
                " ; subtract 1 from 6 conecutive bytes\n"
                " LDX #5      ; counter\n"
                ".dec:\n"
                " DEC data,X  ; decrement\n"
                " DEX\n"
                " BPL .dec    ; loop\n";
    }
    else if (instUpper == "DEX")
    {
        desc = "#title#DEX#text#\nDecrement Index X by One."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_DEX);
        desc += "#exmpl#"
                " ; clear buf[0..31]\n"
                " LDX #31   ; counter\n"
                " LDA #0\n"
                ".clr:\n"
                " STA buf,X ; clear buffer\n"
                " DEX\n"
                " BPL .clr  ; loop\n";
    }
    else if (instUpper == "DEY")
    {
        desc = "#title#DEY#text#\nDecrement Index Y by One."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_DEY);
        desc += "#exmpl#"
                " ; copy 200 bytes\n"
                " LDY #200\n"
                ".copy\n"
                " LDA (src),Y\n"
                " STA (dst),Y\n"
                " DEY\n"
                " BNE .copy\n";
    }
    else if (instUpper == "EOR")
    {
        desc = "#title#EOR#text#\n\"Exclusive-or\" Memory with Accumulator."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_EOR);
    }
    else if (instUpper == "INA")
    {
        desc = "#title#INA#text#\nIncrement Accumulator by One."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_INA);
    }
    else if (instUpper == "INC")
    {
        desc = "#title#INC#text#\nIncrement by One."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_INC);
        desc += "#exmpl#"
                " ; add 1 to 'data' word\n"
                " INC data   ; inc low byte\n"
                " BNE .skip\n"
                " INC data+1 ; inc hi byte\n"
                ".skip:\n";
    }
    else if (instUpper == "INX")
    {
        desc = "#title#INX#text#\nIncrement Index X by One."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_INX);
    }
    else if (instUpper == "INY")
    {
        desc = "#title#INY#text#\n.Increment Index Y by One"
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_INY);
    }
    else if (instUpper == "JML")
    {
        desc = "#title#JML#text#\nLong Jump to New Location using absolute indirect long addressing."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_JML);
        desc += "#desc#JML looks up the 3 bytes starting at the absolute address provided and moves them into the"
                "PBR and PC registers.";

    }
    else if (instUpper == "JMP")
    {
        desc = "#title#JMP#text#\nJump to New Location."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_JMP);
    }
    else if (instUpper == "JSL")
    {
        desc = "#title#JSL#text#\nLong Jump to Subroutine."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_JSL);
        desc += "#desc#JSR calls subroutine: it jumps to the new location saving return address on the stack,"
                "including the PBR register, so program execution can be resumed when subroutine ends with RTL.\n"
                "Due to the peculiarity of 6502 return address pushed on the stack is one less then an address of the"
                " instruction following JSL (i.e. addr - 1 is stored instead of addr).";
    }
    else if (instUpper == "JSR")
    {
        desc = "#title#JSR#text#\nJump to Subroutine."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_JSR);
        desc += "#desc#JSR calls subroutine: it jumps to the new location saving return address on the stack,"
                " so program execution can be resumed when subroutine ends with RTS.\n"
                "Due to the peculiarity of 6502 return address pushed on the stack is one less then an address of the"
                " instruction following JSR (i.e. addr - 1 is stored instead of addr).";
    }
    else if (instUpper == "LDA")
    {
        desc = "#title#LDA#text#\nLoad Accumulator with Memory."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_LDA);
    }
    else if (instUpper == "LDX")
    {
        desc = "#title#LDX#text#\nLoad Index X with Memory."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_LDX);
    }
    else if (instUpper == "LDY")
    {
        desc = "#title#LDY#text#\nLoad Index Y with Memory."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_LDY);
    }
    else if (instUpper == "LSR")
    {
        desc = "#title#LSR#text#\nShift One Bit Right."
               "#flags#NZC#modes#";
        desc += inst.GetModes(CAsm::C_LSR);
        desc += "#desc#LSR shifts all bits right one position. Bit 7 is cleared and original bit 0 is moved into the Carry.\n"
                "#exmpl#"
                " ; fast multiply by 4\n"
                " LDA data ; load data\n"
                " LSR      ; times 2\n"
                " LSR      ; times 2\n";
    }
    else if (instUpper == "NOP")
    {
        desc = "#title#NOP#text#\nNo Operation."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_NOP);
    }
    else if (instUpper == "MVN")
    {
        desc = "#title#MVN#text#\nBlock move descending."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_MVN);
        desc += "#desc#MVN moves a block of data from a higher source to a lower destination.\n"
                "#exmpl#"
                " MVN #00, #01;  Move a block of data from Bank 0 to Bank 1\n"
                " MVN #source>>16, #dest>>16; Move data from the bank byte in source to the bank byte in dest\n";
    }
    else if (instUpper == "MVP")
    {
        desc = "#title#MVP#text#\nBlock move ascending."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_MVP);
        desc += "#desc#MVP moves a block of data from a lower source to a higher destination..\n"
                "#exmpl#"
                " MVP #00, #01;  Move a block of data from Bank 0 to Bank 1\n"
                " MVP #source>>16, #dest>>16; Move data from the bank byte in source to the bank byte in dest\n";
    }
    else if (instUpper == "ORA")
    {
        desc = "#title#ORA#text#\n\"OR\" Memory with Accumulator."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_ORA);
    }
    else if (instUpper == "PEA")
    {
        desc = "#title#PEA#text#\nPush Effective address Immediate."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PEA);
        desc += "#desc#PEA stores a 16bit immediate value on the stack. RTS could be used to retrieve it.\n";
    }
    else if (instUpper == "PEI")
    {
        desc = "#title#PEI#text#\nPush Effective indirect address."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PEI);
        desc += "#desc#PEI stores a 16 bit value from a direct page address on the stack. RTS could be used to retrieve it.\n";
    }
    else if (instUpper == "PER")
    {
        desc = "#title#PER#text#\nPush Effective relative address."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PER);
        desc += "#desc#PER stores a 16 bit value relative to its location on the stack. RTS could be used to retrieve it.\n";
    }
    else if (instUpper == "PHA")
    {
        desc = "#title#PHA#text#\nPush Accumulator on the Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHA);
        desc += "#desc#PHA stores accumulator on the stack. PLA could be used to restore it.\n"
                "#exmpl#"
                " PHA      ; push A\n"
                " JSR putC\n"
                " PLA      ; pull A\n";
    }
    else if (instUpper == "PHB")
    {
        desc = "#title#PHB#text#\nPush Data Bank Register (DB) on Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHB);
        desc += "#desc#PHB stores the Data Bank Register (DB) on the stack. PLB could be used to restore it.\n";
    }
    else if (instUpper == "PHD")
    {
        desc = "#title#PHD#text#\nPush the direct page register (DP) on the Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHD);
        desc += "#desc#PHD stores the direct page register (DP) on the stack. PLD could be used to restore it.\n";
    }
    else if (instUpper == "PHK")
    {
        desc = "#title#PHK#text#\nPush the program bank register on the Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHK);
        desc += "#desc#PHK stores the program bank register on the stack.\n";
    }
    else if (instUpper == "PHP")
    {
        desc = "#title#PHP#text#\nPush Processor Status on Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHP);
    }
    else if (instUpper == "PHX")
    {
        desc = "#title#PHX#text#\nPush Index X on Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHX);
    }
    else if (instUpper == "PHY")
    {
        desc = "#title#PHY#text#\nPush Index Y on Stack."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_PHY);
    }
    else if (instUpper == "PLA")
    {
        desc = "#title#PLA#text#\nPull Accumulator from Stack."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_PLA);
    }
    else if (instUpper == "PLB")
    {
        desc = "#title#PLB#text#\nPull Data Bank Register (DB) from Stack."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_PLB);
    }
    else if (instUpper == "PLD")
    {
        desc = "#title#PLD#text#\nPull Direct Page Register (DP) from Stack."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_PLD);
    }
    else if (instUpper == "PLP")
    {
        desc = "#title#PLP#text#\nPull Process Status from Stack."
               "#flags#all#modes#";
        desc += inst.GetModes(CAsm::C_PLP);
    }
    else if (instUpper == "PLX")
    {
        desc = "#title#PLX#text#\nPull Index X from Stack."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_PLX);
    }
    else if (instUpper == "PLY")
    {
        desc = "#title#PLY#text#\nPull Index Y from Stack."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_PLY);
    }
    else if (instUpper == "REP")
    {
        desc = "#title#REP#text#\nReset bits in Status Register (P)."
               "#flags#NVMXDZC#modes#";
        desc += inst.GetModes(CAsm::C_REP);
    }
    else if (instUpper == "RMB")
    {
        desc = "#title#RMB#text#\nReset Memory Bit."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_RMB);
    }
    else if (instUpper == "ROL")
    {
        desc = "#title#ROL#text#\nRotate One Bit Left."
               "#flags#NZC#modes#";
        desc += inst.GetModes(CAsm::C_ROL);
        desc += "#desc#ROL shifts all bits left one position. Carry is copied to bit 0 and original bit 7 is moved into the Carry.\n"
                "#exmpl#"
                " ; shift left word data\n"
                " ASL data   ; shift low byte\n"
                " ; using Carry as temp bit\n"
                " ROL data+1 ; shift hi byte\n";
    }
    else if (instUpper == "ROR")
    {
        desc = "#title#ROR#text#\nRotate One Bit Right."
               "#flags#NZC#modes#";
        desc += inst.GetModes(CAsm::C_ROR);
        desc += "#desc#ROR shifts all bits right one position. Carry is copied to bit 7 and original bit 0 is moved into the Carry.\n"
                "#exmpl#"
                " ; shift right word data\n"
                " LSR data+1 ; shift hi byte\n"
                " ; using Carry as temp bit\n"
                " ROR data   ; shift low byte\n";
    }
    else if (instUpper == "RTI")
    {
        desc = "#title#RTI#text#\nReturn from Interrupt."
               "#flags#NVDIZC#modes#";
        desc += inst.GetModes(CAsm::C_RTI);
        desc += "#desc#RTI retrieves flags register from the stack, then it retrieves return address, so"
                " program execution can be resumed after an interrupt.";
    }
    else if (instUpper == "RTL")
    {
        desc = "#title#RTL#text#\nLong Return from Subroutine."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_RTL);
        desc += "#desc#RTL retrieves return address from the stack, including the Program Bank Register. RTL is used to return from subroutine invoked by JSL.\n"
                "Note: because JSL places address-1 value on the stack, RTL modifies it by adding 1 before it's used.";
    }
    else if (instUpper == "RTS")
    {
        desc = "#title#RTS#text#\nReturn from Subroutine."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_RTS);
        desc += "#desc#RTS retrieves return address from the stack. RTS is used to return from subroutine invoked by JSR.\n"
                "Note: because JSR places address-1 value on the stack, RTS modifies it by adding 1 before it's used.";
    }
    else if (instUpper == "SBC")
    {
        desc = "#title#SBC#text#\nSubtract Memory from Accumulator with Borrow."
               "#flags#NVZC#modes#";
        desc += inst.GetModes(CAsm::C_SBC);
        desc += "#desc#SBC subtracts memory from the accumulator. If D (decimal) flag bit is set SBC operates"
                " in BCD (packed Binary Coded Decimal) mode, where only decimal digits are allowed. If D flag is clear"
                " SBC operates in binary two's complement mode.\n"
                "#exmpl#"
                " CLD       ; binary mode\n"
                " SEC       ; clear borrow\n"
                " LDA #$90  ; load $90\n"
                " SBC #1    ; minus 1\n"
                " STA data  ; data=$8F\n"
                "\n"
                " SED       ; decimal mode\n"
                " SEC       ; clear borrow\n"
                " LDA #$90  ; this is 90 in BCD\n"
                " SBC #1    ; minus 1\n"
                " STA data  ; data=89 in BCD\n"
                "\n<small> (in BCD mode flags N & Z are only set by 65c02 and undefined in case of 6502)\n";
    }
    else if (instUpper == "SEC")
    {
        desc = "#title#SEC#text#\nSet Carry Flag."
               "#flags#C#modes#";
        desc += inst.GetModes(CAsm::C_SEC);
    }
    else if (instUpper == "SED")
    {
        desc = "#title#SED#text#\nSet Decimal Mode."
               "#flags#D#modes#";
        desc += inst.GetModes(CAsm::C_SED);
        desc += "#desc#SED sets decimal mode for ADC and SBC instructions. In BCD (packed Binary Coded Decimal)"
                " mode addition and subtraction operates on packed BCD numbers.";
    }
    else if (instUpper == "SEI")
    {
        desc = "#title#SEI#text#\nSet Interrupt Disable Bit."
               "#flags#I#modes#";
        desc += inst.GetModes(CAsm::C_SEI);
    }
    else if (instUpper == "SEP")
    {
        desc = "#title#SEP#text#\nSet bits in Status Register (P)."
               "#flags#NVMXDZC#modes#";
        desc += inst.GetModes(CAsm::C_SEP);
    }
    else if (instUpper == "SMB")
    {
        desc = "#title#SMB#text#\nSet Memory Bit."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_SMB);
    }
    else if (instUpper == "STA")
    {
        desc = "#title#STA#text#\nStore Accumulator in Memory."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_STA);
        desc += "#exmpl#"
                " LDA #$FF\n"
                " STA flag ; flag = $FF\n";
    }
    else if (instUpper == "STP")
    {
        desc = "#title#STP#text#\nStop the clock."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_STP);
        desc += "#desc#STP will stop the clock and the processor will wait for a hard reset via the /RES pin.";
    }
    else if (instUpper == "STX")
    {
        desc = "#title#STX#text#\nStore Index X in Memory."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_STX);
    }
    else if (instUpper == "STY")
    {
        desc = "#title#STY#text#\nStore Index Y in Memory."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_STY);
    }
    else if (instUpper == "STZ")
    {
        desc = "#title#STZ#text#\nStore Zero in Memory."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_STZ);
        desc += "#exmpl#"
                " STZ data  ; clear data byte\n";
    }
    else if (instUpper == "TAX")
    {
        desc = "#title#TAX#text#\nTransfer Accumulator in Index X."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TAX);
        desc += "#desc#TAX copies accumulator into the X register.\n";
    }
    else if (instUpper == "TAY")
    {
        desc = "#title#TAY#text#\nTransfer Accumulator in Index Y."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TAY);
        desc += "#desc#TAY copies accumulator into the Y register.\n";
    }
    else if (instUpper == "TCD")
    {
        desc = "#title#TCD#text#\nTransfer 16 bit Accumulator (C) to the Direct Register (D)."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TCD);
        desc += "#desc#TCD copies 16 bit accumulator into the Direct register.\n";
    }
    else if (instUpper == "TCS")
    {
        desc = "#title#TCS#text#\nTransfer 16 bit Accumulator (C) to the Stack Register (S)."
                  "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_TCS);
        desc += "#desc#TCS copies 16 bit accumulator into the Stack register.\n";
    }
    else if (instUpper == "TDC")
    {
        desc = "#title#TDC#text#\nTransfer Direct Register (D) to the 16 bit Accumulator (C)."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TDC);
        desc += "#desc#TDC copies Direct register into the 16 bit accumulator (C).\n";
    }
    else if (instUpper == "TRB")
    {
        desc = "#title#TRB#text#\nTest and Reset Memory Bits with Accumulator."
               "#flags#Z#modes#";
        desc += inst.GetModes(CAsm::C_TRB);
    }
    else if (instUpper == "TSB")
    {
        desc = "#title#TSB#text#\nTest and Set Memory Bits with Accumulator."
               "#flags#Z#modes#";
        desc += inst.GetModes(CAsm::C_TSB);
    }
    else if (instUpper == "TSC")
    {
        desc = "#title#TSC#text#\nTransfer Stack register (S) to the 16 bit Accumulator (C)."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_TSC);
        desc += "#desc#TSC copies the Stack register to the 16 bit accumulator.\n";
    }
    else if (instUpper == "TSX")
    {
        desc = "#title#TSX#text#\nTransfer Stack Pointer to Index X."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TSX);
        desc += "#desc#TSX copies stack pointer register S into the X register.\n";
    }
    else if (instUpper == "TXA")
    {
        desc = "#title#TXA#text#\nTransfer Index X to Accumulator."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TXA);
        desc += "#desc#TXA copies X register into the accumulator.\n";
    }
    else if (instUpper == "TXS")
    {
        desc = "#title#TXS#text#\nTransfer Index X to Stack Pointer."
               "#flags##modes#";
        desc += inst.GetModes(CAsm::C_TXS);
        desc += "#desc#TXS copies X register into the stack pointer register S.\n"
                "#exmpl#"
                " LDX #$FF\n"
                " TXS ; empty the stack\n";
    }
    else if (instUpper == "TXY")
    {
        desc = "#title#TXY#text#\nTransfer Index X to Index Y."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TXY);
        desc += "#desc#TXY copies X register to the Y register.\n"
                "#exmpl#"
                " LDX #$FF\n"
                " TXY ; move to Y register.\n";
    }
    else if (instUpper == "TYA")
    {
        desc = "#title#TYA#text#\nTransfer Index Y to Accumulator."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TYA);
        desc += "#desc#TYA copies Y register into the accumulator.\n"
                "#exmpl#"
                " PHA ; store accumulator\n"
                " TYA\n"
                " PHA ; and store Y\n";
    }
    else if (instUpper == "TYX")
    {
        desc = "#title#TYX#text#\nTransfer Index Y to Index X."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_TYX);
        desc += "#desc#TYX copies Y register to the X register.\n"
                "#exmpl#"
                " LDY #$FF\n"
                " TYX ; move to X register.\n";
    }
    else if (instUpper == "WAI")
    {
        desc = "#title#WAI#text#\nWiat for Interrupt."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_WAI);
        desc += "#desc#WAI Stops execution and waits for an Interrupt to occur.\n";
    }
    else if (instUpper == "WDM")
    {
        desc = "#title#WDM#text#\nReserved for future use.  Executes as a NOP."
               "#flags#-#modes#";
        desc += inst.GetModes(CAsm::C_WDM);
        desc += "#desc#WDM Reserved for future use.  Executes as a NOP.\n";
    }
    else if (instUpper == "XBA")
    {
        desc = "#title#XBA#text#\nSwaps 8 bit Accumulator (A) with upper 8 bits of 16 bit accumulator (B)."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_XBA);
        desc += "\n";
    }
    else if (instUpper == "XCE")
    {
        desc = "#title#XCE#text#\nExchange carry bit C with Emulation bit E in the Status Register P."
               "#flags#NZ#modes#";
        desc += inst.GetModes(CAsm::C_XCE);
        desc += "\n";
    }
    return desc;
}
