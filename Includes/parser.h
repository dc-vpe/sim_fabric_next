﻿#ifndef DSL_PARSER_H
#define DSL_PARSER_H

#include "dsl_types.h"
#include "SystemErrorHandlers.h"
#include "Hashmap.h"
#include "Lexer.h"
#include "Queue.h"



/// \desc Defines the parser component. The parser is a LR recursive descent parser.
class Parser
{
    enum ExitExpressionOn
    {
        EXIT_SEMICOLON_COMMA = 0,
        EXIT_PARAM_END,
        EXIT_CLOSE_PAREN,
        EXIT_IF_COND_END,
        EXIT_IF_BLOCK_END,
        EXIT_ELSE_BLOCK_END,
        EXIT_WHILE_COND_END,
        EXIT_WHILE_BLOCK_END,
        EXIT_FOR_INIT_END,
        EXIT_FOR_COND_END,
        EXIT_FOR_UPDATE_END,
        EXIT_FOR_BLOCK_END,
        EXIT_SWITCH_COND_END,
        EXIT_SWITCH_BLOCK_END,
        EXIT_CASE_COND_END,
        EXIT_CASE_BLOCK_END,
        EXIT_DEFAULT_BLOCK_END,
        EXIT_FUNCTION_DEF_END,
        EXIT_FUNCTION_CALL_END,
        EXIT_LOCATION
    };
public:
    static Token *GetVariableInfo(Token *token);

    static DslValue *OutputCount(int64_t count, int64_t moduleId);
    static DslValue *OutputCode(Token *token, OPCODES opcode);

    /// \desc Creates a parser instance with default settings.
    Parser()
    {
        position = {};
        end.type = END_OF_SCRIPT;
    }

    /// \desc Frees up the resource used by the parser.
    ~Parser()
        = default;

    /// \desc Parses the lexed tokens into IL ready to be runInterpreter by the CPU or LLVM back end.
    /// \return Returns true of no errors or warnings occurred, else false.
    bool Parse();

private:
    //tracks jmp locations that need fixed up when the position they jmp to is encountered.
    List<int64_t> jumpLocations;
    List<int64_t> ifJumpLocations;
    List<int64_t> continueLocations;

    /// \desc Stack of tokens that allow break to exit.
    ///       Token type contains the type switch begin, while begin, for begin

    List<Token *> breakableTokens;

    /// \desc position of encountered breaks.
    List<int64_t> breakLocations;

    List<Token *> switches;

    /// \desc Function call stack used while processing function calls in expression.
    List<Token *> functionCalls;

    /// \desc Shunting yard function outputs the tokens in this queue in the
    ///       correct order to be written to the program list.
    Queue<Token *> output;

    ///\desc The current parsing position within the lexed list of tokens.
    int64_t position;

    /// \desc Token that means end of the lexed program scripts being parsed.
    Token end;

    /// \desc Checks if the position is in range of the tokens in the program token list.
    static bool IsPositionInRange(int64_t pos) { return pos >= 0 && pos < tokens.Count(); }
    Token *Peek(int64_t offset = 1);
    static void PushValue(Token *token);
    static void CreateVariable(Token *token);
    static void CreateOperation(Token *token);
    static void FixUpJumpsToEnd();
    static void FixUpFunctionCalls();
    Token *Expression(int64_t tokenLocation = -1);
    Token *ShuntingYard(Token *token, int64_t tokenLocation);
};

#endif
