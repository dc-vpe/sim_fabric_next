//
// Created by krw10 on 8/25/2024.
//
// Software CPU forms the runtime engine for the DSL

#ifndef DSL_CPP_CPU_H
#define DSL_CPP_CPU_H

#include <cmath>
#include <cctype>
#include <dirent.h>
#include <ctime>
#ifdef __linux__
#include <cerrno>
#endif
#include "DslValue.h"
#include "SystemErrorHandlers.h"
#include "Stack.h"
#include "List.h"
#include "BinaryFileReader.h"
#include "JsonParser.h"


/// \desc TICKS are every 1/10th of a second.
#define TICKS_PER_SECOND 100

/// \desc The CPU class forms a software CPU runtime engine for a compiled program. The CPU
///       reads a serialized set if intermediate language instructions consisting of an
///       opcode and any data needed by the opcode to perform its function. The deserialized
///       instructions are stored in an instructions list which is then executed in a manner
///       similar to how a hardware CPU would run.
class CPU
{
public:
    /// \desc default constructor simply sets the registers and flag to their default value.
    CPU();

    /// \desc frees resources used by the CPU.
    ~CPU()
    {
        delete A;
    }

    /// \desc Raises an error which puts the cpu into error mode
    ///       and calls the on error event handler if one is
    ///       present. If an error handler for the current
    ///       module is not present the default error handler is
    ///       called.
    static void RaiseError(int64_t errorId, const char *msg)
    {
        errorCode = errorId;
        szErrorMsg.CopyFromCString(msg);
    }

    /// \desc Displays the lines of code in the program.
    /// \param programInstructions list of dsl values containing the compiled program.
    static void DisplayASMCodeLines(List<DslValue *> &programInstructions);

    /// \desc Initializes the CPU to run the program.
    /// \param ilFile Pointer to a u8String containing the full path name to the compiled il program.
    /// \param symFile Pointer to u8String containing the full path name to the symbol file name.
    bool Init(U8String *ilFile, U8String *symFile);

    /// \desc Runs the compiled program.
    bool Run();

    /// \desc Displays a CPU instruction, used for testing and debugging.
    /// \param addr address of the instruction to show.
    static int64_t DisplayASMCodeLine(List<DslValue *> &programInstructions, int64_t addr, bool newline = true);

    /// \desc built in array of function pointers. Order is same as lexers built in function names list.
    typedef void (CPU::*method_function)();

    /// \desc This list is the parameter stack used for calculations and parameter passing to string and
    ///       native functions.
    List<DslValue> params;

    /// \desc Top of the parameter stack.
    /// \remark The reason a separate top of stack value is used instead
    ///         of the push back pop back method within list is for performance. Using a separate
    ///         top of stack indicator allows calculations to be processed directly which measured
    ///         out to be 10x faster as calculation data can be processed in place without the
    ///         need for additional stack management instructions.
    int64_t top;

    /// \desc The default return value register. Used for temporary value storage.
    /// /\remark This value can be overwritten so assume this register is only valid
    ///           within the current function or method in which it is used.
    DslValue *A;

private:
    /// \desc Stack frame for local variables defined, passed and used within DSL function calls.
    int64_t BP;

    /// \desc Instruction pointer, always points at the next instruction to be executed.
    int64_t PC;

    /// \desc Time at which the next on tick event call should occur.
    long nextTick;

    /// \desc Module id of the last instruction to be executed.
    int64_t lastModuleId;

    /// \desc The cached index of the currently set on tick function location. This is changed right
    ///       before the instruction which has a different module id is executed.
    int64_t onTickEvent;

    /// \desc This list contains the instructions that make up the program that is being executed by
    ///       this CPU. Multiple CPUs can all be running programs at the same time without
    ///       without conflicting with each other.
    List<DslValue *> instructions = {};   //The deserialized program instructions to execute.

    /// \desc last error code that was raised.
    static int64_t  errorCode;

    /// \desc last error message that was raised.
    static U8String szErrorMsg;

    /// \desc Reads a file current using the local file system. The file is returned
    ///       int the A dsl value. In case of an error the A dsl value will contain
    ///       an error.
    /// \param file position to read the file from.
    /// \param output U8String to write the contents of the file to.
    /// \return True if successful, else false if an error occurs.
    static bool ReadFile(U8String *file, U8String *output);

    /// \desc Raised an error event, currently only prints but will be changed as soon
    ///       as the eventing system is in place.
    static void Error(DslValue *error);

    /// \desc Checks if the ch character is a match for the expression character.
    /// \param ch character to check.
    /// \param ex expression character that defines how the ch character should be checked.
    /// \param caseLessCompare True if the compare should occur in a case agnostic manner,
    ///                        or false if the compare should occur in a case sensitive manner.
    static bool IsMatch(u8chr ch, u8chr ex, bool caseLessCompare);

    /// \desc Gets an expression character from an expression string.
    /// \param expression U8String containing the search expression string.
    /// \param offset Offset within the expression string at which to get the character.
    ///               The offset is set to the next expression character upon return.
    /// \return The expression character without the leading % escape character.
    static u8chr GetExChar(U8String *expression, int64_t &offset);

    /// \desc Checks to see if the expression is contained in the search string beginning at the start
    ///       character location.
    /// \param search Pointer to a U8String containing the characters to be searched.
    /// \param expression Pointer to a U8String containing the expression that specifies the expression
    ///                   to be searched for.
    /// \param start The character location within the search string at which the search should begin.
    /// \returns The character location within search that the expression was found or -1 if the
    ///          expression was not found beginning at start up to the end of the search string.
    /// \remarks The expression string consists of characters with the % character used to
    ///          indicate a special type of compare.
    ///          The % character is used in the expression string to indicate that the next character
    ///          represents one of the character sets in the table shown here:
    ///             Code	Description	            Finds characters	            C equivalent function
    ///             %C	    Control characters	    0x00 though 0x1F and 0x7F	    Iscntrl()
    ///             %B	    Blank characters	    ‘\t’ and ‘ ‘	                Isblank()
    ///             %S	    Space characters	    ‘\t’, ‘\n’, ‘\f’, ‘\v’, ‘\r’    Isspace()
    ///             %U	    Upper case letter	    ABCDEFGHIJKLMNOPQRSTUVWXYZ	    Isupper()
    ///             %u	    Lower case letter	    abcdefghijklmnopqrstuvwxyz	    Islower()
    ///             %A	    Any letter	ABCDEFGHIJKLMNOPQRSTUVWXYZ
    ///                                 abcdefghijklmnopqrstuvwxyz	                Isalpha()
    ///             %D	    Any digit	0123456789	                                Isdigit()
    ///             %N	    Any letter or digit	ABCDEFGHIJKLMNOPQRSTUVWXYZ
    ///                                         Abcdefghijklmnopqrstuvwxyz
    ///                                         0123456789	                        Isalnum()
    ///             %P	Any punctuation	!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~	        Ispunct()
    ///             %G	Any character that can be seen when displayed.
    ///                 !"#$%&'()*+,-.0123456789:;<=>?
    ///                 @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`
    ///                 Abcdefghijklmnopqrstuvwxyz{|}~	                            Isgraph()
    ///             %p	Any printable character	!"#$%&'()*+,-./0123456789
    ///                 :;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`
    ///                 Abcdefghijklmnopqrstuvwxyz{|}~	                            Isprint()
    ///             %X	Any hexadecimal digit	0123456789ABCDEFabcdef.             Isxdigit()
    ///             %?	Wild card matches any character at its position.
    static int64_t Find(U8String *search, U8String *expression, int64_t start);

    /// \desc Creates a sub string from the search string.
    /// \param search String containing the characters from which to build the sub string.
    /// \param result String that will contain the sub string.
    /// \param start Starting character location in the search string.
    /// \param length Number of characters to copy from the search string to the result string.
    static void Sub(U8String *search, U8String *result, int64_t start, int64_t length);

    /// \desc Calculates the number of characters in a search expression.
    /// \param expression Pointer to a U8String that contains the regular expression.
    /// \return The number of characters in the search expression string.
    static int64_t ExpressionLength(U8String *expression);

    /// \desc Ensures that the on tick event pointer is set to the correct module function.
    /// \param dslValue Instruction about to be run.
    void SetTickEvent(DslValue *dslValue);

    /// \desc Deserializes the program symbol file and adds those symbols to the
    ///       internal program.
    /// \param binaryFileReader Pointer to the binary file reader to use.
    /// \return True if the symbols are added to the program, false if the symbol
    ///         file does not exist or can't be read.
    bool DeSerializeSymbols(BinaryFileReader *binaryFileReader);

    /// \desc Adds the locations used in the program as its symbols. Used when the symbol file
    ///       can't be read or is not present.
    void SetProgramLocationsAsSymbols();

    /// \desc Calls one of the standard built in functions.
    void JumpToBuiltInFunction(DslValue *dslValue);

    /// \desc Calls a script function.
    /// \param dslValue Pointer to the dsl value containing the information needed to call script function.
    void JumpToSubroutine(DslValue *dslValue);

    /// \desc processes the switch jump instruction.
    /// \param dslValue Pointer to the index containing the JTB opcode.
    void ProcessJumpTable(DslValue *dslValue);

    /// \desc Reads a list of bytes and translates them into dsl value runnable instructions.
    void DeSerializeIL(BinaryFileReader *binaryFileReader);

    //Gets the position id for the event handler for the current script module.
    static int64_t GetEventLocation(SystemErrorHandlers errorHandler, int64_t moduleId);

    /// \desc Handles on error events.
    void JumpToOnErrorHandler();

    /// \desc Executes the single instruction in dsl value.
    /// \param dslValue Instruction to be executed.
    bool RunInstruction(DslValue *dslValue);

    /// \desc Called when its time to call the on tick handler.
    void JumpToOnTick();

    /// \desc Runs and traces the execution of the compiled program.
    void RunTrace();

    /// \desc Runs the compiled program without trace information.
    void RunNoTrace();

    /// \desc Gets the operands for an instruction. Only active when the trace flag is
    ///       set to 1.
    /// \param instruction Pointer to the next instruction to be executed.
    /// \param left Left side of the instruction for binary instructions or instruction
    ///             for unary or no operand instructions.
    /// \param right Right side of the instruction for binary instructions or position
    ///              for conditional jump instructions.
    int64_t GetInstructionOperands(DslValue *instruction, DslValue *left, DslValue *right);

    /// \desc Extends the number of elements in a collection at runtime.
    /// \param collection Collection to extend.
    /// \param newEnd New end of the collection.
    static void ExtendCollection(DslValue *collection, int64_t newEnd);

    /// \desc Sets the collection element referenced in the dsl value with the value on
    ///       the top of the parameter stack. This is used when dynamically initializing
    ///       a collection with non-static expressions.
    /// \param dslValue Pointer to the dslValue value that contains the collection and
    ///                 element to set information.
    void SetCollectionElementDirect(DslValue *dslValue);

    /// \desc Gets the referenced element in a collection.
    /// \param dslValue Pointer to the dslValue containing the collection.
    /// \return The referenced element in the collection.
    /// \remark This call can extend a collection.
    DslValue *GetCollectionElement(DslValue *dslValue);

    /// \desc Evaluates the variable instruction and pushes a dsl value
    ///       that contains the address of the dsl value to be updated by
    ///       a store instruction.
    /// \param variable Pointer to the variable containing the push variable address instruction.
    void PushVariableAddress(DslValue *variable);

    /// \desc Write local file.
    void WriteFile(U8String *fileName, int64_t totalParams);

public:

    ///////////////////////////////////////////////////////////////////
    // Built-in functions that are considered part of the language. ///
    ///////////////////////////////////////////////////////////////////
    void pfn_string_find();
    void pfn_string_len();
    void pfn_string_sub();
    void pfn_string_replace();
    void pfn_string_tolower();
    void pfn_string_toupper();
    void pfn_string_trimEnd();
    void pfn_string_trimStart();
    void pfn_string_toCollection();
    void pfn_string_fromCollection();
    void pfn_abs();
    void pfn_acos();
    void pfn_asin();
    void pfn_atan();
    void pfn_atan2();
    void pfn_cos();
    void pfn_sin();
    void pfn_tan();
    void pfn_cosh();
    void pfn_sinh();
    void pfn_tanh();
    void pfn_exp();
    void pfn_log();
    void pfn_log10();
    void pfn_sqrt();
    void pfn_ceil();
    void pfn_fabs();
    void pfn_floor();
    void pfn_fmod();
    void pfn_print();
    void pfn_printf();
    void pfn_input();
    void pfn_read();
    void pfn_write();
    void pfn_files();
    void pfn_delete();
    void pfn_random();
    void pfn_seed();
};


extern const char *OpCodeNames[];

#endif //DSL_CPP_CPU_H