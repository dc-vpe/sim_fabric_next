//
// Created by krw10 on 6/14/2023.
//

#ifndef DSL_TOKEN_H
#define DSL_TOKEN_H

#include "../Includes/DslValue.h"
#include "../Includes/U8String.h"
#include "../Includes/List.h"
#include "TokenTypes.h"

/// <summary>
/// Defines the information needed to parse a single element in the rawSource code.
/// </summary>
class Token
{
public:
    /// \desc The Type of token, determines which fields are filled in.
    TokenTypes type;

    /// \desc If this is a value Type token, points to the value contained in the token.
    DslValue *value{};

    /// \desc if the token contains a cText string, for example, a variable _n its identifier is contained here.
    U8String *identifier{};

    /// \desc Indicates the scope access level for variables and functions.
    TokenModifiers modifier;

    /// \desc Indicates that the variable, parameter, or function is read only.
    bool readyOnly{};

    /// \desc Location at which this token was generated. This is used for error output
    ///in the parser and code generator.
    LocationInfo location;

    /// \desc Switch statements this is the index of the token that begins the default case. If
    ///       no default case exists this will be set to -1
    int64_t defaultIndex{};

    /// \desc Switch statements total cases.
    int64_t totalCases{};

    /// \desc Total parameters defined if this is a script function.
    int64_t definedParameters{};

    /// \desc list of local variables. Local variables can only be declared within a function.
    List<Token *>localVariables;

    /// \desc Create a new empty token.
    Token();

    /// \desc Creates a new token with the data from the passed in token.
    /// \param token Pointer to the token containing the data to copy.
    explicit Token(Token *token);

    /// \desc Adds a value token.
    /// \param dslValue containing the value and its Type to be created.
    explicit Token(DslValue *dslValue);

    /// \desc Creates a key word or operator token.
    explicit Token(TokenTypes tokenTypes);

    /// \desc Creates a new token with the specified Type and id.
    explicit Token(TokenTypes tokenTypes, U8String *id);

    /// \desc Gets the binding power or precedence of the token.
    /// \return Binding power of this token.
    [[nodiscard]] int64_t bp() const { return GET_BP(type); }

    /// \desc Checks if the token is an operator.
    [[nodiscard]] bool is_op() const { return IS_OPERATOR(type); }

    /// \desc Checks if the token is a unary (prefix) operator.
    [[nodiscard]] bool is_unary() const { return IS_UNARY_OPERATOR(type); }

    /// \desc Checks if the token is a binary (infix) operator.
    [[nodiscard]] bool is_binary() const { return IS_BINARY_OPERATOR(type); }

    /// \desc Gets the associativity of the token.
    /// \return True if the token is left associative, else false.
    [[nodiscard]] bool is_left_assoc() const { return IS_LEFT_ASSOC(type); }

    /// \desc Checks if the token contains a value.
    /// \return True of the token contains a value, else false.
    [[nodiscard]] bool is_value() const { return IS_VALUE_TYPE(type); }

    /// \desc Checks if the token is a semi-colon indicating the end of the statement.
    /// \return True if the token contains a semicolon operator, else false.
    [[nodiscard]] bool is_semicolon() const { return type == SEMICOLON; }

    /// \desc Checks if the token is a comma indicating the end of the expression.
    /// \return True if the token contains a comma operator, else false.
    [[nodiscard]] bool is_comma() const { return type == COMMA; }

    /// Frees the memory used by a token.
    ~Token();
};

#endif //DSL_TOKEN_H
