//
// Created by krw10 on 8/10/2023.
//
#include "../Includes/List.h"
#include "../Includes/DslValue.h"
#include "../Includes/Token.h"
#include "../Includes/Hashmap.h"
#include "../Includes/Module.h"

/// \desc output of parser.
List<DslValue *> program;

/// \desc modules that make up the program.
List<Module *> modules;

/// \desc List of tokens created by the lexers.
List<Token *> tokens;

/// \desc Hashmaps containing the variables that have been defined in TokenModifier order
Hashmap variables;

/// \desc Hashmaps containing the functions that have been defined.
Hashmap functions;

/// \desc standard functions provided by the DSL.
Hashmap standardFunctions;

/// \desc standard variables provided by the DSL.
Hashmap standardVariables;

WarningLevels warningLevel;
int64_t errors;     //number of errors, if zero no errors happened.
int64_t warnings;   //number of warnings, if 0 no warnings happened.
bool fatal; //True if a fatal error occurs.

/// \desc Current lexer location information.
LocationInfo locationInfo;

/// \desc Previous lexer location information.
LocationInfo  previousInfo;

//Total test cases run.
int64_t total_run = 0;

//Total test cases failed.
int64_t total_failed = 0;

//total test cases passed.
int64_t total_passed = 0;

/// \desc Lexer information level to display.
int64_t lexerInfoLevel = 0;

/// \desc Parse information level to display.
int64_t parserInfoLevel = 0;

/// \desc Run time trace information level to display.
int64_t traceInfoLevel = 0;

/// \desc total number of standard functions,
///       update when adding or removing standard functions.
int64_t totalStandardFunctions = 38;

/// \desc standard built in function names.
const char *standardFunctionNames[] =
{
    "string.find",
    "string.len",
    "string.sub",
    "string.replace",
    "string.tolower",
    "string.toupper",
    "string.trimEnd",
    "string.trimStart",
    "string.toCollection",
    "string.fromCollection",
    "abs",
    "acos",
    "asin",
    "atan",
    "atan2",
    "cos",
    "sin",
    "tan",
    "cosh",
    "sinh",
    "tanh",
    "exp",
    "log",
    "log10",
    "sqrt",
    "ceil",
    "fabs",
    "floor",
    "fmod",
    "print",
    "printf",
    "input",
    "read",
    "write",
    "files",
    "delete",
    "random",
    "seed"
};

int64_t standardFunctionParams[]=
{
        3, //string.find,
        1, //string.len,
        3, //string.sub,
        3, //string.replace,
        1, //string.tolower,
        1, //string.toupper,
        2, //string.trimEnd,
        2, //string.trimStart,
        1, //string.toCollection,
        1, //string.fromCollection,
        1, //abs,
        1, //acos,
        1, //asin,
        1, //atan,
        2, //atan2,
        1, //cos,
        1, //sin,
        1, //tan,
        1, //cosh,
        1, //sinh,
        1, //tanh,
        1, //exp,
        1, //log,
        1, //log10,
        1, //sqrt,
        1, //ceil,
        1, //fabs,
        1, //floor,
        2, //fmod,
        1, //print,
        1, //printf,
        0, //input,
        1, //read,
        2, //write,
        1, //files,
        1, //delete,
        2, //random,
        1, //seed
};
