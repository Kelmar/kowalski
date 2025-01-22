/*=======================================================================*/
/*
 * Copyright (c) 2024 - Bryce Simonds
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*=======================================================================*/

#include "StdAfx.h"
#include "Asm.h"

/*=======================================================================*/

const char *CAsm::ERROR_MESSAGES[] =
{
    /*  0 */ wxTRANSLATE("No errors encountered"),
    /*  1 */ wxTRANSLATE("Unexpected data after instruction/label--comment or <CR> expected"),
    /*  2 */ wxTRANSLATE("Unrecognized data--space or label expected"),
    /*  3 */ wxTRANSLATE("Out of memory"),
    /*  4 */ wxTRANSLATE("Error reading file"),
    /*  5 */ wxTRANSLATE("Number exceeds 16-bit integer range"),
    /*  6 */ wxTRANSLATE("Value doesn't fit in single byte"),
    /*  7 */ wxTRANSLATE("Illegal negative value"),
    /*  8 */ wxTRANSLATE("Illegal data--instruction, comment or <CR> expected"),
    /*  9 */ wxTRANSLATE("Missing index register (X or Y)"),
    /* 10 */ wxTRANSLATE("Missing index register X"),
    /* 11 */ wxTRANSLATE("Missing index register Y"),
    /* 12 */ wxTRANSLATE("Expected comma ',' or bracket ')' or ']'"),
    /* 13 */ wxTRANSLATE("Missing bracket ')' or ']'"),
    /* 14 */ wxTRANSLATE("Expected round bracket '(' or '['"),
    /* 15 */ wxTRANSLATE("Divide by zero in expression"),
    /* 16 */ wxTRANSLATE("Missing bracket '}' closing expression"),
    /* 17 */ wxTRANSLATE("Missing constant value (number, label, function or '*')"),
    /* 18 */ wxTRANSLATE("Redefinition - label already defined"),
    /* 19 */ wxTRANSLATE("Indeterminate expression value"),
    /* 20 */ wxTRANSLATE("Wrapped program counter"),
    /* 21 */ wxTRANSLATE("Undefined label"),
    /* 22 */ wxTRANSLATE("Phase error--inconsistent label value between passes"),
    /* 23 */ wxTRANSLATE("Relative address out of range"),
    /* 24 */ wxTRANSLATE("Addressing mode not allowed"),
    /* 25 */ wxTRANSLATE("Missing string"),
    /* 26 */ wxTRANSLATE(".ENDIF directive without pairing .IF"),
    /* 27 */ wxTRANSLATE(".ELSE directive without pairing .IF"),
    /* 28 */ wxTRANSLATE("Missing .ENDIF directive"),
    /* 29 */ wxTRANSLATE("Local label not allowed - global label expected"),
    /* 30 */ wxTRANSLATE("Missing label"),
    /* 31 */ wxTRANSLATE("Assembly stopped"),
    /* 32 */ wxTRANSLATE("Missing .ORG directive--undetermined program beginning address"),
    /* 33 */ wxTRANSLATE("Missing macro name label"),
    /* 34 */ wxTRANSLATE("Repeated macro parameter name"),
    /* 35 */ wxTRANSLATE("Nested macro definition not allowed"),
    /* 36 */ wxTRANSLATE("Missing .ENDM directive ending macro definition"),
    /* 37 */ wxTRANSLATE("Unrecognized instruction/directive/macro name"),
    /* 38 */ wxTRANSLATE("Not enough parameters in macro call"),
    /* 39 */ wxTRANSLATE(".ENDM directive without pairing .MACRO"),
    /* 40 */ wxTRANSLATE(".EXITM directive outside macro definition not allowed"),
    /* 41 */ wxTRANSLATE("Text expression not allowed"),
    /* 42 */ wxTRANSLATE("Parameter referenced with dollar char '$' has no text value"),
    /* 43 */ wxTRANSLATE("Referenced parameter not exist--param number out of range"),
    /* 44 */ wxTRANSLATE("Undetermined expression value in macro parameter number"),
    /* 45 */ wxTRANSLATE("Local label not allowed for macro definition name"),
    /* 46 */ wxTRANSLATE("After '%' char macro parameter number is expected"),
    /* 47 */ wxTRANSLATE("Label not allowed for directive"),
    /* 48 */ wxTRANSLATE("Wrong repeat number - expected value in range from 0 to 65535"),
    /* 49 */ wxTRANSLATE(".ENDR directive without pairing .REPEAT"),
    /* 50 */ wxTRANSLATE(".INCLUDE directive not allowed in macro/repeat"),
    /* 51 */ wxTRANSLATE("String to long--maximal length: 255 characters"),
    /* 52 */ wxTRANSLATE("Wrong bit number--expected value from 0 to 7"),
    /* 53 */ wxTRANSLATE("Missing required option name"),
    /* 54 */ wxTRANSLATE("Unrecognized option name"),
    /* 55 */ wxTRANSLATE("Input line exceeds 1024 characters limit"),
    /* 56 */ wxTRANSLATE("Missing macro parameter name"),
    /* 57 */ wxTRANSLATE("Indirect post indexed addressing mode requires byte operand"),
    /* 58 */ wxTRANSLATE("Predefined label cannot be redefined"),
    /* 59 */ wxTRANSLATE("Invalid range: first value has to be less than or equal to 2nd value"),
    /* 60 */ wxTRANSLATE("Expected nonzero value"),
    /* 61 */ wxTRANSLATE("Execution crossed a bank boundary"),
    /* 62 */ wxTRANSLATE("Macro parameter count exceeds expected count"),
    /* 63 */ wxTRANSLATE("Debugging error")
};

const char *CAsm::ERROR_FORMATS[] =
{
    /* 1 */ wxTRANSLATE("{file}({line_number}): error E{error_code}: {message}"),
    /* 2 */ wxTRANSLATE("{file}: error E{error_code}: {message}"),
    /* 3 */ wxTRANSLATE("Error E{error_code}: {message}"),
    /* 4 */ wxTRANSLATE("{file}({line_number}): error E{error_code}: {message} '{ident}'"),
    /* 5 */ wxTRANSLATE("{file}({line_number}): user error: {message}"),
    /* 6 */ wxTRANSLATE("{file}({line_number}): user error: NO MESSAGE")
};

/*=======================================================================*/

const size_t CAsm::ERROR_MESSAGES_COUNT = sizeof(CAsm::ERROR_MESSAGES) / sizeof(char *);

/*=======================================================================*/
