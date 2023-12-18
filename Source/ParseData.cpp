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
LocationInfo locationInfo;

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

/// \desc In strict mode variables are not promoted to collections and the fields must match.
///       In non strict mode the default, variables are promoted and field checking does not
///       take place.
bool strict = false;

/// \desc total number of standard functions,
///       update when adding or removing standard functions.
int64_t totalStandardFunctions = 28;

/// \desc standard built in function names.
const char *standardFunctionNames[] =
{
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
