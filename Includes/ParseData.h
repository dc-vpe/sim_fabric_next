//
// Created by krw10 on 8/10/2023.
//
// Defines and allows access to the common data passed between lexers and the parser.
#ifndef DSL_CPP_PARSE_DATA_H
#define DSL_CPP_PARSE_DATA_H

#include "Hashmap.h"
#include "WarningLevels.h"
#include "List.h"
#include "Token.h"
#include "Module.h"

/// \desc List of tokens created by the lexers. This is added to
///       by each lexer. The resultant list of tokens contains
///       all of the information the parser requires to produce
///       the AST.
extern List<Token *> tokens;

/// \desc Contains the compiled IL code from the parser.
extern List<DslValue *> program;

/// \desc modules that make up the program.
extern List<Module *> modules;

/// \desc list of components that have been defined.
extern List<ComponentData *> componentsData;

//Hashmaps containing the variables that have been defined in TokenModifier order
extern Hashmap variables;

//Hashmaps containing the functions that have been defined.
extern Hashmap functions;

/// \desc Standard functions provided by the DSL.
extern Hashmap standardFunctions;

/// \desc Standard variables provided by the DSL.
extern Hashmap standardVariables;

extern WarningLevels warningLevel;
extern int64_t           errors;     //number of errors, if zero no errors happened.
extern int64_t           warnings;   //number of warnings, if 0 no warnings happened.
extern bool          fatal; //True if a fatal error occurs.

/// \desc current position information.
extern LocationInfo  locationInfo;

/// \desc Previous position information.
extern LocationInfo  previousInfo;

//Total test cases run.
extern int64_t total_run;

//Total test cases failed.
extern int64_t total_failed;

//total test cases passed.
extern int64_t total_passed;

/// \desc Lexer information level to display.
extern int64_t lexerInfoLevel;

/// \desc Parse information level to display.
extern int64_t parserInfoLevel;

/// \desc Run time trace information level to display.
extern int64_t traceInfoLevel;

/// \desc Total number of standard (built in)  functions available.
extern int64_t totalStandardFunctions;

/// \desc Names of the standard built in functions.
extern const char *standardFunctionNames[];

/// \desc Number of required parameters for build in functions.
extern const int64_t standardFunctionParams[];

/// \desc List of currently supported run time system.
extern List<U8String *> systemEventNames;

/// \desc Full path name of the output file to generate.
extern U8String outputFile;

/// \desc symbol file, contains the debugging symbols not needed for runtime.
extern U8String symbolFile;

#endif //DSL_CPP_PARSE_DATA_H
