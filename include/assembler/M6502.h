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

#ifndef M6502_H__
#define M6502_H__

#include "output.h"

#include "Asm.h"

#include "Ident.h"
#include "DebugInfo.h"
#include "OutputMem.h"

#include "InputStack.h"

#include "ConditionalAsm.h"

#include "MarkArea.h"

enum class CTokenType
{
    L_UNKNOWN,      // Unrecognized character

    L_NUM,          // Number (dec, hex, bin, or sign)
    L_STR,          // A string of characters in apostrophes or quotation marks
    L_IDENT,        // Identifier
    L_IDENT_N,      // Numbered identifier (ending with '#' and a number)
    L_SPACE,        // Space
    L_OPER,         // Operator
    L_BRACKET_L,    // Left paren '('
    L_BRACKET_R,    // Right paren ')'
    L_LBRACKET_L,   // Left bracket '('
    L_LBRACKET_R,   // Right bracket ')'
    L_EXPR_BRACKET_L, // Left square bracket '['
    L_EXPR_BRACKET_R, // Right square bracket ']'
    L_COMMENT,      // Comment character ';'
    L_LABEL,        // Label character ':'
    L_COMMA,        // Comma character ','
    L_STR_ARG,      // Dollar sign '$', ends text type parameter
    L_MULTI,        // Ellipsis character '...'
    L_INV_COMMAS,   // Sign in quotation marks ????
    L_LHASH,        // '!#'
    L_HASH,         // Hash sign '#'
    L_EQUAL,        // Assignment character '='
    L_PROC_INSTR,   // Processor instruction
    L_ASM_INSTR,    // Assembler directive
    L_CR,           // End of line
    L_FIN,          // End of data
    L_ERROR         // Bad token type
};

class CToken
{
public:
    struct Code
    {
        CAsm::OpCode code;
        CAsm::CodeAdr adr;
    };

    enum InstrArg // Type of directive arguments
    {
        A_BYTE,
        A_NUM,
        A_LIST,
        A_STR,
        A_2NUM
    };

    struct Instr
    {
        CAsm::InstrType type;
        InstrArg arg;
    };

    enum NumType // Type of number
    {
        N_DEC,
        N_HEX,
        N_BIN,
        N_CHR,	// character
        N_CHR2	// Two characters, e.g. 'ab'
    };

    struct Num
    {
        NumType type;
        int32_t value;
    };

    enum Error
    {
        ERR_NUM_HEX,
        ERR_NUM_DEC,
        ERR_NUM_BIN,
        ERR_NUM_CHR,
        ERR_NUM_BIG,    // Out of bounds error
        ERR_BAD_CHR,
        ERR_STR_UNLIM	// An unclosed string of characters
    };

    //...........................................................................

    CTokenType type;

private:
    union
    {
        //Id id;               // Identifier
        CAsm::OperType op;     // Binary or unary operator
        CAsm::OpCode code;     // Polynomial
        CAsm::InstrType instr; // Directive
        Num num;               // Numeric or character constant
        
        Error err;

        char block[32]; // Used for block copy
    };

    std::string str; // Identifier or string

public:
    CToken(const CToken &leks);

    ~CToken();

    CToken(CTokenType t)
        : type(t)
        , op()
    {
    }

    CToken(CAsm::OperType oper)
        : type(CTokenType::L_OPER)
        , op(oper)
    {
    }

    CToken(CAsm::OpCode code)
        : type(CTokenType::L_PROC_INSTR)
        , code(code)
    {
    }

    CToken(CAsm::InstrType it)
        : type(CTokenType::L_ASM_INSTR)
        , instr(it)
    {
    }

    CToken(Error err)
        : type(CTokenType::L_ERROR)
        , err(err)
    {
    }

    CToken(NumType type, int32_t val)
        : type(CTokenType::L_NUM)
    {
        num.type = type;
        num.value = val;
    }

    CToken(std::string str) 
        : type(CTokenType::L_STR)
        , str(str)
    {
    }

    CToken(std::string str, int) 
        : type(CTokenType::L_IDENT)
        , str(str)
    {
    }

    CToken(std::string str, long) 
        : type(CTokenType::L_IDENT_N)
        , str(str)
    {
    }

//  CToken(const CToken &leks, long) : type(TokenType::L_IDENT_N)
//  { ASSERT(leks.type == TokenType::L_IDENT);  str = new std::string(*leks.str); }

    CToken & operator =(const CToken &);

    const std::string GetIdent() const
    {
        ASSERT(type == CTokenType::L_IDENT);
        return str;
    }

    CAsm::OperType GetOper()
    {
        ASSERT(type == CTokenType::L_OPER);
        return op;
    }

    CAsm::OpCode GetCode()
    {
        ASSERT(type == CTokenType::L_PROC_INSTR);
        return code;
    }

    CAsm::InstrType GetInstr()
    {
        ASSERT(type == CTokenType::L_ASM_INSTR);
        return instr;
    }

    int GetValue()
    {
        ASSERT(type == CTokenType::L_NUM);
        return num.value;
    }

    const std::string &GetString() const
    {
        ASSERT(type == CTokenType::L_STR || type == CTokenType::L_IDENT || type == CTokenType::L_IDENT_N);
        return str;
    }

    void Format(int32_t val) // Normalize the form of the numeric label
    {
#if 0
        ASSERT(type == CTokenType::L_IDENT_N);
        CString num(' ', 9);
        num.Format("#%08X", (int)val);
        *str += num; // Append number
#endif

        // TODO: Definitely need to verify this change. -- B.Simonds (April 25, 2024)

        ASSERT(type == CTokenType::L_IDENT_N);

        char tmp[64];

        snprintf(tmp, sizeof(tmp), "       #%08X%d", (int)val, num.value);
        str = tmp;
    }
};

/*************************************************************************/

class CIdentTable
{
private:
    std::unordered_map<std::string, CIdent> m_map;

public:
    /* constructor */ CIdentTable() { }
    virtual          ~CIdentTable() { }

    void set(const std::string &str, const CIdent &ident)
    {
        m_map[str] = ident;
    }

    bool insert(const std::string &str, _Inout_ CIdent &ident)
    {
        auto itr = m_map.find(str);

        if (itr == m_map.end())
        {
            // New element
            set(str, ident);
            return true;
        }

        ident = itr->second;
        return false;
    }

    bool replace(const std::string &str, const CIdent &ident)
    {
        auto itr = m_map.find(str);

        if (itr == m_map.end())
        {
            // New element
            m_map[str] = ident;
        }
        else
        {
            CIdent &val = itr->second;

            if ((val.variable || val.info == CIdent::I_UNDEF) && ident.variable)
            {
                // replace the old variable value with a new element
                m_map[str] = ident;
                return true;
            }
            else if (val.variable || ident.variable) // check here either, or
            {
                // redefinition not allowed (changing type from constant to variable or vice versa)
                return false;
            }
            else if (val.info != CIdent::I_UNDEF) // old element already defined?
            {
                m_map[str] = ident; // replacing an old one with a new element
                return false; // redefinition notification
            }

            m_map[str] = ident; // replacing an old, undefined element with a new element
        }

        return true; // OK
    }

    bool contains(const std::string &str)
    {
        auto itr = m_map.find(str);
        return itr != m_map.end();
    }

    bool lookup(const std::string &str, _Out_ CIdent &ident)
    {
        auto itr = m_map.find(str);

        if (itr != m_map.end())
        {
            ident = itr->second;
            return true;
        }

        return false;
    }
};

/*************************************************************************/

/**
 * @brief A class to describe an arithmetic/logical/text expression
 */
struct Expr
{
    int32_t value;
    std::string string;
    
    enum
    {
        EX_UNDEF,   // Unknown value
        EX_BYTE,    // Byte i.e. from -255 to 255 (sic!)
        EX_WORD,    // Word, from -65535 to 65535
        EX_LONG,    // outside the above range
        EX_STRING   // String of characters
    } inf;

    Expr()
        : value(0)
        , inf(EX_UNDEF)
    {}

    Expr(int32_t value)
        : value(value)
        , inf(EX_LONG)
    {}
};


// Line reading common elements for .MACRO, .REPEAT and normal reading
class CSource //: /*public CObject,*/ public CAsm
{
    CAsm::FileUID m_fuid;
    int cond_level_;

public:
    CSource()
        : m_fuid(0)
    {}

    virtual ~CSource()
    {}

    virtual void Start(CConditionalAsm* cond)
    {
        cond_level_ = cond ? cond->get_level() : INT_MAX;
    }

    virtual void Fin(CConditionalAsm* cond)
    {
        if (cond && cond_level_ != INT_MAX)
            cond->restore_level(cond_level_);
    }

    virtual bool GetCurrLine(std::string &str) = 0;

    virtual int GetLineNo() const = 0;

    virtual CAsm::FileUID GetFileUID() const
    {
        return m_fuid;
    }

    void SetFileUID(CAsm::FileUID fuid)
    {
        m_fuid = fuid;
    }

    virtual std::string GetFileName() const = 0;

private:
    // remember nesting level of conditional assemblation
    void StoreConditionLevel(int level)
    {
        cond_level_ = level;
    }

    int GetConditionLevel() const
    {
        return cond_level_;
    }
};

//.............................................................................

// Elements required for storing and reproducing program source lines
class CRecorder
{
private:
    std::vector<std::string> m_strarrLines;
    std::vector<uint32_t> m_narrLineNums;

    int m_nLine;

public:
    CRecorder(int nInitSize = 10) // Initial array sizes
        : m_strarrLines(nInitSize)
        , m_narrLineNums(nInitSize)
        , m_nLine(0)
    {
    }

    virtual ~CRecorder()
    {}

    void AddLine(const std::string &strLine, int num) // Save the next line
    {
        m_strarrLines[m_nLine] = strLine;
        m_narrLineNums[m_nLine] = num;

        ++m_nLine;
        
        //m_strarrLines.SetAtGrow(m_nLine, strLine);
        //m_narrLineNums.SetAtGrow(m_nLine, num);

        //m_nLine++;
    }

    const std::string& GetLine(int nLineNo) // reading the 'nLineNo' line
    {
        return m_strarrLines[nLineNo];
    }

    int GetLineNo(int nLineNo) const // read the line number in the source file
    {
        return m_narrLineNums[nLineNo];
    }

    int GetSize() // Reading the number of rows in the array
    {
        //ASSERT(m_narrLineNums.GetSize() == m_narrLineNums.GetSize());
        return m_nLine;
    }
};

class CAsm6502;

#include "MacroDef.h"

typedef std::vector<CMacroDef> CMacroDefs;

//-----------------------------------------------------------------------------

class CRepeatDef : public CSource, public CRecorder
{
private:
    int m_nLineNo; // current line number (when reading)
    int m_nRepeat; // number of repeated lines

public:

    CRepeatDef(int nRept = 0)
        : m_nLineNo(0)
        , m_nRepeat(nRept)
    {}

    ~CRepeatDef()
    {}

    virtual bool GetCurrLine(std::string &str); // Read the current line

    virtual int GetLineNo() const
    {
        return CRecorder::GetLineNo(m_nLineNo - 1);
    }

    virtual void Start(CConditionalAsm* cond)
    {
        CSource::Start(cond); // Line counter at the end
        m_nLineNo = GetSize();
    }

    virtual void Fin(CConditionalAsm* cond) // end repeat lines
    {
        CSource::Fin(cond);
        delete this;
    }

    //virtual bool IsRepeat() //data source is repeat (.REPEAT)
    //{ return true; }

    CRepeatDef &operator= (const CRepeatDef &)
    {
        ASSERT(false); // objects of type CRepeatDef cannot be assigned
        return *this;
    }

    virtual std::string GetFileName() const { return ""; }
};

#if 0
class CRepeatDefs : public CArray<CRepeatDef, CRepeatDef&>
{
};
#endif

typedef std::vector<CRepeatDef> CRepeatDefs;

//-----------------------------------------------------------------------------

class CSourceText : public CSource
{
private:
    CInputStack input;

public:
    CSourceText(const std::string &fileName)
        : input(fileName)
    {}

    CSourceText()
    {}

    virtual void Start(CConditionalAsm* cond) override // start reading lines
    {
        CSource::Start(cond);
        input.SeekToBegin();
    }

    virtual bool GetCurrLine(std::string &str) override // reading less than an entire line
    {
        str.reserve(1024 + 4);
        return input.ReadLine(str);
    }

    virtual int GetLineNo() const override // read the line number
    {
        return input.GetLineNumber();
    }

    virtual CAsm::FileUID GetFileUID() const override // read file ID
    {
        return input.GetFileUID();
    }

    void SetFileUID(CDebugInfo* pDebugInfo)
    {
        if (pDebugInfo)
            input.SetFileUID(pDebugInfo->GetFileUID(input.GetFileName()));
    }

    void Include(const std::string &fname, CDebugInfo* pDebugInfo = nullptr) // include file
    {
        input.OpenFile(fname);

        if (pDebugInfo)
            input.SetFileUID(pDebugInfo->GetFileUID(fname));
    }

    bool TextFin() // current file finished
    {
        if (input.GetCount() > 1) // nested read (.include)?
        {
            input.CloseFile();
            return true;
        }
        else
            return false; // end of source files
    }

    /*
    bool IsPresent() const // Check whether there is any file being read
    { return input.IsPresent(); }
    */
   
    virtual std::string GetFileName() const override
    {
        return input.GetFileName();
    }
};

//-----------------------------------------------------------------------------

// Stack of objects that are the source of the rows
class CSourceStack
{
private:
    std::vector<CSource *> m_items;

    template <typename T>
    T *ReverseFind()
    {
        for (auto &i : std::views::reverse(m_items))
        {
            T *source = dynamic_cast<T *>(i);
            if (source)
                return source;
        }

        return nullptr;
    }

public:
    CSourceStack()
        : m_items()
    {
    }

    ~CSourceStack()
    {
        for (auto item : m_items)
            item->Fin(0);
    }

    void Push(CSource *source) // Add an item to the top of the stack
    {
        m_items.push_back(source);
    }

    CSource *Peek() // Check the item at the top of the stack
    {
        if (m_items.empty())
            return nullptr;

        return m_items.back();
    }

    CSource *Pop() // Remove an item from the stack
    {
        if (m_items.empty())
            return nullptr;

        CSource *rval = m_items.back();
        m_items.pop_back();
        return rval;
    }

    CSource* FindMacro() // Find the last macro
    {
        return ReverseFind<CMacroDef>();
    }

    CSource* FindRepeat() // Find the last repetition
    {
        return ReverseFind<CRepeatDef>();
    }
};

//-----------------------------------------------------------------------------

class CAsm6502 : public CAsm /*, public CObject */
{
private:
    friend class CMacroDef;

    io::output &m_console;

    std::string current_line;
    const char *ptr;                // To keep track of the current line
    const char *err_start;
    const char *ident_start;        // Position of the identifier on the line
    const char *ident_fin;          // Position of the end of the identifier on the line

    bool check_line;                // flag: true - Analysis of one line, false - Program
    uint32_t origin;
    bool originWrapped;             // true - if the command counter "wrapped"
    uint32_t m_progStart;         // Beginning of the program
    uint32_t mem_mask;              // Processor memory limit (mask), normally $FFFF
    int local_area;                 // Local label area number
    int proc_area;                  // Local label area number
    int macro_local_area;           // Area number of the local macro definitions
    int pass;                       // Transition number (1 or 2)
    //int conditional;              // Conditional assembly - depth level
    std::string m_includeFileName;
    std::string user_error_text;    // User error text (.ERROR directives)

    const char *instr_start;        // To remember the beginning and 
    const char *instr_fin;          // end of an instruction in a line
    CMacroDefs macros;              // Macro definitions

    CSource *text;                  // Current source text
    CSourceText entire_text;        // First (initial) source text
    CRepeatDef *pRept;

    // leksyka:
    CToken get_dec_num();          // Interpretation of a decimal number
    CToken get_hex_num();          // Interpretation of a hexadecimal number
    CToken get_bin_num();          // Interpretation of binary number
    CToken get_char_num();         // Interpretation of the character constant

    // Extract a string from the lexer.
    std::shared_ptr<std::string> GetIdent();

    CToken get_string(char lim);	// Extracting a string of characters
    CToken eat_space();			// Skip whitespace
    bool proc_instr(const std::string &str, OpCode &code);
    bool asm_instr(const std::string &str, InstrType &it);
    CToken next_leks(bool nospace= true); // Get the next symbol
    bool next_line();               // Load the next line

    COutputMem *out;				// memory for the object code
    CMarkArea *markArea;			// to mark used memory areas with 'out'
    CIdentTable local_ident;        // Array of local identifiers
    CIdentTable proc_local_ident;   // Array of local identifiers
    CIdentTable global_ident;       // Array of global identifiers
    CIdentTable macro_name;         // Array of macro definition names
    CIdentTable macro_ident;        // Array of IDs in macro expansions
    CDebugInfo *debug;              // Commissioning information for the simulator

    //std::string err_file;         // Name of the file whose reading resulted in an error
    //int err_line;                 // Line number where the error was encountered
    std::string err_ident;          // Name of the label that caused the error

    bool temporary_out;             // Flag -'out' has been allocated in the constructor
    bool abort_asm;                 // Variable to force assembly to be interrupted externally
    bool is_aborted()
    {
        return abort_asm ? abort_asm=false, true : false;
    }

    bool add_ident(const std::string &ident, CIdent &inf);

    Stat def_ident(const std::string &ident, const CIdent &inf);
    Stat chk_ident(const std::string &ident, _Inout_ CIdent &inf);
    Stat chk_ident_def(const std::string &ident, CIdent &inf);
    Stat def_macro_name(const std::string &ident, CIdent &inf);
    Stat chk_macro_name(const std::string &ident);

    std::string format_local_label(const std::string &ident, int area);
    // Interpretation of processor instructions
    Stat proc_instr_syntax(CToken &leks, CodeAdr &mode, Expr &expr, Expr &expr_bit, Expr &expr_zpg);
    // Interpretation of the directive
    Stat asm_instr_syntax_and_generate(CToken &leks, InstrType it, const std::string *pLabel);

    CMacroDef *get_new_macro_entry()
    {
        macros.resize(macros.size() + 1);
        return &macros[macros.size() - 1];
    }

    CMacroDef *get_last_macro_entry()
    {
        ASSERT(!macros.empty());
        return &macros[macros.size() - 1];
    }

    int get_last_macro_entry_index()
    {
        ASSERT(!macros.empty());
        return macros.size() - 1;
    }

//  Stat (CLeksem &leks);
    int find_const(const std::string& str);
    Stat predef_const(const std::string &str, Expr &expr, bool &found);
    Stat predef_function(CToken &leks, Expr &expr, bool &fn);
    Stat constant_value(CToken &leks, Expr &expr, bool nospace);
    Stat factor(CToken &leks, Expr &expr, bool nospace= true);
    Stat mul_expr(CToken &leks, Expr &expr);
    Stat shift_expr(CToken &leks, Expr &expr);
    Stat add_expr(CToken &leks, Expr &expr);
    Stat bit_expr(CToken &leks, Expr &expr);
    Stat cmp_expr(CToken &leks, Expr &expr);
    Stat bool_expr_and(CToken &leks, Expr &expr);
    Stat bool_expr_or(CToken &leks, Expr &expr);
    Stat expression(CToken &leks, Expr &expr, bool str= false); // Expression expansion
    bool is_expression(const CToken &leks);
    Stat assemble_line();           // Line interpretation

    const char *get_next_line();    // Load the next line into the assembly
    const char *play_macro();       // Reading the next line of the macro
    const char *play_repeat();      // Reading the next replay line

    CSourceStack source;            // Stack of objects returning source rows

    void asm_start();               // Start assembly
    void asm_fin();                 // Complete assembly
    void asm_start_pass();          // Start taking over assembly
    void asm_fin_pass();            // Complete assembly takeover
    Stat chk_instr_code(OpCode &code, CodeAdr &mode, Expr expr, int &length);
    void generate_code(OpCode code, CodeAdr mode, Expr expr, Expr expr_bit, Expr expr_zpg);
    Stat inc_prog_counter(int dist);
    Stat look_for_endif();          // Search for .ENDIF or .ELSE
    //int get_line_no();            // Line number (for debug info)
    //FileUID get_file_UID();       // File id (for debug info)
    void generate_debug(uint32_t addr, int line_no, FileUID file_UID);
    Stat generate_debug(InstrType it, int line_no, FileUID file_UID);
    void generate_debug();
    Stat look_for_endm();
    Stat record_macro();
    //void MacroExpandStart(CMacroDef *pMacro); // Switch to macro definition expansion mode
    //void MacroExpandFin();        // Ends macro expansion mode
    CMacroDef *in_macro;            // Currently recording macro or NULL
    CMacroDef *expanding_macro;     // Currently expanding macro or NULL
    //CPtrStack<CMacroDef> expand_macros; // List of drop-down macros
    CRepeatDef *repeating;          // Current replay (.REPEAT)
    //CPtrStack<CRepeat Def> repeats; // Replay list
    Stat record_rept(CRepeatDef *pRept);
    Stat look_for_repeat();         // Search for .ENDR or .REPEAT
    int reptInit;                   // Value to initialize the number of repetitions
    int reptNested;                 // Nest counter .REPEAT (when registering)
    //void RepeatStart(CRepeatDef *pRept);
    //void RepeatFin();
    bool b_listing;

    //static int __cdecl asm_str_key_cmp(const void *elem1, const void *elem2);
    static int asm_str_key_cmp(const void *elem1, const void *elem2);

    struct ASM_STR_KEY
    {
        const char *str;
        CAsm::InstrType it;
    };

    CConditionalAsm conditional_asm;

    class CListing
    {
    private:
        std::FILE *m_file;
        std::string m_fileName;
        std::string m_str; // Current listing line
        int m_lineNumber;

        void Open(const char *fname)
        {
            m_file = std::fopen(fname, "w");
        }

        void Close()
        {
            if (m_file != nullptr)
            {
                fclose(m_file);
                m_file = nullptr;
            }
        }

    public:
        CListing()
            : m_file(nullptr)
            , m_fileName()
            , m_str()
            , m_lineNumber(0)
        {
        }

        CListing(const char *fname);

        ~CListing()
        {
            if (m_file != nullptr)
                Close();
        }

        const std::string &filename() const { return m_fileName; }

        int lineNumber() const { return m_lineNumber; }

        void Remove();
        void NextLine();
        void AddCodeBytes(uint32_t addr, int code1 = -1, int code2 = -1, int code3 = -1, int code4 = -1);
        void AddValue(uint32_t val);
        void AddBytes(uint32_t addr, uint16_t mask, const uint8_t mem[], int len);
        void AddSourceLine(const std::string &line);

        bool IsOpen() const { return m_file != nullptr; }
    } listing;

    void init();
    void init_members();

public:
    // TODO: Make this method private later
    Stat report_error(Stat err)
    {
        const std::string msg = GetErrMsg(err);
        m_console.write(msg.c_str());
        m_console.write("\r\n");
        return err;
    }

    ProcessorType m_procType;

    static bool caseinsense;
    static bool case_insensitive; //true -> lowercase and uppercase letters in labels are not distinguished
    static bool swapbin;
    static bool swap_bin;
    static uint8_t forcelong;
    static bool generateBRKExtraByte; // Generate an extra byte after the BRK instruction?
    static uint8_t BRKExtraByte;      // The value of the additional byte after the BRK instruction

    CAsm6502(const std::string &file_in_name,
             io::output &console,
             COutputMem *out,
             CDebugInfo *debug,
             CMarkArea *area,
             ProcessorType procType,
             const char *listing_file = NULL)
        : m_console(console)
        , entire_text(file_in_name)
        , out(out)
        , markArea(area)
        , debug(debug)
        , listing(listing_file)
        , m_procType(procType)
    {
        init();
    }

#if 0
    CAsm6502(io::output &console)
        : m_console(console)
    {
        m_procType = ProcessorType::M6502;
        swapbin = false;
        forcelong = 0;
        init_members();
        temporary_out = false;
        check_line = true;
        text = NULL;
        pRept = NULL;
    }
#endif

    ~CAsm6502()
    {
        if (temporary_out)
           ::delete out;

        if (text)
            text->Fin(0);

        if (pRept)
            pRept->Fin(0);
    }

    // syntax check on 'str' line
    // in 'instr_idx_start' the returned position of the statement on the line or 0
    // in 'instr_idx_fin' the returned position of the end of the statement on the line or 0
    Stat CheckLine(const char *str, int &instr_idx_start, int &instr_idx_fin);

    void Abort()
    {
        abort_asm = true;
    }

    std::string GetErrMsg(Stat stat); // Error description

    Stat assemble(); // Assembly

    uint32_t GetProgramStart() const // Beginning of the program
    {
        return m_progStart;
    }
};

#endif /* M6502_H__ */
