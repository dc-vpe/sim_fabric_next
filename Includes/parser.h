#ifndef DSL_PARSER_H
#define DSL_PARSER_H

#include "dsl_types.h"
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
    //tracks jmp locations that need fixed up when the location they jmp to is encountered.
    List<int64_t> jumpLocations;
    List<int64_t> continueLocations;
    List<int64_t> breakLocations;

    //Function calls have to be put into the queue when parsed so that they end up
    //in the correct position for expression evaluation. However, they can't be
    //inserted directly into the program when encountered. So they are parsed
    //into their own queue for processing.
    List<Queue<DslValue> *> queuedFunctionCalls;

    ///\desc The current parsing position within the lexed list of tokens.
    int64_t position;

    /// \desc Token that means end of the lexed program scripts being parsed.
    Token end;

    /// \desc Checks if the position is in range of the tokens in the program token list.
    static bool IsPositionInRange(int64_t pos) { return pos >= 0 && pos < tokens.Count(); }

    Token *Advance(int64_t offset = 1);
    Token *Peek(int64_t offset = 1);
    static void PushValue(Token *token);
    static void CreateVariable(Token *token);
    Token *ProcessFunctionCall(Token *token);
    void QueueFunctionCall(Token *token, Queue<Token *> *output);
    static void IncrementOptimization();
    static Token *CreateOperation(Token *token);
    static void FixUpJumpsToEnd();
    static void FixUpFunctionCalls();
    Token *Expression(ExitExpressionOn exitExpressionOn, int64_t tokenLocation = -1);
    Token *ShuntingYard(ExitExpressionOn exitExpressionOn, Token *token, Queue<Token *> *output, int64_t tokenLocation);
    [[nodiscard]] bool ExitExpression(ExitExpressionOn exitExpressionOn, TokenTypes type, int64_t tokenLocation) const;
};

#endif
