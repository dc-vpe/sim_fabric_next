#ifndef DSL_PARSER_H
#define DSL_PARSER_H

#include "dsl_types.h"
#include "Hashmap.h"
#include "Lexer.h"
#include "Queue.h"

/// \desc Index which is the id of the built-in print function. Used as default on error if
///       on error is not defined for the module.
#define PRINT_FUNCTION_ID 29


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
        EXIT_FUNCTION_CALL_END
    };
public:
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
    ///\desc The current parsing position within the lexed list of tokens.
    int64_t position;

    /// \desc Token that means end of the lexed program scripts being parsed.
    Token end;

    /// \desc Checks if the position is in range of the tokens in the program token list.
    static bool IsPositionInRange(int64_t pos) { return tokens.Count() > 0 && pos >= 0 && pos < tokens.Count(); }

    Token *Advance(int64_t offset = 1);
    Token *Peek(int64_t offset = 1);
    static Token *PushValue(Token *token);
    static Token *CreateVariable(Token *token);
    Token *ProcessFunctionCall(Token *token);
    static void IncrementOptimization();
    static Token *CreateOperation(Token *token);
    static void FixUpJumpsToEnd();
    static void FixUpFunctionCalls();
    Token *ProcessSwitchStatement(Token *token);
    Token *Expression(ExitExpressionOn exitExpressionOn);
    Token *ShuntingYard(ExitExpressionOn exitExpressionOn, Token *token, Queue<Token *> *output);
    static bool ExitExpression(ExitExpressionOn exitExpressionOn, TokenTypes type);
};

#endif
