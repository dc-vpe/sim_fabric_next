//
// Created by krw10 on 5/26/2023.
//
// This _f defines the lexer class. The lexer is responsible for
// converting the source lexerVariableDefinitions into a list of defined tokens
// and hashmap of variables, and functions which combined contain
// all the information necessary to build the IR code.
//

#ifndef DSL_LEXER_H
#define DSL_LEXER_H

#include "dsl_types.h"
#include "Hashmap.h"
#include "Token.h"
#include "../Includes/KeyWords.h"
#include "../Includes/LocationInfo.h"
#include "../Includes/Stack.h"
#include "../Includes/Queue.h"
#include "WarningLevels.h"
#include "ParseData.h"

/// \desc Checks if the text character is a number 0 though 9 inclusive.
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

/// \desc Checks if the text character is a period.
#define IS_PERIOD(c) ((c) == '.')

/// \desc Checks if the text character is a number 0 to 9 or a decimal point.
#define IS_NUMBER(c) (IS_DIGIT(c) || IS_PERIOD(c))

/// \desc Checks if the text character is an upper or lower case letter.
#define IS_LETTER(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >='a' && (c) <= 'z'))

/// \desc Checks if the text character is an extended UTF8 character.
#define IS_U8_EXTENDED(c) ((c) >= 127)

/// \desc Checks if the text character is an underscore character.
#define IS_UNDERSCORE(c) ((c) == '_')

/// \desc definition of the start of an identifier.
/// \remarks The start of an identifier is any letter, underscore, or utf8 extended character.
///          non-printable characters or special characters are not allowed to be identifier
///          characters per common code language convention and the fact these characters
///          are often used for operators simplifying lexing.
///          [UNDERSCORE | ALPHA | UTF8_EXTENDED]
#define IS_ID_START(c) (IS_LETTER(c) || IS_UNDERSCORE(c) || IS_U8_EXTENDED(c))

/// \desc definition of an identifier. An identifier can consist of numbers and the start of identifier characters.
#define IS_IDENTIFIER(c) (IS_ID_START(c) || IS_NUMBER(c))

/// \desc Checks if the token is a valid target for a unary operation.
#define IS_VALID_UNARY_TARGET(_tokenType)   (IS_VALUE_TYPE(_tokenType) ||\
                                      (_tokenType) == OPEN_PAREN ||\
                                      (_tokenType) == UNARY_NOT ||\
                                      (_tokenType) == UNARY_POSITIVE ||\
                                      (_tokenType) == UNARY_NEGATIVE)

/// \desc Checks if the type is a token that can be assigned to.
#define IS_ASSIGN_TYPE(type) ((type) == VARIABLE_VALUE || (type) == VARIABLE_DEF || (type) == PARAMETER_VALUE)

/// \desc This structure defines the possible ways in which binary and unary operators can function.
typedef enum OperatorSubTypeEnum
{
    /// \desc Error could not determine prefix or postfix.
    TypeError = 0, /// \desc The operator is a unary operator requiring a single value.
    TypeUnary = 1, /// \desc The operator is a binary operator requiring 2 values.
    TypeBinary = 2, /// \desc The operator is a postfix operator and is after the value or variable to be acted upon.
    TypePostfix = 3, /// \desc The operator is a prefix operator and precedes the value or variable to be acted upon.
    TypePrefix = 4 //Prefix operator
} OperatorSubType;

/// \desc Lexer translates a source code cText into a list of tokens containing the information
///       needed to pass to the parser to generate the llvm compatible AST.
class Lexer
{
private:
    //Current source being lexed.
    U8String parseBuffer;

    U8String *tmpBuffer; //temporary buffer for a general work area.

    bool definingFunction;       //True if a function is being defined, else false.

    U8String currentFunction;   //Current function being lexed.

    /// \desc True if a functions parameter list is being defined.
    bool definingFunctionsParameters;

    /// \desc The number of parameter stack variables a script function requires.
    int64_t stackVariables;

    DslValue *tmpValue; //temporary value structure for converted values.

    TokenTypes last; //Last token type returned from get next token type
    TokenTypes prev; //Previous token type return from get next token type

public:
    /// \desc Gets the total number of tokens in the list.
    [[nodiscard]] static int64_t TotalTokens()
    { return tokens.Count(); }

    /// \desc Gets the Type of a token.
    static TokenTypes GetTokenType(int64_t index)
    {
        if (index >= 0 && index < tokens.Count())
        {
            return tokens[index]->type;
        }
        return INVALID_TOKEN;
    }

    /// \desc Creates a lexer and returns a pointer to it. See header for detailed description.
    Lexer();

    /// \desc Destructor removes allocated resources when the Lexer is deleted.
    ~Lexer();

    /// \desc Initializes the lexer for a lex operation.
    static void Initialize();

    /// \desc Adds a module to the list of modules that make up a program.
    /// \return ID that represents the module. Pass this it Lex to translate it into
    ///         tokens and detect any error conditions.
    static int64_t AddModule(const char *moduleName, const char *file, const char *script);

    /// \desc Processes and translates all of the added modules.
    bool Lex();

private:
    /// \desc Name of current module being lexed.
    U8String module;

    /// \desc true if the entire _s has been processed.
    bool finished;

    /// \desc top level parenthesis count. self makes sure they are balanced.
    int64_t parenthesis = 0;

    /// \desc Used when defining a variable or function, contains the modifier type if any.
    TokenModifiers modifier;

    /// \desc Used when defining a variable or function, tracks if the var key word was specified.
    bool varSpecified;

    /// \desc Used when defining a variable or function, tracks if the const modifier was specified.
    bool constSpecified;

    /// \desc True when a collection element is being defined.
    bool isCollectionElement;

    /// \desc True when a new variable's assigment is being processed.
    bool definingAndAssigningVariable;

    /// \desc Used when defining a code block for a function, tracks the number of open and close { }
    ///       to determine when the function ends.
    int64_t blockCount;

    /// \desc Stores the full path name of a variable, if one is found when getting the next token type.
    /// \remark The full path name is composed of its scope, module, function if local, and variable name.
    U8String fullVarName;

    /// \desc Stores the full path name of a function, if one is found when getting the next token type.
    /// \remark The full path name is composed of its scope, module, function if local, and variable name.
    U8String fullFunName;

    /// \desc Tracks the parameters of the tok function being compiled.
    Hashmap parameters;

    u8chr GetCurrent();

    u8chr GetNext();

    //Temporary values used for value stack when calculating static expressions.
    List<DslValue *> tmpValues;

    bool Lex(int64_t id);
    static bool IsHexDigit(u8chr ch);
    static int64_t ConvertHexDigit(u8chr ch);
    static bool IsOperatorChar(u8chr ch);
    static bool IsValidUnaryPreviousToken(TokenTypes type);
    static bool IsValidStringEscape(char ch);
    TokenTypes GetOperatorToken(bool ignoreErrors);
    u8chr SkipWhiteSpace();
    u8chr PeekNextChar();
    bool GetIdentifier();
    TokenTypes GetKeyWordTokenType(bool ignoreErrors);
    bool IsSingleLineComment(u8chr ch);
    bool IsMultiLineComment(u8chr ch);
    bool GetNumber(bool ignoreErrors);
    u8chr GetEscapeValue(int64_t factor);
    bool ProcessEscapeCharacter(u8chr &ch, bool ignoreErrors);
    bool GetStringDirect(bool ignoreErrors);
    bool GetString(bool ignoreErrors);
    int CheckFunctionNamedParameter();
    bool CheckFunctionDefinitionSyntax(Token *token);
    bool DefineFunction(Token *token);
    static bool ProcessOperation(Stack<DslValue *> &values, TokenTypes type, bool ignoreErrors);
    bool DefineFunctionParameters();
    bool ValidateAndGetFullName(Token *token, bool ignoreErrors);
    bool ProcessFunctionDefinition(Token *token);
    bool DefineVariable();
    bool PushTmpValue(Stack<DslValue *> &values, DslValue *dslValue, int64_t &top);
    bool GetExpressionEnd(bool ignoreErrors, LocationInfo &end);
    static bool IsStaticType(TokenTypes type);
    bool IsStaticExpression(LocationInfo end);
    bool ProcessStaticExpression(DslValue *dslValue, LocationInfo end, bool ignoreErrors, bool initializeVariable);
    bool ProcessSingleAssignmentExpression(Token *token, DslValue *dslValue, bool &isStaticExpression, int64_t index);
    bool DefineSingleVariableAssignment(TokenTypes type, Token *token);
    bool IsCollectionAssignment();
    static void AddEmptyCollectionElement(Token *token, U8String *key, int64_t &index);
    static void GenerateKey(Token *token, U8String *key, int64_t &index);
    TokenTypes GetKey(Token *token, U8String *key, int64_t &index);
    bool DefineCollection(Token *token);
    bool DefineIf();
    bool CheckWhileSyntax();
    bool DefineWhile();
    bool DefineCond(const char *szErrorMsg);
    bool DefineStatementBlock(LocationInfo start, TokenTypes blockStart, TokenTypes blockEnd);
    bool DefineStatements(LocationInfo end, bool noStatements = false);
    bool DefineSwitch();
    bool CheckForSyntax(LocationInfo &init, LocationInfo &cond, LocationInfo &update, LocationInfo &block, LocationInfo &end);
    bool DefineForSection(TokenTypes beginToken, TokenTypes endToken, LocationInfo start, LocationInfo end);
    bool DefineFor();
    bool SkipToEndOfBlock(LocationInfo start);
    bool AddVariableValue();
    static bool ExpressionContinue(TokenTypes type);
    bool CheckFunctionCallSyntax();
    int64_t CountFunctionArguments();
    TokenTypes AddFunctionCall();
    bool GetSingleLineComment();
    TokenTypes LexParam();
    bool GetMultiLineComment();
    bool GetCharacterValue(bool ignoreErrors);
    TokenTypes SkipToEndOfStatement();
    TokenTypes PeekNextTokenType(int64_t skip=1, bool checkColon = false);
    u8chr GetNextNonCommentCharacter();
    void SkipNextTokenType(bool checkColon = false);
    bool IsNumber(u8chr ch);
    TokenTypes GetNextTokenType(bool ignoreErrors = false, bool checkForColon = false);
    void GetFullName(U8String *fullName, TokenModifiers scope);
    bool IsVariableDefined(bool ignoreErrors);
    bool IsFunctionDefined(bool ignoreErrors);
    bool CheckIfFunctionDefined();
    u8chr SkipToNextNonWhiteSpaceCh();
    bool SkipToCharacterBeforeCharacter(u8chr toCh, u8chr beforeCh);
    static bool AddStandardFunction(U8String function, int64_t index);
    static bool AddStandardAssets();
    TokenTypes ReadNextToken();
    TokenTypes GenerateTokens(TokenTypes type);
};

#endif