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

    void HandleError(const char *msg)
    {
        params[++top].type = STRING_VALUE;
        params[top].sValue.CopyFromCString(msg);
        params[++top].type = INTEGER_VALUE;
        params[top].iValue = 1;
        JumpToSubroutineNoTrace(program[1+1]);
    }

    static void RaiseError(const char *msg)
    {
    }

    /// \desc Displays the lines of code in the program.
    static void DisplayASMCodeLines();

    /// \desc Calls one of the standard built in functions.
    void JumpToBuiltInFunctionNoTrace(DslValue *dslValue);

    /// \desc Calls one of the standard built in functions.
    void JumpToBuiltInFunctionTrace(DslValue *dslValue);

    /// \desc processes the switch jump instruction.
    /// \param dslValue Pointer to the index containing the JTB opcode.
    void ProcessJumpTableNoTrace(DslValue *dslValue);

    /// \desc processes the switch jump instruction.
    /// \param dslValue Pointer to the index containing the JTB opcode.
    void ProcessJumpTableTrace(DslValue *dslValue);

    /// \desc Processes function calls.
    void JumpToSubroutineNoTrace(DslValue *dslValue);

    /// \desc Processes function calls.
    void JumpToSubroutineTrace(DslValue *dslValue);

    /// \desc Runs the compiled program.
    void Run();

    /// \desc Runs and traces the execution of the compiled program.
    void RunTrace();

    /// \desc Runs the compiled program without trace information.
    void RunNoTrace();

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

    /// \desc Displays out the top of the parameter stack and the right and left values for an operation.
    void OutputValues(DslValue *left, DslValue *right) const;

    /// \desc Displays the results of any alu operation.
    static void PrintResult(DslValue *result);

/// \desc Displays a CPU instruction, used for testing and debugging.
/// \param addr address of the instruction to show.
static int64_t DisplayASMCodeLine(int64_t addr, bool newline = true);

/// \desc built in array of function pointers. Order is same as lexers built in function names list.
typedef void (CPU::*method_function)();

private:

    int64_t        BP; //stack frame register for local variables.
    int64_t        PC; //program instruction counter.
    Stack<int64_t> SP{};    //Call and return stack.
    //external params stack is used for performance.
    List<DslValue> params;  //function call parameters stack
    int64_t        top;   //top of params stack
    DslValue       *A;  //Temporary A register storage.

    static bool ReadFile(U8String *file, U8String *output);
    static void Error(DslValue *error);
    static bool IsMatch(u8chr ch, u8chr ex, bool caseLessCompare);
    static u8chr GetExChar(U8String *expression, int64_t &offset);
    static int64_t Find(U8String *search, U8String *expression, int64_t start);
    static void Sub(U8String *search, U8String *result, int64_t start, int64_t length);
    static int64_t ExpressionLength(U8String *expression);


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
