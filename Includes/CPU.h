//
// Created by krw10 on 8/25/2024.
//
//


#ifndef DSL_CPP_CPU_H
#define DSL_CPP_CPU_H

#include "DslValue.h"
#include "ParseData.h"
#include "../Includes/Stack.h"
#include "../Includes/List.h"

/// \desc TICKS are every 1/10th of a second.
#define TICKS_PER_SECOND 100

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
    static void DisplayASMCodeLines();

    /// \desc Initializes the CPU to run the program.
    /// \param mode If True the program has been compiled in debug mode and contains debug information.
    /// \param ilFile Pointer to a u8String containing the full path name to the compiled il program.
    bool Init(bool mode, U8String *ilFile);

    /// \desc Runs the compiled program.
    bool Run();

    /// \desc Displays a CPU instruction, used for testing and debugging.
    /// \param addr address of the instruction to show.
    static int64_t DisplayASMCodeLine(int64_t addr, bool newline = true);

    /// \desc built in array of function pointers. Order is same as lexers built in function names list.
    typedef void (CPU::*method_function)();

private:

    int64_t        BP; //stack frame register for local variables.
    int64_t        PC; //program instruction counter.
    Stack<int64_t> SP{};    //Call and return stack.
    List<DslValue> params;  //function call parameters stack
    int64_t        top;   //top of params stack
    DslValue       *A;  //Temporary A register storage.
    long           nextTick; //next on text time
    int64_t        lastModuleId; //last module id changes with the instruction being executed
    int64_t        onTickEvent; //tracks the last tick event set, this changes when the instructions module changes.
    bool           debugMode; //If true the program is compiled with debug information.


    List<Byte>          IL; //Bytes that make up the program.
    List<DslValue *>    instructions;   //The actual instructions that are run.

    static int64_t  errorCode;
    static U8String szErrorMsg;
    List<DslValue>  eventHandlers; //Event function information in module id order.

    static bool ReadFile(U8String *file, U8String *output);
    static void Error(DslValue *error);
    static bool IsMatch(u8chr ch, u8chr ex, bool caseLessCompare);
    static u8chr GetExChar(U8String *expression, int64_t &offset);
    static int64_t Find(U8String *search, U8String *expression, int64_t start);
    static void Sub(U8String *search, U8String *result, int64_t start, int64_t length);
    static int64_t ExpressionLength(U8String *expression);
    void SetTickEvent(DslValue *dslValue);

    /// \desc Calls one of the standard built in functions.
    void JumpToBuiltInFunction(DslValue *dslValue);

    /// \desc Calls a script function.
    /// \param dslValue Pointer to the dsl value containing the information needed to call script function.
    void JumpToSubroutine(DslValue *dslValue);

    /// \desc processes the switch jump instruction.
    /// \param dslValue Pointer to the index containing the JTB opcode.
    void ProcessJumpTable(DslValue *dslValue);

    /// \desc Gets the next integer value from the input IL vyte stream.
    /// \param position Reference to the position position within the input stream.
    int64_t GetInt(int64_t &position);

    /// \desc Gets the next double from the input IL byte stream.
    /// \param position Reference to the position position within the input stream.
    double GetDouble(int64_t position);

    /// \desc Gets the next string from the input IL byte stream.
    /// \param position Reference to the position position within the input stream.
    /// \param u8String Pointer to the string to be filled in with the decoded characters.
    void GetString(int64_t &position, U8String *u8String);

    /// \desc Gets the next value from the input list of bytes.
    /// \param position Current position in the input list.
    /// \param dslValue Returns value.
    void GetValue(int64_t &position, DslValue *dslValue);

    /// \desc Reads a list of bytes and translates them into dsl value runnable instructions.
    void DeSerialize();

    //Gets the location id for the event handler for the current script module.
    int64_t GetEventLocation(SystemErrorHandlers errorHandler, int64_t moduleId);

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
    /// \param right Right side of the instruction for binary instructions or location
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


//Standard library function.
public:
    /// \desc First call gets the total number of parameters passed to the function.
    /// \return Gets the total number of parameters passed to the external function.
    /// \Remark These functions form the API for accessing parameters passed to an
    ///         external function.
    int64_t GetTotalParams();

    /// \desc Gets a parameter passed to the external function.
    /// \param totalParams total number of parameters passed to the function, this
    ///                    value is returned from Get Total Parameters.,
    /// \param index       Index of the parameter to get. Parameters begin at 0
    ///                    and are passed from left to right in CDECL style.
    DslValue *GetParam(int64_t totalParams, int64_t index);

    /// \desc Returns the provided result to the run time.
    /// \param totalParams total number of parameters passed to the function, this
    ///                    value is returned from Get Total Parameters.,
    /// \param returnValue Pointer to the dslValue to return from the function. If
    ///                    the external function does not return a value then this
    ///                    should be set to INTEGER_VALUE of 0.
    void ReturnResult(int64_t totalParams, DslValue *returnValue);

    /// \desc Write local file.
    void WriteFile(U8String *fileName, int64_t totalParams);

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
