#include "../Includes/Lexer.h"
#include <cstdio>
#include <cctype>


//Prevents clang from complaining the source is too complex to perform a static analyze on. Warnings
//and errors are still generated this is informational only.
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

/// \desc Gets the character at the location.
/// \return The character or U8_NULL_CHR if location is outside the range of the code.
u8chr Lexer::GetCurrent()
{
    if ( locationInfo.location < 0 || locationInfo.location >= parseBuffer.Count() )
    {
        return U8_NULL_CHR;
    }
    return parseBuffer.get(locationInfo.location);
}

/// \desc Gets the character after the one at the current location.
/// \return The character or U8_NULL_CHR if location is outside the range of the code.
u8chr Lexer::GetNext()
{
    if ( locationInfo.location < 0 || (locationInfo.location+1 >= parseBuffer.Count()) )
    {
        return U8_NULL_CHR;
    }
    return parseBuffer.get(locationInfo.location+1);
}

/// \desc Checks if the character is a valid hex digit.
/// \param ch character to check.
/// \return True if the character is a hex digit, else false.
bool Lexer::IsHexDigit(u8chr ch)
{
    return isdigit((char)ch) || ((char)ch >= 'A' && (char)ch <= 'F') || ((char)ch >= 'a' && (char)ch <= 'f');
}

/// \desc Converts the UTF8 hex char into its decimal value.
/// \param ch character to convert.
/// \return The decimal value of the hex digit.
/// \remark Assumes the character is a valid hex digit.
int64_t Lexer::ConvertHexDigit(u8chr ch)
{
    int64_t value;

    if ( (char) ch <= '9' )
    {
        value = (char)ch - '0';
    }
    else
    {
        value = 10 + (int64_t)(TO_UPPER_HEX(ch)-'A');
    }

    return value;
}

/// \desc Checks if the character one of the characters used for an operator.
/// \param ch character to check.
/// \return True if the character belongs to the set of operator characters, else false.
bool Lexer::IsOperatorChar(u8chr ch)
{
    switch( ch )
    {
        case '(': case ')': case '{': case '}': case '+': case '-':
        case '!': case '*': case '/': case '%': case '<': case '>': case '=':
        case '&': case '|': case '?': case ',': case ';': case '[': case ']':
            return true;
        default:
            return false;
    }
}

/// \desc Checks if the token Type is a valid token to precede a unary operator.
/// \param type Token Type to check.
/// \return True if the token Type is valid to precede a unary operator, else false.
bool Lexer::IsValidUnaryPreviousToken(TokenTypes type)
{
    switch( type )
    {
        case OPEN_PAREN: case MULTIPLY: case DIVIDE: case MODULO: case ADDITION: case SUBTRACTION:
        case COMMA: case SEMICOLON: case CASE: case RETURN:
            return true;
        default:
            return false;
    }
}

/// \desc Checks if the character following a \ is a valid character escape character.
/// \param ch character to check.
/// \return True if the character is valid following a \ character in a string, else false.
bool Lexer::IsValidStringEscape(char ch)
{
    switch(ch)
    {
        case '\\': case '\'': case '\"': case 'n': case 'r': case 't': case 'b': case 'f': case '{': case '}':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        case 'X': case 'x':
            return true;
        default:
            return false;
    }
}

/// \desc Gets the Type of the token at the tok location. The
///       location is moved to the next parse location.
TokenTypes Lexer::GetOperatorToken(bool ignoreErrors)
{
    u8chr ch = GetCurrent();
    u8chr ch1 = PeekNextChar();

    locationInfo.Increment(ch);
    if ( locationInfo.location > parseBuffer.Count() )
    {
        return END_OF_SCRIPT;
    }

    LocationInfo saved = locationInfo;

    switch((char)ch)
    {
        case '(':
            if ( IS_IDENTIFIER(ch1))
            {
                if ( !GetIdentifier() ) //out of memory.
                {
                    locationInfo = saved;
                    return INVALID_TOKEN;
                }
                if ( tmpBuffer->IsEqual("int") )
                {
                    ch = GetCurrent();
                    if ( ch == ')' )
                    {
                        locationInfo.Increment(ch);
                        return CAST_TO_INT;
                    }
                }
                else if (tmpBuffer->IsEqual("double"))
                {
                    ch = GetCurrent();
                    if ( ch == ')' )
                    {
                        locationInfo.Increment(ch);
                        return CAST_TO_DBL;
                    }
                }
                else if (tmpBuffer->IsEqual("char"))
                {
                    ch = GetCurrent();
                    if ( ch == ')' )
                    {
                        locationInfo.Increment(ch);
                        return CAST_TO_CHR;
                    }
                }
                else if (tmpBuffer->IsEqual("string"))
                {
                    ch = GetCurrent();
                    if ( ch == ')' )
                    {
                        locationInfo.Increment(ch);
                        return CAST_TO_STR;
                    }
                }
                else if ( tmpBuffer->IsEqual("bool"))
                {
                    ch = GetCurrent();
                    if ( ch == ')' )
                    {
                        locationInfo.Increment(ch);
                        return CAST_TO_BOOL;
                    }
                }
            }
            locationInfo = saved;
            return OPEN_PAREN;
        case ')':
            return CLOSE_PAREN;
        case '{':
            return OPEN_BLOCK;
        case '}':
            return CLOSE_BLOCK;
        case '[':
            return OPEN_BRACE;
        case ']':
            return CLOSE_BRACE;
        case '+':
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return ADD_ASSIGNMENT;
            }
            if ( ch1 != '+' )
            {
                if ( IsNumber(ch) )
                {
                    return UNARY_POSITIVE;
                }
                return ADDITION;
            }
            locationInfo.Increment(ch1);
            if (PeekNextTokenType() == VARIABLE_VALUE )
            {
                return PREFIX_INC;
            }
            return POSTFIX_INC;
        case '-':
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return SUBTRACT_ASSIGNMENT;
            }
            if ( ch1 != '-' )
            {
                if ( IsNumber(ch) )
                {
                    return UNARY_NEGATIVE;
                }
                return SUBTRACTION;
            }
            locationInfo.Increment(ch1);
            if (PeekNextTokenType() == VARIABLE_VALUE )
            {
                return PREFIX_DEC;
            }
            return POSTFIX_DEC;
        case '!':
        {
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return NOT_EQUAL_TO;
            }
            TokenTypes type = PeekNextTokenType();
            if (type == VARIABLE_VALUE || type == FUNCTION_CALL || IsNumber(ch1))
            {
                return UNARY_NOT;
            }
            if (!ignoreErrors)
            {
                PrintIssue(2000, true, false,
                           "NOT operator ! is missing a value to negate");
            }
            return ERROR_TOKEN;
        }
        case '*':
            if (ch1 == '*')
            {
                locationInfo.Increment(ch1);
                return EXPONENT;
            }
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return MULTIPLY_ASSIGNMENT;
            }
            return MULTIPLY;
        case '/':
            if (ch1 == '=' )
            {
                locationInfo.Increment(ch1);
                return DIVIDE_ASSIGNMENT;
            }
            return DIVIDE;
        case '%':
            if (ch1 == '=' )
            {
                locationInfo.Increment(ch1);
                return MODULO_ASSIGNMENT;
            }
            return MODULO;
        case '<':
            if (ch1 == '<')
            {
                locationInfo.Increment(ch1);
                return BITWISE_SHIFT_LEFT;
            }
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return LESS_OR_EQUAL;
            }
            return LESS_THAN;
        case '>':
            if (ch1 == '>')
            {
                locationInfo.Increment(ch1);
                return BITWISE_SHIFT_RIGHT;
            }
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return GREATER_OR_EQUAL;
            }
            return GREATER_THAN;
        case '=':
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return EQUAL_TO;
            }
            return ASSIGNMENT;
        case '&':
            if (ch1 == '&')
            {
                locationInfo.Increment(ch1);
                return LOGICAL_AND;
            }
            return BITWISE_AND;
        case '^':
            return BITWISE_XOR;
        case '|':
            if (ch1 == '|' )
            {
                locationInfo.Increment(ch1);
                return LOGICAL_OR;
            }
            return BITWISE_OR;
        case ',':
            return COMMA;
        case ';':
            return SEMICOLON;
        default:
            break;
    }
    return INVALID_TOKEN;
}

/// \desc Skips any white space and returns the first character after the white space.
/// \return The tok character in the buffer.
u8chr Lexer::SkipWhiteSpace()
{
    u8chr ch = GetCurrent();

    while( isspace((char)ch) )
    {
        locationInfo.Increment(ch);
        if ( locationInfo.location >= parseBuffer.Count() )
        {
            return U8_NULL_CHR;
        }
        ch = GetCurrent();
    }

    return ch;
}

/// \desc Looks at the next character in the source parse buffer.
/// \return The next u8chr or U8_NULL_CHR if there is no next character.
u8chr Lexer::PeekNextChar()
{
    if ( locationInfo.location + 1 <= parseBuffer.Count())
    {
        return parseBuffer.get(locationInfo.location + 1);
    }
    return U8_NULL_CHR;
}

/// \desc Gets the next identifier in the input source. An identifier starts with a letter or underscore
/// and can contain and number of letters, numbers, or underscores after the first one. The identifier
/// is returned in the lexer tmpBuffer UTF8String.
/// \return True if successful, false if an error occurs because the tmp buffer could not be extended.
bool Lexer::GetIdentifier( )
{
    tmpBuffer->Clear();
    u8chr ch = GetCurrent();
    while (locationInfo.location < parseBuffer.Count() && IS_IDENTIFIER(ch))
    {
        if ( !tmpBuffer->push_back(ch))
        {
            fatal = true; //out of memory.
            return false;
        }
        locationInfo.Increment(ch);
        ch = GetCurrent();
    }
    return true;
}

/// \desc Gets the type of token for the key word in the tmp buffer.
/// \return The token Type of the keyword.
TokenTypes Lexer::GetKeyWordTokenType(bool ignoreErrors)
{
    for(int64_t ii=0; !keyWords[ii]->IsEqual(INVALID_TOKEN); ++ii)
    {
        if ( keyWords[ii]->IsEqual(tmpBuffer) )
        {
            return keyWords[ii]->Type();
        }
    }
    return INVALID_TOKEN;
}

/// \desc Checks if the code source at the tok location is the start of a single line comment.
/// \param ch Currently read character from the location in the parse buffer.
/// \return True if the start of a single line comment, else false.
bool Lexer::IsSingleLineComment( u8chr ch)
{
    u8chr ch1;

    if ( (char)ch != '/')
    {
        return false;
    }

    //if at end of _f it can't be a double //
    ch1 = PeekNextChar();
    if ( ch1 == U8_NULL_CHR )
    {
        return false;
    }

    return (char)ch1 == '/';
}

/// \desc Checks if the tok location in the source starts a multi-line comment.
/// \param ch Currently read character from the location in the parse buffer.
/// \return True if the location starts a multi-line comment, else false.
bool Lexer::IsMultiLineComment( u8chr ch)
{
    u8chr ch1;

    if ( (char)ch != '/')
    {
        return false;
    }

    ch1 = PeekNextChar();
    if ( ch1 == U8_NULL_CHR )
    {
        return false;
    }
    return (char)ch1 == '*';
}

/// \desc converts the tok location to a number.
/// \return True, if successful or false if a fatal error occurs.
bool Lexer::GetNumber(bool ignoreErrors)
{
    //convert string to number.
    double value = 0.0;
    double divisor = 1.0;
    bool period = false;
    bool warningShown = false;

    double sign = 1.0;

    u8chr ch = GetCurrent();
    if ( ch == '-' )
    {
        locationInfo.Increment(ch);
        sign = -1.0;
    }
    else if (ch == '+' )
    {
        locationInfo.Increment(ch);
    }

    tmpBuffer->Clear();
    while(locationInfo.location < parseBuffer.Count())
    {
        ch = GetCurrent();
        LocationInfo save = locationInfo;
        locationInfo.Increment(ch);
        if (!IS_DIGIT(ch) && ch != '.')
        {
            locationInfo = save;
            break;
        }
        if ( !tmpBuffer->push_back(ch))
        {
            fatal = true; //out of memory.
            return false;
        }
        if (ch == '.')
        {
            if (period)
            {
                if (!warningShown)
                {
                    if ( !ignoreErrors )
                    {
                        PrintIssue(2003, true, false,
                                   "A number can contain only a single period ignoring");
                    }
                    warningShown = true;
                }
            }
            period = true;
        }
        else
        {
            auto d = (double) ((char) ch - '0');
            if (d >= 0 && d <= 9)
            {
                if (period)
                {
                    divisor /= 10.0;
                }
                value *= 10.0;
                value += d;
            }
        }
    }

    if ( period )
    {
        tmpValue->type = DOUBLE_VALUE;
        tmpValue->dValue = value * divisor * sign;
    }
    else
    {
        tmpValue->type = INTEGER_VALUE;
        tmpValue->iValue = (int64_t)(value * sign);
    }

    return true;
}

/// \desc Gets the value of the escape number value at the tok pLocation.
/// \param factor set to 16 for hex or 10 for decimal conversion.
/// \return The calculated value of the escape code.
u8chr Lexer::GetEscapeValue(int64_t factor)
{
    u8chr value = 0;
    //the conversion expects pLocation to be set to the first digit.
    while (locationInfo.location < parseBuffer.Count())
    {
        u8chr ch = GetCurrent();
        if ( !IsHexDigit(ch))
        {
            break;
        }
        value *= factor;
        value += ConvertHexDigit(ch);
        locationInfo.Increment(ch);
    }

    return value;
}

/// \desc Processes a single string escape character
/// \param ch Character at previous location, will be updated with the new character.
/// \return True if successful, false if an error occurs and processing can't continue.
bool Lexer::ProcessEscapeCharacter(u8chr &ch, bool ignoreErrors)
{
    if (locationInfo.location >= parseBuffer.Count())
    {
        if ( !ignoreErrors )
        {
            PrintIssue(2006, true, false, "Expected a character after \\ escape character in string value");
        }
        return false;
    }
    ch = GetCurrent();
    if (!IsValidStringEscape((char) ch))
    {
        if ( !ignoreErrors )
        {
            PrintIssue(2009, false, false,
                       "The '%c' character is not a valid escape character", (char)ch);
        }
        return true;
    }
    switch(ch)
    {
        case 'X': case 'x':
            locationInfo.Increment(ch); //Skip the X || x hex format identifier.
            ch = GetEscapeValue(16);
            break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            ch = GetEscapeValue(10);
            break;
        case 'n':
            locationInfo.Increment(ch);
            ch = '\n';
            break;
        case 'r':
            locationInfo.Increment(ch);
            ch = '\r';
            break;
        case 't':
            locationInfo.Increment(ch);
            ch = '\t';
            break;
        case 'b':
            locationInfo.Increment(ch);
            ch = '\b';
            break;
        case 'f':
            locationInfo.Increment(ch);
            ch = '\f';
            break;
        default:
            locationInfo.Increment(ch);
            break;
    }

    return true;
}

/// \desc Gets a string by including the characters between $" and "$ directly.
bool Lexer::GetStringDirect(bool ignoreErrors)
{
    tmpValue->type = STRING_VALUE;
    tmpValue->sValue.Clear();

    //skip opening $.
    locationInfo.Increment('$');
    //skip opening double quote.
    locationInfo.Increment('\"');
    u8chr ch = '\"';

    while( ch != U8_NULL_CHR )
    {
        ch = GetCurrent();
        locationInfo.Increment(ch);
        if ( ch == U8_NULL_CHR )
        {
            PrintIssue(2012, true, false,"End of script instead of \"$ end of string definition");
            return false;
        }
        if ( ch != '\"' )
        {
            tmpValue->sValue.push_back(ch);
            continue;
        }
        ch = GetCurrent();
        locationInfo.Increment(ch);
        if ( ch == '$' )
        {
            return true;
        }
        tmpValue->sValue.push_back(ch);
    }

    return false;
}

/// \desc Gets a string value from the code source.
/// \return True if the string extraction is successful or false if an error occurs.
bool Lexer::GetString(bool ignoreErrors)
{
    int64_t quotes = 0;

    tmpValue->type = STRING_VALUE;
    tmpValue->sValue.Clear();

    while (locationInfo.location < parseBuffer.Count() && quotes < 2)
    {
        u8chr ch = GetCurrent();
        locationInfo.Increment(ch);
        if (ch != '\\')
        {
            if ( ch == '\"' )
            {
                quotes++;
                continue;
            }
            if (!tmpValue->sValue.push_back(ch))
            {
                fatal = true; //out of memory.
                return false;
            }
        }
        else //else escape code
        {
            if (!ProcessEscapeCharacter(ch, ignoreErrors))
            {
                return false;
            }
            if (!tmpValue->sValue.push_back(ch))
            {
                fatal = true;
                return false;
            }
        }
    }

    return true;
}

/// \desc Gets the next character token from the source code for a char '' or string ""
/// \return True if successful, false if an error occurs.
bool Lexer::GetCharacterValue(bool ignoreErrors)
{
    tmpValue->type = CHAR_VALUE;

    u8chr ch = GetCurrent();

    //Skip the open single quote.
    locationInfo.Increment(ch);
    ch = GetCurrent();
    if ( ch == '\'' )
    {
        locationInfo.Increment(ch);
        ch = U8_NULL_CHR;
    }
    else if ( ch == '\\')
    {
        if (!ProcessEscapeCharacter(ch, ignoreErrors))
        {
            return false;
        }
    }
    tmpValue->cValue = ch;
    locationInfo.Increment(ch);
    ch = GetCurrent();
    if ( ch != '\'')
    {
        PrintIssue(2015, true, false,
                   "Single character must be enclosed in single quotes");
        return false;
    }

    locationInfo.Increment(ch);

    return true;
}

bool Lexer::ValidateAndGetFullName(Token *token, bool ignoreErrors)
{
    switch( token->modifier )
    {
        case TMLocalScope:
            if ( !definingFunction )
            {
                if ( !ignoreErrors )
                {
                    PrintIssue(2018, true, false, "Local variables can only be created inside of a function definition");
                }
                return false;
            }
            if ( tmpBuffer->IndexOf('.') >= 0 )
            {
                if ( !ignoreErrors )
                {
                    PrintIssue(2021, true, false,
                               "Local variables can't contain a period as they only exist inside "
                               "the function in which they are declared.");
                }
                return false;
            }
            tmpBuffer->Clear();
            tmpBuffer->CopyFromCString("TMLocalScope.");
            tmpBuffer->push_back(&module);
            tmpBuffer->push_back('.');
            tmpBuffer->push_back(&currentFunction);
            tmpBuffer->push_back('.');
            tmpBuffer->push_back(token->identifier);
            break;
        case TMScriptScope:
            if ( tmpBuffer->IndexOf('.') >= 0 )
            {
                if ( !ignoreErrors )
                {
                    PrintIssue(2024, true, false,
                               "Script variables can't contain a period as they only exist inside "
                               "the script in which they are declared.");
                }
                return false;
            }
            tmpBuffer->Clear();
            tmpBuffer->CopyFromCString("TMScriptScope.");
            tmpBuffer->push_back(&module);
            tmpBuffer->push_back('.');
            tmpBuffer->push_back(token->identifier);
            break;
        case TMGlobalScope:
        {
            tmpBuffer->Clear();
            tmpBuffer->CopyFromCString("TMGlobalScope.");
            tmpBuffer->push_back(&module);
            tmpBuffer->push_back('.');

            int64_t end = token->identifier->IndexOf('.');
            if ( end >= 0 )
            {
                U8String check = U8String();
                for(int64_t ii=0; ii<end; ++ii)
                {
                    check.push_back(token->identifier->get(ii));
                }
                if ( !module.IsEqual(&check) )
                {
                    if ( !ignoreErrors )
                    {
                        PrintIssue(2027, true, false,
                                   "Global variables can only be defined in the module in which they are created.");
                    }
                    return false;
                }
            }
            tmpBuffer->push_back(token->identifier);
        }
            break;
    }

    token->identifier->CopyFrom(tmpBuffer);
    token->value->variableName.CopyFrom(tmpBuffer);

    return true;
}

/// \desc Checks the function definition syntax and calls define function to define the tokens
///       for the function if the syntax is valid.
/// \return True if successful, false if an error occurs.
bool Lexer::ProcessFunctionDefinition(Token *token)
{
    //Check if this function is a signal handler
    //OnError, OnMouse, OnKey, OnTick
    currentFunction.CopyFrom(token->identifier);
    LocationInfo start = locationInfo;
    bool rc = CheckFunctionDefinitionSyntax(token);
    locationInfo = start;
    if ( rc )
    {
        rc = DefineFunction(token);
    }
    else
    {
        SkipToEndOfBlock(start);
        varSpecified = false;
    }

    definingFunction = false;

    return rc;
}

/// \desc Replaces the var token for a variable with a VARIABLE_DEF token.
/// \return True if successful, else false if an error occurs.
bool Lexer::DefineVariable()
{
    auto *token = new Token(VARIABLE_DEF, tmpBuffer);
    token->modifier = modifier;
    token->readyOnly = constSpecified;
    token->value = new DslValue();
    token->value->variableScriptName.CopyFrom(tmpBuffer);

    //Check that the value in tmp buffer, which matches the script, is valid.
    //If it is update token identifier and value -> variable Name so that
    //they contain the full path. Note: This can't be done with get full name
    //as that method assumes the variable or function has already been defined.
    if ( !ValidateAndGetFullName(token, false) )
    {
        SkipToEndOfStatement();
        return false;
    }

    TokenTypes type = PeekNextTokenType();

    //If an open paren follows the name this is a function definition.
    if ( type == OPEN_PAREN )
    {
        return ProcessFunctionDefinition(token);
    }

    int64_t assignmentStart = -1;

    if (IS_ASSIGNMENT_TOKEN(type))
    {
        assignmentStart = tokens.Count();

        if ( !variables.Set(token->identifier, token) )
        {
            return false;
        }

        if ( !tokens.push_back(token) )
        {
            return false;
        }

        auto *t = new Token(token);
        t->type = VARIABLE_ADDRESS;
        t->value->type = VARIABLE_ADDRESS;
        t->value->opcode = PVA;
        if ( !tokens.push_back(t) )
        {
            return false;
        }

        if ( IsCollectionAssignment() )
        {
            //Skip the open curly brace to enable define-collection to be recursive.
            SkipNextTokenType();
            SkipNextTokenType();
            if( !DefineCollection(token) )
            {
                return false;
            }
        }
        else
        {
            if ( !DefineSingleVariableAssignment(type, token))
            {
                return false;
            }
        }

        type = PeekNextTokenType();
        if ( type == COMMA )
        {
            type = PeekNextTokenType();
            if ( type == VAR )
            {
                SkipNextTokenType();
            }
            //Makes var specification optional when multiple variables are being treated.
            varSpecified = true;
        }

        return true;
    }
    else
    {
        auto *variableInfoToken = new Token(token);
        if ( !variables.Set(variableInfoToken->identifier, variableInfoToken) )
        {
            return false;
        }
        if ( !tokens.push_back(token) )
        {
            return false;
        }
        type = GetNextTokenType(true);
        if ( type == SEMICOLON )
        {
            modifier = TMScriptScope;
            varSpecified = false;
            constSpecified = false;
        }
    }

    if (type != SEMICOLON )
    {
        varSpecified = false;
        PrintIssue(2030, true, false, "Missing semicolon at end of expression");
        SkipToEndOfStatement();
        return false;
    }

    return true;
}

#define PARAM_DONE      1
#define PARAM_CONTINUE  2
#define PARAM_ERROR     3

int Lexer::CheckFunctionNamedParameter()
{
    TokenTypes type = GetNextTokenType(true);
    if ( type == CLOSE_PAREN )
    {
        return PARAM_DONE;
    }
    if ( type == FUNCTION_PARAMETER )
    {
        type = GetNextTokenType(true);
        if ( type == COMMA )
        {
            if ( PeekNextTokenType() == CLOSE_PAREN )
            {
                PrintIssue(2033, true, false, "Expected a name for the parameter");
                return PARAM_ERROR;
            }
            return PARAM_CONTINUE;
        }
        if ( type == CLOSE_PAREN )
        {
            return PARAM_DONE;
        }

        PrintIssue(2036, true, false, "Expected a comma or close paren after the parameter name");
        return PARAM_ERROR;
    }

    PrintIssue(2039, true, false,
               "Expected a close paren or the parameters name after the functions open paren");
    return PARAM_ERROR;
}

/// \desc Checks that the function is defined correctly.
/// \param token The token containing the start token for a function definition.
/// \return True if the function is syntax is correct, or false if an error occurs.
bool Lexer::CheckFunctionDefinitionSyntax(Token *token)
{
    LocationInfo start = locationInfo;
    if ( definingFunction )
    {
        PrintIssue(2042, true, false, "Functions can't be declared inside other functions");
        return false;
    }

    TokenTypes type = GetNextTokenType(true);

    if ( type != OPEN_PAREN )
    {
        PrintIssue(2045, true, false,
                   "Functions require an open paren after the name of the function");
        return false;
    }

    definingFunctionsParameters = true;
    int result = PARAM_CONTINUE;
    while( result == PARAM_CONTINUE )
    {
        result = CheckFunctionNamedParameter();
        if ( result == PARAM_DONE )
        {
            break;
        }
        if ( result == PARAM_ERROR )
        {
            definingFunctionsParameters = false;
            locationInfo = start;
            return false;
        }
    }
    definingFunctionsParameters = false;

    if ( PeekNextTokenType() != OPEN_BLOCK )
    {
        PrintIssue(2048, true, false, "Missing open curly brace after close parenthesis in function definition");
        locationInfo = start;
        return false;
    }

    if (!SkipToEndOfBlock(start))
    {
        PrintIssue(2051, true, false, "Missing close curly brace in function definition");
        locationInfo = start;
        return false;
    }

    locationInfo = start;
    return true;
}

/// \desc Processes a function definition.
/// \param token Token being processed.
/// \return True if successful, else false. The caller is responsible for setting the
///         lex position to after the function block so the lexing can continue.
bool Lexer::DefineFunction(Token *token)
{
    LocationInfo start = locationInfo;
    definingFunction = true;
    varSpecified = false;

    auto *funBegin = new Token(token);
    funBegin->type = FUNCTION_DEF_BEGIN;
    tokens.push_back(funBegin);

    if ( !functions.Set(funBegin->identifier, funBegin) )
    {
        SkipToEndOfBlock(start);
        definingFunction = false;
        return false;
    }

    if ( !DefineFunctionParameters())
    {
        SkipToEndOfBlock(start);
        definingFunction = false;
        return false;
    }

    //define functions code block
    //Functions can use = { } or simply { }
    if ( PeekNextTokenType() == ASSIGNMENT )
    {
        SkipNextTokenType();
    }

    if ( PeekNextTokenType() != OPEN_BLOCK )
    {
        PrintIssue(2054, true, false, "Missing open curly brace in function definition");
        SkipToEndOfBlock(start);
        definingFunction = false;
        return false;
    }

    LocationInfo next = locationInfo;
    if (!SkipToEndOfBlock(start))
    {
        PrintIssue(2057, true, false, "Missing close curly brace in function definition");
        SkipToEndOfBlock(start);
        definingFunction = false;
        return false;
    }

    LocationInfo end = locationInfo;
    locationInfo = next;
    //Skip open curly brace
    SkipNextTokenType();
    start = locationInfo;
    if ( !DefineStatements(end, false) )
    {
        SkipToEndOfBlock(start);
        definingFunction = false;
        return false;
    }

    auto *funEnd = new Token(token);
    funEnd->type = FUNCTION_DEF_END;
    tokens.push_back(funEnd);

    if ( PeekNextTokenType() == CLOSE_BLOCK )
    {
        //Skip close block
        SkipNextTokenType();
    }
    definingFunction = false;

    return true;
}

/// \desc Processes the parameters for a function definition. A function
///       does not really need parameters since all functions are defined
///       as variable arguments. But its easier to make the function with
///       named parameters instead of $1, $2 and so on.
/// \return True if successful, false if an error occurs.
bool Lexer::DefineFunctionParameters()
{
    TokenTypes type = GetNextTokenType(true);
    definingFunctionsParameters = true;
    stackVariables = 0;

    while( type != CLOSE_PAREN )
    {
        type = GetNextTokenType(false);
        if ( type == ERROR_TOKEN )
        {
            definingFunctionsParameters = false;
            return false;
        }
        else if ( type == FUNCTION_PARAMETER )
        {
            auto *dslValue = new DslValue();
            dslValue->type = FUNCTION_PARAMETER;
            auto *param = new Token(dslValue);
            param->modifier = TMLocalScope;
            param->identifier = new U8String(tmpBuffer);
            GetFullName(param->identifier, TMLocalScope);
            param->value->variableName.CopyFrom(param->identifier);
            param->value->operand = stackVariables++;
            tokens.push_back(param);
            variables.Set(param->identifier, param);
        }
        else if ( type != COMMA && type != CLOSE_PAREN )
        {
            PrintIssue(2060, true, false, "Invalid identifier specified for function parameter");
            definingFunctionsParameters = false;
            return false;
        }
    }

    definingFunctionsParameters = false;
    return true;
}

/// \desc Processes the operator specified in type.
/// \param values Reference to the value stack containing the values for the operation.
/// \param type TokenType specifying the operation to be performed.
/// \param ignoreErrors If true errors are generated, else they are ignored.
/// \remark True if the operation is processed successfully, else false if an error occurs.
/// \remark The top of the value stack is adjusted based on the operation performed.
bool Lexer::ProcessOperation(Stack<DslValue *> &values, TokenTypes type, bool ignoreErrors)
{
    switch(type)
    {
        case UNARY_POSITIVE: break;
        case UNARY_NEGATIVE: values[values.top()-1]->NEG();  break;
        case UNARY_NOT: values[values.top()-1]->NOT();  break;
        case CAST_TO_INT: values[values.top()-1]->Convert(INTEGER_VALUE);  break;
        case CAST_TO_DBL: values[values.top()-1]->Convert(DOUBLE_VALUE);  break;
        case CAST_TO_CHR: values[values.top()-1]->Convert(CHAR_VALUE);  break;
        case CAST_TO_STR: values[values.top()-1]->Convert(STRING_VALUE);  break;
        case CAST_TO_BOOL: values[values.top()-1]->Convert(BOOL_VALUE);  break;
        case EXPONENT: values[values.top()-2]->EXP(values[values.top()-1]);  values.reduce(); break;
        case MULTIPLY_ASSIGNMENT: values[values.top()-2]->Convert(values[values.top()-1]->type);
        case MULTIPLY: values[values.top()-2]->MUL(values[values.top()-1]); values.reduce(); break;
        case DIVIDE_ASSIGNMENT: values[values.top()-2]->Convert(values[values.top()-1]->type);
        case DIVIDE:
            if ( values[values.top()-1]->IsZero() )
            {
                if ( !ignoreErrors )
                {
                    PrintIssue(2063, true, false, "Divide by zero is undefined");
                    return false;
                }
                else
                {
                    values.reduce();
                    break;
                }
            }
            values[values.top()-2]->DIV(values[values.top()-1]); values.reduce(); break;
        case MODULO_ASSIGNMENT: values[values.top()-2]->Convert(values[values.top()-1]->type);
        case MODULO:
            if ( values[values.top()-1]->IsZero() )
            {
                if ( !ignoreErrors )
                {
                    PrintIssue(2066, true, false, "Modulo by zero is undefined");
                    return false;
                }
                else
                {
                    values.reduce();
                    break;
                }
            }
            values[values.top()-2]->MOD(values[values.top()-1]); values.reduce();
            break;
        case ADD_ASSIGNMENT: values[values.top()-2]->Convert(values[values.top()-1]->type);
        case ADDITION: values[values.top()-2]->ADD(values[values.top()-1]); values.reduce(); break;
        case SUBTRACT_ASSIGNMENT: values[values.top()-2]->Convert(values[values.top()-1]->type);
        case SUBTRACTION: values[values.top()-2]->SUB(values[values.top()-1]); values.reduce(); break;
        case BITWISE_SHIFT_LEFT:
            values[values.top() - 2]->SVL(values[values.top() - 1]); values.reduce(); break;
        case BITWISE_SHIFT_RIGHT:
            values[values.top() - 2]->SVR(values[values.top() - 1]); values.reduce(); break;
        case LESS_THAN: values[values.top()-2]->TLS(values[values.top()-1]); values.reduce(); break;
        case LESS_OR_EQUAL: values[values.top()-2]->TLE(values[values.top()-1]); values.reduce(); break;
        case GREATER_THAN: values[values.top()-2]->TGR(values[values.top()-1]); values.reduce(); break;
        case GREATER_OR_EQUAL: values[values.top()-2]->TGE(values[values.top()-1]); values.reduce(); break;
        case EQUAL_TO: values[values.top()-2]->TEQ(values[values.top()-1]); values.reduce(); break;
        case NOT_EQUAL_TO: values[values.top()-2]->TNE(values[values.top()-1]); values.reduce(); break;
        case BITWISE_AND: values[values.top()-2]->BND(values[values.top()-1]); values.reduce(); break;
        case BITWISE_XOR: values[values.top()-2]->XOR(values[values.top()-1]); values.reduce(); break;
        case BITWISE_OR: values[values.top()-2]->BOR(values[values.top()-1]); values.reduce(); break;
        case LOGICAL_AND: values[values.top()-2]->AND(values[values.top()-1]); values.reduce(); break;
        case LOGICAL_OR: values[values.top()-2]->LOR(values[values.top()-1]); values.reduce(); break;
        case ASSIGNMENT: values[values.top()-2]->LiteCopy(values[values.top()-1]); values.reduce(); break;
        default:
            break;
    }

    return true;
}

/// \desc Adds the dslValue to the end of the values stack. Used when attempting to evaluate
///       an expression at lex time.
bool Lexer::PushTmpValue(Stack<DslValue *> &values, DslValue *dslValue, int64_t &top)
{
    tmpValues[top]->LiteCopy(dslValue);
    if ( top >= tmpValues.Size() )
    {
        int64_t start = tmpValues.Count();
        int64_t end = start + ALLOC_BLOCK_SIZE;
        for(int64_t ii=start; ii<end; ++ii)
        {
            tmpValues[ii] = new DslValue();
            if ( tmpValues[ii] == nullptr )
            {
                PrintIssue(2069, true, false,
                           "Out of memory extending temporary work values buffer");
                return false;
            }
        }
    }
    values.push_back(tmpValues[top++]);
    return true;
}

/// \desc Scans a variable assignment expression looking for the end of the expression.
/// \param ignoreErrors If true then normal error processing occurs, else error generation except
///                      for fatal errors are ignored.
/// \return Location Info the expression ends at.
bool Lexer::GetExpressionEnd(bool ignoreErrors, LocationInfo &end)
{
    LocationInfo start;
    start = locationInfo;
    TokenTypes type = INVALID_TOKEN;
    while( type != END_OF_SCRIPT )
    {
        type = PeekNextTokenType();
        if ( type == COMMA || type == CLOSE_BLOCK || type == SEMICOLON )
        {
            end = locationInfo;
            locationInfo = start;
            return true;
        }
        SkipNextTokenType();
    }

    locationInfo = start;
    return false;
}

/// \desc Checks if the token type is one that leads to an expression that can be
///       calculated before being parsed.
/// \return True if the token type can be used in a static expression else false.
bool Lexer::IsStaticType(TokenTypes type)
{
    switch( type )
    {
        case ERROR_TOKEN:
        case VARIABLE_VALUE:
        default:
        case END_OF_SCRIPT:
            return false;
        case COMMA: case CLOSE_BRACE: case MULTI_LINE_COMMENT: case SINGLE_LINE_COMMENT: case SEMICOLON:
        case INTEGER_VALUE: case DOUBLE_VALUE: case CHAR_VALUE: case BOOL_VALUE: case STRING_VALUE:
        case FALSE: case TRUE:
        case UNARY_NOT: case CAST_TO_INT: case CAST_TO_DBL: case CAST_TO_CHR: case CAST_TO_STR:
        case CAST_TO_BOOL: case UNARY_POSITIVE: case UNARY_NEGATIVE:
        case OPEN_BLOCK: case OPEN_PAREN: case CLOSE_BLOCK:
        case CLOSE_PAREN:
        case EXPONENT: case MULTIPLY:
        case DIVIDE: case MODULO: case ADDITION: case SUBTRACTION: case BITWISE_SHIFT_LEFT:
        case BITWISE_SHIFT_RIGHT: case LESS_THAN: case LESS_OR_EQUAL: case GREATER_THAN:
        case GREATER_OR_EQUAL: case EQUAL_TO: case NOT_EQUAL_TO: case BITWISE_AND: case BITWISE_XOR:
        case BITWISE_OR: case LOGICAL_AND: case LOGICAL_OR:
            return true;
    }
}

/// \desc Checks if the assignment expression an be pre-calculated.
/// \return True if it can else false.
bool Lexer::IsStaticExpression(LocationInfo end)
{
    LocationInfo start = locationInfo;
    TokenTypes prevToken = INVALID_TOKEN;
    TokenTypes type = INVALID_TOKEN;
    while( locationInfo.location < end.location)
    {
        prevToken = type;
        type = GetNextTokenType(true);
        if ( !IsStaticType(type) )
        {
            locationInfo = start;
            return false;
        }
    }

    locationInfo = start;
    return true;
}


/// \desc Scans an expression within a collection definition time and either assigns the result
///       to the token or generates an error if the expression can't be evaluated at lex time.
/// \param dslValue Pointer to the dsl value to be updated with the result of the calculation.
/// \param end Location at which the expression evaluation should stop.
/// \param ignoreErrors If true then normal error processing occurs, else error generation except
///                      for fatal errors are ignored.
/// \param initializeVariable If true a collection or variable is being initialized with an expression
///                           else false.
/// \remark Collections can't be initialized with dynamic values as their initialization must
///         contain only values or references to other collections. This is slightly different
///         than variable definitions as variable definitions are processed at runtime while
///         collections are associated arrays of static values or references to variables and
///         other collections.
/// \remark This method expects the lex position to be at the start of the expression and that
///         type contains the first token for the expression and that type is valid for an
///         expression that can be calculated at lex time.
bool Lexer::ProcessStaticExpression(DslValue *dslValue,
        LocationInfo end,
        bool ignoreErrors,
        bool initializeVariable)
{
    Stack<TokenTypes> ops;
    Stack<DslValue *> values;

    //Last work value in tmpValues list.
    int64_t top = 0;

    TokenTypes prevToken = INVALID_TOKEN;
    TokenTypes type = INVALID_TOKEN;
    if ( locationInfo.location == end.location )
    {
        PushTmpValue(values, tmpValue, top);
    }
    while( locationInfo.location < end.location)
    {
        prevToken = type;
        type = GetNextTokenType(ignoreErrors);
        if ( type == UNARY_NEGATIVE || type == UNARY_POSITIVE )
        {
            if (!IsValidUnaryPreviousToken(prevToken))
            {
                if (type == UNARY_POSITIVE )
                {
                    type = ADDITION;
                }
                else
                {
                    type = SUBTRACTION;
                }
            }
        }
        if ( type == ERROR_TOKEN )
        {
            definingAndAssigningVariable = false;
            return false;
        }
        switch( type )
        {
            case END_OF_SCRIPT:
                if ( ignoreErrors )
                {
                    PrintIssue(2072, true, false, "Missing close curly brace in collection definition");
                }
                return false;
            case COMMA: case CLOSE_BRACE: case MULTI_LINE_COMMENT: case SINGLE_LINE_COMMENT: case SEMICOLON:
                break;
            case INTEGER_VALUE: case DOUBLE_VALUE: case CHAR_VALUE: case BOOL_VALUE: case STRING_VALUE:
            case FALSE: case TRUE:
                PushTmpValue(values, tmpValue, top);
                break;
            case UNARY_NOT: case CAST_TO_INT: case CAST_TO_DBL: case CAST_TO_CHR: case CAST_TO_STR:
            case CAST_TO_BOOL: case UNARY_POSITIVE: case UNARY_NEGATIVE:
                while (ops.top() != 0 && ops.peek(1) != OPEN_PAREN && GET_BP(type) < GET_BP(ops.peek(1)))
                {
                    if (!ProcessOperation(values, type, ignoreErrors) )
                    {
                        definingAndAssigningVariable = false;
                        return false;
                    }
                }
                break;
            case OPEN_BLOCK: case OPEN_PAREN:
                ops.push_back(type);
                break;
            case CLOSE_BLOCK:
                while (ops.top() != 0 && ops.peek(1) != OPEN_BLOCK)
                {
                    if ( !ProcessOperation(values, ops.pop_back(), ignoreErrors) )
                    {
                        definingAndAssigningVariable = false;
                        return false;
                    }
                }
                if (ops.top() != 0 && ops.peek(1) == OPEN_BLOCK)
                {
                    ops.pop_back();
                }
                break;
            case CLOSE_PAREN:
                while (ops.top() != 0 && ops.peek(1) != OPEN_PAREN)
                {
                    if ( !ProcessOperation(values, ops.pop_back(), ignoreErrors) )
                    {
                        definingAndAssigningVariable = false;
                        return false;
                    }
                }
                if (ops.top() != 0 && ops.peek(1) == OPEN_PAREN)
                {
                    ops.pop_back();
                }
                break;
            case EXPONENT: case MULTIPLY:
            case DIVIDE: case MODULO: case ADDITION: case SUBTRACTION: case BITWISE_SHIFT_LEFT:
            case BITWISE_SHIFT_RIGHT: case LESS_THAN: case LESS_OR_EQUAL: case GREATER_THAN:
            case GREATER_OR_EQUAL: case EQUAL_TO: case NOT_EQUAL_TO: case BITWISE_AND: case BITWISE_XOR:
            case BITWISE_OR: case LOGICAL_AND: case LOGICAL_OR:
                while (ops.top() != 0 && ops.peek(1) != OPEN_PAREN && (ops.peek(1) < GET_BP(type) ||
                                                                       (type == GET_BP(ops.peek(1)) && IS_LEFT_ASSOC(type))))
                {
                    if ( !ProcessOperation(values, ops.pop_back(), ignoreErrors) )
                    {
                        definingAndAssigningVariable = false;
                        return false;
                    }
                }
                ops.push_back(type);
                break;
            case VARIABLE_VALUE:
                if (!initializeVariable )
                {
                    auto *token = new Token(VARIABLE_VALUE, tmpBuffer);
                    GetFullName(token->identifier, modifier);
                    auto *variable = variables.Get(token->identifier);
                    PushTmpValue(values, variable->value, top);
                    break;
                }
            default:
                if ( !ignoreErrors )
                {
                    PrintIssue(2075, true, false,
                               "Expressions used to initialize a variable "
                               "can only contain values, operators, and collections");
                    definingAndAssigningVariable = false;
                }
                break;
        }
    }
    while ( ops.top() != 0 )
    {
        if ( !ProcessOperation(values, ops.pop_back(), ignoreErrors) )
        {
            definingAndAssigningVariable = false;
            return false;
        }
    }

    dslValue->LiteCopy(new DslValue(values.pop_back()));

    return true;
}

/// \desc Process the script while checking the expression being used to
///       initialize the variable or collection element.
/// \param token Pointer to the variable or collection being initialized.
/// \param dslValue Pointer to a dslValue to be updated with the value
///                 to be stored in the variable or collection element.
/// \param isStaticExpression True if this is a static expression, else false.
/// \return True if successful, false if an error occurs.
/// \remark If the expression can't be evaluated at lex time a default
///         value is set in the dslValue. This works because the code
///         will replace this value when it is run.
bool Lexer::ProcessSingleAssignmentExpression(Token *token, DslValue *dslValue, bool &isStaticExpression, int64_t index)
{
    LocationInfo end;
    if ( !GetExpressionEnd(false, end) )
    {
        definingAndAssigningVariable = false;
        return false;
    }
    if (!IsStaticExpression(end))
    {
        if ( !DefineStatements(end, true))
        {
            definingAndAssigningVariable = false;
            return false;
        }
        if ( isCollectionElement )
        {
            auto *t = new Token(token);
            t->type = COLLECTION_ADDRESS;
            t->value->type = COLLECTION_ADDRESS;
            t->value->iValue = index;
            t->value->opcode = DCS;
            t->value->variableName.CopyFrom(&token->value->variableName);
            t->value->variableScriptName.CopyFrom(&token->value->variableScriptName);
            tokens.push_back(t);

            dslValue->SAV(t->value);
            dslValue->opcode = DCS;
        }
        else
        {
            dslValue->type = INTEGER_VALUE;
            dslValue->iValue = 0;
        }
        return true;
    }
    isStaticExpression = true;
    return ProcessStaticExpression(dslValue, end, false, true);
}

/// \desc Initializes a single variable with the following expression when the variable is
///       being defined.
/// \param type Type of assignment operation that triggered the initialization.
/// \param token Token containing the variable to be initialized.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineSingleVariableAssignment(TokenTypes type, Token *token)
{
    DslValue dslValue = {};

    definingAndAssigningVariable = true;
    bool isStaticExpression = false;

    if ( !ProcessSingleAssignmentExpression(token, &dslValue, isStaticExpression, -1) )
    {
        definingAndAssigningVariable = false;
        return false;
    }
    definingAndAssigningVariable = true;
    if ( isStaticExpression )
    {
        switch( type )
        {
            default:
            case ASSIGNMENT:
                token->value->LiteCopy(&dslValue);
                break;
            case MULTIPLY_ASSIGNMENT:
                token->value->MUL(&dslValue);
                break;
            case DIVIDE_ASSIGNMENT:
                token->value->DIV(&dslValue);
                if ( fatal )
                {
                    return false;
                }
                break;
            case MODULO_ASSIGNMENT:
                token->value->MOD(&dslValue);
                if ( fatal )
                {
                    return false;
                }
                break;
            case ADD_ASSIGNMENT:
                token->value->ADD(&dslValue);
                break;
            case SUBTRACT_ASSIGNMENT:
                token->value->SUB(&dslValue);
                break;
        }
        //If static expression then the variable address and assignment are not required.
        //as these operations have already happened.
        tokens.Resize(tokens.Count()-2);
    }

    return true;
}

/// \desc Determines if the assignment during variable collection creation is a variable or a collection.
/// \remark Needs to be checked as the variable type is not known when it is being created when the
///         assignment token is encountered.
bool Lexer::IsCollectionAssignment()
{
    LocationInfo start = locationInfo;

    SkipNextTokenType(); //skip assignment.
    TokenTypes type = GetNextTokenType(false);
    if ( type == ERROR_TOKEN )
    {
        return false;
    }
    locationInfo = start;
    return type == OPEN_BLOCK;
}

/// \desc Adds an empty collection element to a collection.
/// \param token Token containing the collection being built.
/// \param key Pointer to the key to be returned.
/// \param index Reference to the current index of the collection element.
///              This is used when a key needs to be generated.
void Lexer::AddEmptyCollectionElement(Token *token, U8String *key, int64_t &index)
{
    key->Clear();
    key->push_back(&token->value->variableScriptName);
    key->push_back('.');
    key->Append(index++);
    token->value->indexes.Set(new U8String(key), new DslValue());
}

/// \desc Creates a key for an element that does not have a specified key.
/// \param token Token containing the collection being built.
/// \param key Pointer to the key to be returned.
/// \param index Reference to the current index of the collection element.
///              This is used when a key needs to be generated.
void Lexer::GenerateKey(Token *token, U8String *key, int64_t &index)
{
    key->Clear();
    key->push_back(&token->value->variableScriptName);
    key->push_back('.');
    key->Append(index);
    index++;
}

/// \desc Gets or generates a key value for a collection element.
/// \param token Token containing the collection being built.
/// \param key Pointer to the key to be returned.
/// \param index Reference to the current index of the collection element.
///              This is used when a key needs to be generated.
TokenTypes Lexer::GetKey(Token *token, U8String *key, int64_t &index)
{
    TokenTypes type = PeekNextTokenType(0);
    switch (type)
    {
        case END_OF_SCRIPT:
            PrintIssue(2078, true, false,
                       "End of script reached before key definition was complete.");
            return ERROR_TOKEN;
        case COLON:
            PrintIssue(2081, true, false, "Collection element keys cannot be empty.");
            return ERROR_TOKEN;
        case DOUBLE_VALUE:
        case STRING_VALUE:
            if (PeekNextTokenType(2, true) == COLON)
            {
                if (tmpValue->sValue.Count() == 0)
                {
                    PrintIssue(2084, true, false, "Collection element keys cannot be empty.");
                    return ERROR_TOKEN;
                }
                key->Clear();
                key->push_back(&tmpValue->sValue);
                SkipNextTokenType();    //skip integer
                SkipNextTokenType(true);    //skip :
                return PeekNextTokenType(0);
            }
            else
            {
                //If there is no colon the next character must be either an open block or a comma.
                type = PeekNextTokenType(2);
                if ( type != COMMA && type != CLOSE_BLOCK )
                {
                    PrintIssue(2087, true, false,
                               "Invalid key format expected either a comma or a close curly brace");
                    return ERROR_TOKEN;
                }
            }
            GenerateKey(token, key, index);
            break;
        case OPEN_BLOCK:
            GenerateKey(token, key, index);
            break;
        case CLOSE_BLOCK:
        case COMMA:
            break;
        case INTEGER_VALUE:
            if (PeekNextTokenType(2, true) == COLON)
            {
                U8String tmp = {};
                if ( tmpValue->iValue < token->value->indexes.Count() )
                {
                    key->CopyFrom(token->value->indexes.keys[tmpValue->iValue]);
                }
                else
                {
                    for(int64_t ii=token->value->indexes.Count(); ii<tmpValue->iValue; ++ii)
                    {
                        AddEmptyCollectionElement(token, key, index);
                    }
                    key->Clear();
                    key->push_back(&token->value->variableScriptName);
                    key->push_back('.');
                    key->Append(tmpValue->iValue);
                }
                SkipNextTokenType();    //skip integer
                SkipNextTokenType(true);    //skip :
                return PeekNextTokenType(0);
            }
            GenerateKey(token, key, index);
            break;
        case VARIABLE_VALUE:
        case COLLECTION:
            if (PeekNextTokenType(2, true) == COLON)
            {
                PrintIssue(2090, true, false,
                           "Variables and collections can't be used as keys");
                return ERROR_TOKEN;
            }
            else
            {
                GenerateKey(token, key, index);
            }
            break;
        case FUNCTION_CALL:
            GenerateKey(token, key, index);
            break;
        default:
            PrintIssue(2093, true, false,
                       "%s is not a valid value for a collection element key",
                       tokenNames[(int64_t)type & 0xFF]);
            return ERROR_TOKEN;
    }

    return type;
}

/// \desc Defines a collection from the in line script data.
/// \param token Pointer to the token containing the collection variable.
/// \return True if successful or false if an error occurs.
/// \remark Assumes that the parse position is after the open block that starts a new collection.
bool Lexer::DefineCollection(Token *token)
{
    TokenTypes type;
    TokenTypes next;
    U8String key = {};

    int64_t index = 0;
    int64_t startBlocks = 1;

    int64_t element = 0;

    token->value->type = COLLECTION;
    token->modifier = modifier;

    while( startBlocks > 0 )
    {
        type = GetKey(token, &key, index);
        if ( type == ERROR_TOKEN )
        {
            return false;
        }
        if ( type == COMMA )
        {
            AddEmptyCollectionElement(token, &key, index);
            SkipNextTokenType();
            continue;
        }
        if ( type == CLOSE_BLOCK )
        {
            SkipNextTokenType();
            startBlocks--;
            continue;
        }
        if ( type == OPEN_BLOCK )
        {
            //Create a new variable for the collection.
            U8String tmp;
            tmpBuffer->CopyFrom(&key);
            GetFullName(&tmp, token->modifier);
            auto *inner = new Token(VARIABLE_DEF, tmpBuffer);
            inner->modifier = token->modifier;
            inner->readyOnly = token->readyOnly;
            inner->value = new DslValue();
            inner->value->variableScriptName.CopyFrom(tmpBuffer);
            inner->value->variableName.CopyFrom(&tmp);
            inner->identifier->CopyFrom(&tmp);

            if ( !variables.Set(inner->identifier, inner) )
            {
                return false;
            }

            if ( !tokens.push_back(inner) )
            {
                return false;
            }

            SkipNextTokenType();
            if ( !DefineCollection(inner))
            {
                return false;
            }
            token->value->indexes.Set(new U8String(&key), new DslValue(inner->value));
            //Skip comma if present as index has already been accounted for.
            if (PeekNextTokenType() == COMMA )
            {
                SkipNextTokenType();
            }
            continue;
        }

        DslValue dslValue = {};
        bool isStaticExpression = false;
        isCollectionElement = true;
        int64_t keyIndex = token->value->indexes.Count();
        for(int ii=0; ii<token->value->indexes.keys.Count(); ++ii)
        {
            if ( key.IsEqual(token->value->indexes.keys[ii]) )
            {
                keyIndex = ii;
                break;
            }
        }

        if ( !ProcessSingleAssignmentExpression(token, &dslValue, isStaticExpression, keyIndex) )
        {
            isCollectionElement = false;
            return false;
        }
        token->value->indexes.Set(new U8String(&key), new DslValue(&dslValue));
        isCollectionElement = false;
        if (PeekNextTokenType() == COMMA )
        {
            SkipNextTokenType();
        }
    }

    return true;
}

/// \desc Checks that the syntax of the while statement is correct and emits
///       a targeted error if the syntax is incorrect.
bool Lexer::CheckWhileSyntax()
{
    LocationInfo save =  locationInfo;

    TokenTypes type = GetNextTokenType(true);
    if ( type != OPEN_PAREN )
    {
        PrintIssue(2096, true, false, "a ( must follow the while statement");
        locationInfo = save;
        SkipToEndOfBlock(save);
        return false;
    }

    TokenTypes p;
    while( type != END_OF_SCRIPT && type != OPEN_BLOCK )
    {
        p = type;
        type = GetNextTokenType(false);
    }

    if ( type == END_OF_SCRIPT || (prev != CLOSE_PAREN) )
    {
        PrintIssue(2099, true, false,
                   "Missing close paren before start of the statement block");
        SkipToEndOfBlock(save);
        return false;
    }

    if ( type != OPEN_BLOCK )
    {
        PrintIssue(2102, true, false, "Missing open curly brace after while conditional");
        SkipToEndOfBlock(save);
        return false;
    }

    if (!SkipToEndOfBlock(save) )
    {
        PrintIssue(2105, true, false, "Missing close curly brace after while conditional");
        return false;
    }

    locationInfo = save;
    return true;
}

/// \desc Translates the text for the while code block into the the tokens used by the parser.
bool Lexer::DefineWhile()
{
    LocationInfo save = locationInfo;

    LocationInfo whileCondition = locationInfo;

    TokenTypes type = INVALID_TOKEN;

    while( PeekNextTokenType() != OPEN_BLOCK )
    {
        type = GetNextTokenType(false);
    }

    if (!DefineStatementBlock(save, WHILE_BLOCK_BEGIN, WHILE_BLOCK_END))
    {
        SkipToEndOfBlock(save);
        return false;
    }

    LocationInfo whileEnd = locationInfo;

    locationInfo = whileCondition;
    tokens.push_back(new Token(WHILE_COND_BEGIN));

    //Skip open paren as define condition assumes it is present.
    SkipNextTokenType();

    //Generate the conditional statements for the IF statement
    if ( !DefineConditionalExpression())
    {
        PrintIssue(2108, true, false,
                   "The while statement's condition is missing a close parenthesis");
        SkipToEndOfBlock(save);
        return false;
    }

    tokens.push_back(new Token(WHILE_COND_END));

    locationInfo = whileEnd;

    return true;
}

/// \desc Checks that the if statement is formatted correctly and displays errors
///       for what is wrong if incorrectly formatted.
bool Lexer::DefineIf()
{
    LocationInfo start = locationInfo;

    TokenTypes type = GetNextTokenType(false);
    if ( type != OPEN_PAREN )
    {
        SkipToEndOfBlock(start);
        PrintIssue(2111, true, false, "An ( must follow the if statement");
        return false;
    }
    tokens.push_back(new Token(IF_COND_BEGIN));

    //Generate the conditional statements for the IF statement
    if ( !DefineConditionalExpression())
    {
        PrintIssue(2114, true, false,
                   "The if statement conditional expression is missing a close paren");
        SkipToEndOfBlock(start);
        return false;
    }

    tokens.push_back(new Token(IF_COND_END));

    if ( PeekNextTokenType() != OPEN_BLOCK )
    {
        SkipToEndOfBlock(start);
        PrintIssue(2117, true, false, "Missing open curly brace after if loop condition");
        return false;
    }

    if (!DefineStatementBlock(start, IF_BLOCK_BEGIN, IF_BLOCK_END))
    {
        SkipToEndOfBlock(start);
        return false;
    }

    //Check for else
    if ( PeekNextTokenType() == ELSE )
    {
        //Skip the else
        SkipNextTokenType();

        if ( PeekNextTokenType() == ELSE )
        {
            SkipToEndOfBlock(start);
            PrintIssue(2120, true, false, "else without a matching if");
            return false;
        }

        //If this is a chained ELSE IF
        //Needs to be handled as an else without open close braces
        if ( PeekNextTokenType() == IF )
        {
            //This needs parsed as else { if () { } }
            tokens.push_back(new Token(ELSE_BLOCK_BEGIN));
            SkipNextTokenType();
            DefineIf();
            tokens.push_back(new Token(ELSE_BLOCK_END));
            return true;
        }

        if ( PeekNextTokenType() != OPEN_BLOCK )
        {
            SkipToEndOfBlock(start);
            PrintIssue(2123, true, false, "Missing open curly brace after else condition");
            return false;
        }

        //define the else block
        start = locationInfo;
        if (!DefineStatementBlock(start, ELSE_BLOCK_BEGIN, ELSE_BLOCK_END))
        {
            SkipToEndOfBlock(start);
            return false;
        }
    }

    return true;
}

/// \desc Gets the end of an expression enclosed within parenthesis.
/// \return True if successful, false if an error occurs.
bool Lexer::GetEndOfEnclosedExpression(LocationInfo &end)
{
    LocationInfo start = locationInfo;
    if (PeekNextTokenType() == OPEN_PAREN )
    {
        SkipNextTokenType();
    }

    int64_t parens = 1;

    while( parens > 0 )
    {
        end = locationInfo;
        TokenTypes type = GetNextTokenType(true);
        if ( type == END_OF_SCRIPT )
        {
            return false;
        }
        if ( type == OPEN_PAREN )
        {
            ++parens;
        }
        if ( type == CLOSE_PAREN )
        {
            --parens;
        }
    }

    locationInfo = start;

    return true;
}

/// \desc Generates the statements for a conditional expression
/// \return True if successful or false if an error occurs.
bool Lexer::DefineConditionalExpression()
{
    LocationInfo end;
    if ( !GetEndOfEnclosedExpression(end) )
    {
        return false;
    }

    if ( !DefineStatements(end) )
    {
        return false;
    }
    //Skip the close paren that ends the expression to allow lexing to continue
    //as each expression will need to do this to process the following statement
    //block.
    SkipNextTokenType();

    return true;
}

/// \desc Adds the tokens for a statement block.
/// \param blockStart TokenType to use for the start of the block
/// \param blockEnd TokenType to use for the end of the block.
/// \return True if successful or false if an error occurs.
/// \remark Token types for statement block start and statement block end are used
///         by the parsers expression evaluator to determine when the statement
///         expression begins and ends. This allows the expression evaluator to
///         be used within a recursive loop.
bool Lexer::DefineStatementBlock(LocationInfo start, TokenTypes blockStart, TokenTypes blockEnd)
{
    //Skip the close paren so block parsing works correctly.
    if ( PeekNextTokenType() == CLOSE_PAREN )
    {
        SkipNextTokenType();
    }

    LocationInfo saved = locationInfo;
    //Find the end of the block
    if ( !SkipToEndOfBlock(start) )
    {
        PrintIssue(2126, true, false, "Too many open curly braces in statement block.");
        return false;
    }
    LocationInfo end = locationInfo;

    locationInfo = saved;
    //Skip open block
    SkipNextTokenType();
    saved = locationInfo;

    tokens.push_back(new Token(blockStart));

    if ( !DefineStatements(end, false) )
    {
        return false;
    }

    tokens.push_back(new Token(blockEnd));

    if ( PeekNextTokenType() == CLOSE_BLOCK )
    {
        //Skip the close block.
        SkipNextTokenType();
    }

    return true;
}

/// \desc Processes a block of statements and adds the tokens for those statements to the tokens list.
/// \param end Location of the end of block marker.
/// \param noStatements If true statements will generate an error.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineStatements(LocationInfo end, bool noStatements)
{
    while( locationInfo.location < end.location )
    {
        TokenTypes type = GetNextTokenType(false);
        if ( noStatements && IS_STATEMENT(type) )
        {
            if ( type != VARIABLE_VALUE && type != COLLECTION_VALUE )
            {
                PrintIssue(2129, true, false,
                           "Statements like %s do not return a value that can be "
                           "used to initialize a variable or element of a collection",
                           tokenNames[(int64_t)type & 0xFF]);
                return false;
            }
        }
        if ( type == INVALID_EXPRESSION )
        {
            return false;
        }
        if ( type == ERROR_TOKEN )
        {
            return false;
        }
        if ( type == CLOSE_BLOCK )
        {
            continue;
        }
        type = GenerateTokens(type);
        if ( type == INVALID_TOKEN )
        {
            return false;
        }
    }

    return true;
}

/// \desc Processes the condition and the block of statements for a switch statement.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineSwitch()
{
    LocationInfo start = locationInfo;

    TokenTypes type = INVALID_TOKEN;

    auto *token = new Token(SWITCH_BEGIN);
    tokens.push_back(token);

    token->switchStart = tokens.Count();
    token->switchCondStart = tokens.Count();

    if (PeekNextTokenType() != OPEN_PAREN )
    {
        PrintIssue(2132, true, false, "Missing open curly brace after switch condition");
        SkipToEndOfBlock(start);
        return false;
    }

    SkipNextTokenType();

    tokens.push_back(new Token(SWITCH_COND_BEGIN));

    if ( !DefineConditionalExpression())
    {
        PrintIssue(2135, true, false,
                   "The switch statement's condition is missing a close parenthesis.");
        SkipToEndOfBlock(start);
        return false;
    }

    tokens.push_back(new Token(SWITCH_COND_END));
    token->switchCondEnd = tokens.Count();

    type = GetNextTokenType(true);
    if ( type != OPEN_BLOCK )
    {
        PrintIssue(2138, true, false,
                   "Switch statement open curly brace after condition is missing");
        SkipToEndOfBlock(start);
        return false;
    }
    LocationInfo switchStart = locationInfo;

    if ( !SkipToEndOfBlock(start) )
    {
        return false;
    }
    LocationInfo switchEnd = locationInfo;
    locationInfo = switchStart;
    bool defaultSpecified = false;

    List<LocationInfo> locationsStart;
    List<LocationInfo> locationsEnd;
    List<DslValue *>   dslValues;

    while( locationInfo.location < switchEnd.location )
    {
        type = GetNextTokenType(true);
        if ( type == CLOSE_BLOCK && locationInfo.location == switchEnd.location )
        {
            break;
        }
        if ( type != CASE && type != DEFAULT )
        {
            PrintIssue(2141, true, false,
                       "Switch statement open curly brace after condition is missing");
            SkipToEndOfBlock(start);
            return false;
        }
        if ( type == CASE || type == DEFAULT )
        {
            auto caseValue = DslValue();
            if ( type == CASE )
            {
                LocationInfo save = locationInfo;
                LocationInfo end;
                while( type != END_OF_SCRIPT )
                {
                    previousInfo = locationInfo;
                    type = GetNextTokenType(true, true);
                    if ( type == COLON )
                    {
                        end = previousInfo;
                        locationInfo = save;
                        break;
                    }
                    if ( !IsStaticType(type) )
                    {
                        PrintIssue(2144, true, false,
                                   "Case value must be a simple expression that can be pre-calculated to"
                                   "a constant value like an integer, double, string, char, or boolean");
                        SkipToEndOfBlock(start);
                        return false;
                    }
                }
                if ( !ProcessStaticExpression(&caseValue, end, true, true) )
                {
                    SkipToEndOfBlock(start);
                    return false;
                }
                caseValue.SAV(&caseValue);
            }
            else
            {
                if ( defaultSpecified )
                {
                    PrintIssue(2147, true, false, "A switch statement can only have one default case");
                    SkipToEndOfBlock(start);
                    return false;
                }
                defaultSpecified = true;
                auto *dslValue = new DslValue();
                dslValue->type = DEFAULT;
                caseValue.SAV(dslValue);
            }
            type = GetNextTokenType(true, true);
            if ( type != COLON )
            {
                PrintIssue(2150, true, false,
                           "A colon must follow the case value or default keyword in a switch statement");
                SkipToEndOfBlock(start);
                return false;
            }
            LocationInfo blockStart = locationInfo;
            if ( GetNextTokenType() != OPEN_BLOCK )
            {
                PrintIssue(2153, true, false,
                           "Missing open curly brace after case statement");
                SkipToEndOfBlock(start);
                return false;
            }

            //Make sure cases are not duplicated.
            for(int kk=0; kk<dslValues.Count(); ++kk)
            {
                if ( dslValues[kk]->IsEqual(&caseValue))
                {
                    PrintIssue(2156, true, false,
                               "Case value %s has already used in this switch statement",
                               caseValue.GetAsString());
                    SkipToEndOfBlock(start);
                    return false;
                }
            }

            dslValues.push_back(new DslValue(caseValue));
            locationsStart.push_back(locationInfo);
            if ( !SkipToEndOfBlock(blockStart) )
            {
                return false;
            }
            locationsEnd.push_back(previousInfo);
        }
    }

    for(int64_t ii=0; ii<dslValues.Count(); ++ii)
    {
        dslValues[ii]->location = tokens.Count();
        locationInfo = locationsStart[ii];
        tokens.push_back(new Token(CASE_BLOCK_BEGIN));
        if ( !DefineStatements(locationsEnd[ii], false) )
        {
            SkipToEndOfBlock(start);
            return false;
        }
        tokens.push_back(new Token(CASE_BLOCK_END));
        dslValues[ii]->operand = tokens.Count();
    }

    auto *switchEndToken = new Token(SWITCH_END);
    tokens.push_back(switchEndToken);

    locationInfo = switchEnd;

    token->switchEnd = tokens.Count();

    for(int64_t ii=0; ii<dslValues.Count(); ++ii)
    {
        token->value->cases.CopyFrom(&dslValues);
    }

    return true;
}

/// \desc Checks that the syntax of a for loop is correct. Generates errors
///       if not. On error parsing is moved to the end of the for loop
///       statement block.
/// \param init Reference to the location info structure that receives the start of the initialization section.
/// \param cond Reference to the location info structure that receives the start of the conditional section.
/// \param update Reference to the location info structure that receives the start of the update section.
/// \param block Reference to the location info structure that receives the start of the statement block section.
/// \param end Reference to the location info structure that receives the end of the statement block section.
/// \return True if successful or false if onr or mote errors occur.
bool Lexer::CheckForSyntax(LocationInfo &init, LocationInfo &cond, LocationInfo &update, LocationInfo &block, LocationInfo &end)
{
    LocationInfo save = locationInfo;

    TokenTypes type = GetNextTokenType(true);
    if ( type != OPEN_PAREN )
    {
        PrintIssue(2159, true, false, "An open parenthesis must follow the for keyword");
        return false;
    }

    init = locationInfo;
    if ( !SkipToCharacterBeforeCharacter(';', ')') )
    {
        PrintIssue(2162, true, false, "No semicolon between the initialization and conditional sections");
        return false;
    }

    SkipNextTokenType();
    cond = locationInfo;
    if (!SkipToCharacterBeforeCharacter(';', ')'))
    {
        PrintIssue(2165, true, false, "No semicolon between the conditional and update sections");
        return false;
    }

    SkipNextTokenType();
    update = locationInfo;
    if (!SkipToCharacterBeforeCharacter(')', '{'))
    {
        PrintIssue(2168, true, false, "No close paren before the open block and after the section");
        return false;
    }

    block = locationInfo;
    //Skip the close paren.
    SkipNextTokenType();
    type = GetNextTokenType(false);
    if ( type != OPEN_BLOCK )
    {
        PrintIssue(2171, true, false, "Missing open block and after the close paren");
        return false;
    }

    SkipToEndOfBlock(save);
    end = locationInfo;
    locationInfo = save;

    return true;
}

/// \desc Defines a section of the for loop based on its starting output token type. Drives the
///       define section method.
/// \param beginToken Token to output for the beginning part of the for loop section.
/// \param endToken Token to output for the end part of the for loop section.
/// \param start Start location of the section.
/// \param end End location of the section.
bool Lexer::DefineForSection(TokenTypes beginToken, TokenTypes endToken, LocationInfo start, LocationInfo end)
{
    bool rc = false;
    locationInfo = start;

    if ( beginToken != FOR_BLOCK_BEGIN )
    {
        tokens.push_back(new Token(beginToken));
        rc = DefineStatements(end, false);
        tokens.push_back(new Token(endToken));
    }
    else
    {
        //Skip the close paren.
        rc = DefineStatementBlock(start, FOR_BLOCK_BEGIN, FOR_BLOCK_END);
    }

    return rc;
}

//for ( expression ; conditional expression ; update expression ) { statements }
/// \desc Defines the tokens for a for loop.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineFor()
{
    LocationInfo init;
    LocationInfo cond;
    LocationInfo update;
    LocationInfo block;
    LocationInfo end;

    LocationInfo statementStart = locationInfo;

    if ( !CheckForSyntax(init, cond, update, block, end))
    {
        SkipToEndOfBlock(statementStart);
        return false;
    }

    if ( !DefineForSection(FOR_INIT_BEGIN, FOR_INIT_END, init, cond) )
    {
        SkipToEndOfBlock(statementStart);
        return false;
    }

    if ( !DefineForSection(FOR_COND_BEGIN, FOR_COND_END, cond, update))
    {
        SkipToEndOfBlock(statementStart);
        return false;
    }

    if ( !DefineForSection(FOR_BLOCK_BEGIN, FOR_BLOCK_END, block, end))
    {
        SkipToEndOfBlock(statementStart);
        return false;
    }

    if ( !DefineForSection(FOR_UPDATE_BEGIN, FOR_UPDATE_END, update, block))
    {
        SkipToEndOfBlock(statementStart);
        return false;
    }

    locationInfo = end;
    return true;
}

/// \desc Skips to the end of a code block.
/// \remark sets the previous location information as switch needs to back up one place on case block end.
bool Lexer::SkipToEndOfBlock(LocationInfo start, int64_t errorCode, const char *errorMsg)
{
    locationInfo = start;

    TokenTypes type = INVALID_TOKEN;
    while( type != OPEN_BLOCK )
    {
        type = GetNextTokenType(true);
    }

    int64_t blocks = locationInfo.Blocks();

    while( locationInfo.Blocks() >= blocks )
    {
        previousInfo = locationInfo;
        type = GetNextTokenType(true);
        if ( type == END_OF_SCRIPT )
        {
            PrintPassedIssue(errorCode, true, false, errorMsg);
            return false;
        }
    }
    return true;
}

/// \desc Adds a variable token to the list of tokens in the LEXER structure.
/// \return True if successful, else false if an error occurs.
bool Lexer::AddVariableValue()
{
    auto *variable = variables.Get(&fullVarName);
    auto *token = new Token(variable);

    //If inside a function check local scope since function parameters
    //are defined at local scope.
    if ( definingFunction )
    {
        if ( token->type == FUNCTION_PARAMETER )
        {
            //Force local scope all function parameters are implicitly local scope.
            token->modifier = TMLocalScope;
        }
    }
    token->type = VARIABLE_VALUE;
    if ( token->modifier == TMLocalScope )
    {
        token->value->opcode = PSL;
    }
    else
    {
        token->value->opcode = PSV;
    }
    if ( token->value->type == COLLECTION )
    {
        if ( PeekNextTokenType() != OPEN_BRACE && !definingAndAssigningVariable)
        {
            if (IS_ASSIGNMENT_TOKEN(PeekNextTokenType()))
            {
                PrintIssue(2174, true, false,
                           "Collections can't be reassigned once created.");
                return false;
            }
        }
    }
    //If accessing a field in a collection.
    if ( PeekNextTokenType() == OPEN_BRACE )
    {
        token->type = COLLECTION_VALUE;
        TokenTypes type = INVALID_TOKEN;
        int totalIndexes = 0;
        while( type != CLOSE_BRACE )
        {
            type = GetNextTokenType(false);
            if ( type == ERROR_TOKEN )
            {
                delete token;
                return false;
            }
            if ( type == CLOSE_BRACE )
            {
                totalIndexes++;
                if ( type == ERROR_TOKEN )
                {
                    delete token;
                    return false;
                }
                type = GenerateTokens(type);
                if ( type == ERROR_TOKEN )
                {
                    delete token;
                    return false;
                }
                break;
            }
            if ( type == COMMA )
            {
                //next key
                totalIndexes++;
                type = CLOSE_BRACE;
                type = GenerateTokens(type);
                if ( type == ERROR_TOKEN )
                {
                    delete token;
                    return false;
                }
                type = OPEN_BRACE;
            }
            type = GenerateTokens(type);
            if ( type == ERROR_TOKEN )
            {
                delete token;
                return false;
            }
        }

        auto *dslIndexes = new DslValue(totalIndexes);
        dslIndexes->type = INTEGER_VALUE;
        if (!tokens.push_back(new Token(dslIndexes)))
        {
            delete token;
            delete dslIndexes;
            PrintIssue(2177, true, false,
                       "Out of memory allocating token for collection index");
            return false;
        }
        token->value->opcode = PCV;
        if (IS_ASSIGNMENT_TOKEN(PeekNextTokenType()))
        {
            token->type = COLLECTION_ADDRESS;
            token->value->type = COLLECTION_ADDRESS;
            token->value->opcode = PVA;
            return tokens.push_back(token);
        }
    }

    //Assignments need the address of the variable not the value.
    if (IS_ASSIGNMENT_TOKEN(PeekNextTokenType()))
    {
        token->type = VARIABLE_ADDRESS;
        token->value->type = VARIABLE_ADDRESS;
        token->value->opcode = PVA;
    }

    return tokens.push_back(token);
}

/// \desc Checks if the token to the right of the current token would allow the expression
///       to continue to be evaluated.
bool Lexer::ExpressionContinue(TokenTypes type)
{
    switch( type )
    {
        default:
            return false;
        case CLOSE_PAREN:
            return locationInfo.Parens() > 0;
        case EXPONENT: case MULTIPLY: case DIVIDE: case MODULO: case ADDITION: case SUBTRACTION:
        case BITWISE_SHIFT_LEFT: case BITWISE_SHIFT_RIGHT: case LESS_THAN: case LESS_OR_EQUAL: case GREATER_THAN:
        case GREATER_OR_EQUAL: case EQUAL_TO: case NOT_EQUAL_TO: case BITWISE_AND: case BITWISE_XOR: case BITWISE_OR:
        case LOGICAL_AND: case LOGICAL_OR:
        case COMMA:
            return true;
    }
}

/// \desc Checks if the syntax of a function call is correct.
bool Lexer::CheckFunctionCallSyntax()
{
    U8String tmp;
    tmp.CopyFrom(tmpBuffer);
    U8String tmpFullFunctionName;
    tmpFullFunctionName.CopyFrom(&fullFunName);

    LocationInfo start = locationInfo;

    TokenTypes type = GetNextTokenType(true);
    if ( type != OPEN_PAREN )
    {
        PrintIssue(2180, true, false, "Expected an open paren after the name of the function call");
        locationInfo = start;
        return false;
    }

    while( locationInfo.Parens() > 0 && type != END_OF_SCRIPT )
    {
        type = GetNextTokenType(true);
    }

    if ( type == END_OF_SCRIPT )
    {
        PrintIssue(2183, true, false, "Too many open parenthesis in function call");
        locationInfo = start;
        return false;
    }

    //Have to include this function call in the operator calculation, or it won't correctly determine
    //binary vs unary operations.
    tokens.push_back(new Token(new DslValue(NOP)));
    type = PeekNextTokenType();
    tokens.Resize(tokens.Count()-1);
    if ( type == SEMICOLON )
    {
        locationInfo = start;
        tmpBuffer->CopyFrom(&tmp);
        fullFunName.CopyFrom(&tmpFullFunctionName);
        return true;
    }
    bool rc = ExpressionContinue(type);
    if ( rc )
    {
        locationInfo = start;
        tmpBuffer->CopyFrom(&tmp);
        fullFunName.CopyFrom(&tmpFullFunctionName);
        return true;
    }
    if ( isCollectionElement )
    {
        if (PeekNextTokenType() == CLOSE_BLOCK )
        {
            locationInfo = start;
            tmpBuffer->CopyFrom(&tmp);
            fullFunName.CopyFrom(&tmpFullFunctionName);
            return true;
        }
    }

    PrintIssue(2186, true, false,
               "Function call does not end with a semicolon, or an operator that would allow the "
               "expression to be evaluated.");

    locationInfo = start;
    return false;
}

/// \desc When counting function call parameters this skips any inner function calls that
///       may be made as a parameter.
/// \returns True if the inner function is skipped or false if an error occurs.
bool Lexer::SkipInnerFunctionCall()
{
    int64_t parens = 1;
    SkipNextTokenType();
    while(parens > 0 )
    {
        TokenTypes type = GetNextTokenType(true);
        switch(type)
        {
            case OPEN_PAREN:
                ++parens;
                break;
            case CLOSE_PAREN:
                --parens;
                break;
            case END_OF_SCRIPT:
                PrintIssue(2189, true, false, "Missing close paren in function call");
                return false;
            default:
                break;
        }
    }

    return true;
}

/// \desc Counts how many parameters there are in a function call. Expects the
///       location to be at the open paren of the function call.
/// \return Number of parameters sent to the function.
int64_t Lexer::CountFunctionArguments()
{
    int64_t params = 0;
    TokenTypes type = INVALID_TOKEN;
    LocationInfo start = locationInfo;
    U8String tmpFullFunctionName;
    LocationInfo end;

    tmpFullFunctionName.CopyFrom(&fullFunName);

    //Skip the open paren
    SkipNextTokenType(true);

    //Check for empty function call.
    if (PeekNextTokenType() == CLOSE_PAREN )
    {
        locationInfo = start;
        fullFunName.CopyFrom(&tmpFullFunctionName);
        return params;
    }

    //There is at least 1 parameter.
    params = 1;

    if ( !GetEndOfEnclosedExpression(end) )
    {
        PrintIssue(2192, true, false,
                   "The function call is missing a close paren");
        SkipToEndOfBlock(start);
        locationInfo = start;
        fullFunName.CopyFrom(&tmpFullFunctionName);
        return false;
    }

    while( locationInfo.location < end.location )
    {
        type = GetNextTokenType();
        if ( type == ERROR_TOKEN )
        {
            SkipToEndOfBlock(start);
            locationInfo = start;
            fullFunName.CopyFrom(&tmpFullFunctionName);
            return false;
        }
        if ( type == FUNCTION_CALL )
        {
            if ( !SkipInnerFunctionCall() )
            {
                PrintIssue(2195, true, false, "Missing close paren in function call");
                locationInfo = start;
                fullFunName.CopyFrom(&tmpFullFunctionName);
                return false;
            }
        }
        else if ( type == COMMA )
        {
            ++params;
        }
    }
    locationInfo = start;
    fullFunName.CopyFrom(&tmpFullFunctionName);

    return params;
}

/// Adds a function call token to the list of tokens in the LEXER structure.
/// \return True if successful, else false if an error occurs.
TokenTypes Lexer::AddFunctionCall()
{
    Token *function;
    bool isStandardFunction = false;
    if ( standardFunctions.Exists(&fullFunName) )
    {
        function = standardFunctions.Get(&fullFunName);
        isStandardFunction = true;
    }
    else
    {
        function = functions.Get(&fullFunName);
    }

    //The output token is FUNCTION_CALL_BEGIN which indicates the start of a function call.
    auto *token = new Token(function);
    token->type = FUNCTION_CALL_BEGIN;
    token->modifier = modifier;
    token->value->variableName.CopyFrom(&fullFunName);
    token->identifier->CopyFrom(&fullFunName);
    token->value->variableScriptName.CopyFrom(tmpBuffer);

    int64_t count = CountFunctionArguments();
    if ( count == -1 )
    {
        delete token;
        return INVALID_TOKEN;
    }
    if ( isStandardFunction )
    {

        if ( count < standardFunctionParams[function->value->operand] )
        {
            PrintIssue(2198, true, false,
                       "Function %s requires at least %lld parameters",
                       fullFunName.cStr(),
                       standardFunctionParams[function->value->operand]);
            delete token;
            return INVALID_TOKEN;
        }
    }
    token->value = new DslValue(count);
    if (!tokens.push_back(token) )
    {
        delete token;
        return INVALID_TOKEN;
    }

    //Skip open paren start parens is now 1 greater than close parens for end of function.
    SkipNextTokenType();
    parenthesis = locationInfo.Parens();
    if ( count == 0 )
    {
        SkipNextTokenType(); //skip the close paren for the function call.
    }
    else
    {
        for(int64_t ii=0; ii<count; ++ii)
        {
            if ( LexParam() == ERROR_TOKEN)
            {
                return INVALID_TOKEN;
            }
        }
    }

    if ( !tokens.push_back(new Token(FUNCTION_CALL_END)) )
    {
        return INVALID_TOKEN;
    }

    return FUNCTION_CALL_END;
}

/// \desc Creates the param token sequence for a function call parameter.
TokenTypes Lexer::LexParam()
{
    tokens.push_back(new Token(PARAM_BEGIN));
    TokenTypes type = INVALID_TOKEN;
    int64_t parens = 1; //starts after open paren on function call.
    while( type != COMMA && parens > 0 )
    {
        type = GetNextTokenType(false);
        if ( type == ERROR_TOKEN )
        {
            return ERROR_TOKEN;
        }
        if ( type == OPEN_PAREN )
        {
            ++parens;
        }
        if ( type == CLOSE_PAREN )
        {
            --parens;
            if ( parens == 0 )
            {
                continue;
            }
        }
        if( type != COMMA )
        {
            type = GenerateTokens(type);
        }
        if ( type == INVALID_TOKEN )
        {
            return ERROR_TOKEN;
        }
    }

    tokens.push_back(new Token(PARAM_END));

    return type;
}


/// \desc Gets the single line comment.
/// \return True if successful, else false if an error occurs.
bool Lexer::GetSingleLineComment()
{
    u8chr ch;

    ch = GetCurrent();
    if ( !tmpBuffer->push_back(ch) )
    {
        return false; //fatal error condition out of memory.
    }
    LocationInfo save = locationInfo;
    locationInfo.Increment(ch);
    ch = GetCurrent();
    if ( !tmpBuffer->push_back(ch) )
    {
        locationInfo = save;
        return false; //fatal error condition out of memory.
    }

    while(locationInfo.location < parseBuffer.Count())
    {
        //Since this is a comment ignore grouping symbols.
        if ( ch == '(' || ch == ')' || ch == '{' || ch == '}' )
        {
            locationInfo.Increment(' ');
        }
        else
        {
            locationInfo.Increment(ch);
        }
        ch = GetCurrent();
        if ( !tmpBuffer->push_back(ch) )
        {
            return false; //fatal error condition out of memory.
        }

        if ( ch == '\n')
        {
            break;
        }
    }
    return true;
}

/// \desc   Extracts a multi-line comment from the code source. The comment is not
///         parsed simply extracted.
/// \return True if successful, else false if an error occurs.
/// \remark Expects to begin at the /* and that the /* has already been
///         verified as present in the _s. The function is created to be recursive
///         to allow multi-line comment to be contained within multi-line comment.
bool Lexer::GetMultiLineComment()
{
    u8chr ch;

    int64_t comments = 0;

    while(locationInfo.location < parseBuffer.Count())
    {
        ch = GetCurrent();
        u8chr ch1 = PeekNextChar();
        if ( !tmpBuffer->push_back(ch) )
        {
            return false; //fatal error condition out of memory.
        }
        //Since this is a comment ignore grouping symbols.
        if ( ch == '(' || ch == ')' || ch == '{' || ch == '}' )
        {
            locationInfo.Increment(' ');
        }
        else
        {
            locationInfo.Increment(ch);
        }
        if ( ch == '/' && ch1 == '*' )
        {
            ++comments;
            continue;
        }
        if ( ch == '*' && ch1 == '/' )
        {
            --comments;
            continue;
        }
        if ( comments == 0 )
        {
            return true;
        }
    }

    PrintIssue(2201, true, false, "Closing */ not found before end of _s");
    return false;
}

/// \desc Reads characters up to the next statement terminator or end of _s.
TokenTypes Lexer::SkipToEndOfStatement()
{
    TokenTypes type = INVALID_TOKEN;

    while ( type != END_OF_SCRIPT && type != SEMICOLON && !fatal )
    {
        type = GetNextTokenType(true);
    }

    return type;
}

/// \desc Gets the next token to be read without moving the lexing location.
/// \param skip Number of tokens to skip ahead, default is 1 which is the next token.
/// \return The next token type to be read.
TokenTypes Lexer::PeekNextTokenType(int64_t skip, bool checkColon)
{
    LocationInfo saved = locationInfo;

    TokenTypes saveLast = last;
    TokenTypes savePrev = prev;

    auto *tmp = new U8String(tmpBuffer);
    if ( skip > 1 )
    {
        for(int ii=1; ii<skip; ++ii)
        {
            SkipNextTokenType();
        }
    }

    TokenTypes type = GetNextTokenType(true, checkColon);
    tmpBuffer->CopyFrom(tmp);
    delete tmp;
    locationInfo = saved;
    last = saveLast;
    prev = savePrev;

    return type;
}

/// \desc Gets the next character that is not a while space character or a multi-line or single line comment.
/// \return The next character or U8_NULL_CHR if at the end of the _s or INVALID_UTF_CHAR if out of
///         memory from parsing a single or multi-line comment.
u8chr Lexer::GetNextNonCommentCharacter()
{
    u8chr ch = SkipWhiteSpace();

    //if end of code encountered
    if (ch == U8_NULL_CHR)
    {
        return ch;
    }

    if (IsSingleLineComment(ch))
    {
        if (GetSingleLineComment())
        {
            return GetNextNonCommentCharacter();
        }
        return INVALID_UTF_CHAR;
    }

    if (IsMultiLineComment(ch))
    {
        if (GetMultiLineComment())
        {
            return GetNextNonCommentCharacter();
        }
        return INVALID_UTF_CHAR;
    }

    return ch;
}

/// \desc Skips the next token setting the lex point to the token after the current token.
void Lexer::SkipNextTokenType(bool checkColon)
{
    (void) GetNextTokenType(true, checkColon);
}

/// \desc Checks if the next token is a number value.
bool Lexer::IsNumber(u8chr ch)
{
    bool rc = false;
    if ( IS_NUMBER(ch) )
    {
        //Unique case where a number begins an identifier.
        if (IS_ID_START(GetNext()) )
        {
            return false;
        }
        rc = true;
    }
    else if ( ch == '-' || ch == '+' )
    {
        LocationInfo save = locationInfo;
        locationInfo.Increment(ch);
        ch = GetCurrent();
        if ( IS_NUMBER(ch) )
        {
            if ( tokens.Count() == 0 )
            {
                rc = true;
            }
            else
            {
                if ( prev == OPEN_BLOCK || prev == OPEN_PAREN || prev == OPEN_BRACE ||
                     prev == EXPONENT || prev == ASSIGNMENT || prev == MULTIPLY_ASSIGNMENT ||
                     prev == DIVIDE_ASSIGNMENT || prev == MODULO_ASSIGNMENT || prev == ADD_ASSIGNMENT ||
                     prev == SUBTRACT_ASSIGNMENT || prev == COMMA || prev == SEMICOLON || prev == RETURN ||
                     prev == IF_COND_BEGIN || prev == IF_BLOCK_BEGIN || prev == ELSE_BLOCK_BEGIN ||
                     prev == WHILE_COND_BEGIN || prev == WHILE_BLOCK_BEGIN || prev == FOR_INIT_BEGIN ||
                     prev == FOR_COND_BEGIN || prev == FOR_UPDATE_BEGIN || prev == FOR_BLOCK_BEGIN ||
                     prev == SWITCH_COND_BEGIN || prev == SWITCH_BEGIN || prev == CASE_COND_BEGIN ||
                     prev == CASE_BLOCK_BEGIN || prev == DEFAULT_BLOCK_BEGIN || prev == FUNCTION_DEF_BEGIN ||
                     prev == FUNCTION_CALL_BEGIN || prev == PARAM_BEGIN || prev == SWITCH_COND_END)
                {
                    rc = true;
                }
            }
        }
        locationInfo = save;
    }

    return rc;
}

/// \desc Gets the next token Type based on location from the code source.
/// \param ignoreErrors If true no errors will be generated. This is used when
///                     recursively calling this method.
/// \return The next token or INVALID_TOKEN if an error or at end of code _s.
TokenTypes Lexer::GetNextTokenType(bool ignoreErrors, bool checkForColon)
{
    prev = last;
    TokenTypes type = ProcessGetNextTokenType(ignoreErrors, checkForColon);
    last = type;

    if ( ignoreErrors )
    {
        return type;
    }

    if ( (IS_VALUE_TYPE(type) && IS_VALUE_TYPE(prev)) ||
         (IS_VALUE_TYPE(type) && prev == VARIABLE_VALUE) ||
         (type == VARIABLE_VALUE && IS_VALUE_TYPE(prev)) ||
         (type == VARIABLE_VALUE && prev == VARIABLE_VALUE)
            )
    {
        PrintIssue(2204, true, false, "Missing operation or separator between variables or values");
        return INVALID_EXPRESSION;
    }

    return type;
}

TokenTypes Lexer::ProcessGetNextTokenType(bool ignoreErrors, bool checkForColon)
{
    u8chr ch = GetNextNonCommentCharacter();
    //if end of code encountered
    if (ch == U8_NULL_CHR)
    {
        return END_OF_SCRIPT;
    }
    if ( ch == INVALID_UTF_CHAR )
    {
        if ( !ignoreErrors )
        {
            return ERROR_TOKEN;
        }
    }
    if ( checkForColon && ch == ':' )
    {
        locationInfo.Increment(ch);
        return COLON;
    }
    if (ch == '$' && PeekNextChar() == '\"' )
    {
        if (GetStringDirect(ignoreErrors))
        {
            return STRING_VALUE;
        }
        return ERROR_TOKEN;
    }
    if (ch == '\"')
    {
        if (GetString(ignoreErrors))
        {
            return STRING_VALUE;
        }
        return ERROR_TOKEN;
    }
    if (ch == '\'')
    {
        if (GetCharacterValue(ignoreErrors))
        {
            return CHAR_VALUE;
        }
        if ( !ignoreErrors )
        {
            return ERROR_TOKEN;
        }
        return CHAR_VALUE;
    }
    if (IsNumber(ch))
    {
        if (GetNumber(ignoreErrors))
        {
            return (tmpValue->type == DOUBLE_VALUE) ? DOUBLE_VALUE : INTEGER_VALUE;
        }
        if ( !ignoreErrors )
        {
            return ERROR_TOKEN;
        }
        return INVALID_TOKEN;
    }
    if (IsOperatorChar(ch))
    {
        return GetOperatorToken(ignoreErrors);
    }
    //At this point the character is not whitespace, operator, or number, so it
    //is either a valid identifier or an error.
    if (!IS_ID_START(ch))
    {
        //The member access operator is included in the identifier string if present.
        if (!ignoreErrors)
        {
            PrintIssue(2207, true, false,
                       "Unexpected character '%c', expected a key word,"
                       "variable, function, value or operator", ch);
            return ERROR_TOKEN;
        }
        locationInfo.Increment(ch); //Skip invalid character and attempt to continue.
        return GetNextTokenType(ignoreErrors);
    }

    if (!GetIdentifier())
    {
        //Get identifier stops as soon as an invalid character is encountered.
        //So the only error that can occur is if memory can't be allocated which
        //is a fatal error.
        return ERROR_TOKEN;
    }

    TokenTypes type = GetKeyWordTokenType(ignoreErrors);
    if ( type == STOP )
    {
        return END_OF_SCRIPT;
    }

    //Remap TRUE and FALSE to a boolean value type and return bool value token type.
    if (type == TRUE || type == FALSE)
    {
        tmpValue->type   = BOOL_VALUE;
        tmpValue->bValue = (type == TRUE);
        return BOOL_VALUE;
    }
    else if ( type == BRK )
    {
        //Need to manually add a debug break point as signal and debug break
        //do not work on all systems.
        printf("BRK encountered.\n");
        LocationInfo saved = locationInfo;
        type = GetNextTokenType(false);
        if ( type != SEMICOLON )
        {
            PrintIssue(2210, true, false, "Expected a semi-colon after break");
            locationInfo = saved;
        }
        else
        {
            //tokens.push_back(new Token(BRK));
            //consume the semicolon so the next token is what would have been
            //encountered without the brk;
            type = GetNextTokenType(false);
        }
    }
    //If the identifier is not an invalid type it has to be a valid key word.
    if ( type != INVALID_TOKEN )
    {
        return type;
    }

    //If var has been the text work is either a modifier, function or variable name, or an error.
    if ( varSpecified )
    {
        if ( type == LOCAL || type == GLOBAL || type == SCRIPT || type == CONST )
        {
            return type;
        }

        if ( type != INVALID_TOKEN )
        {
            varSpecified = false;
            if ( !ignoreErrors )
            {
                PrintIssue(2213, true, false,
                           "%s can't be used as a variable or function name as it is a key word",
                           tmpBuffer->cStr());
            }
            return ERROR_TOKEN;
        }

        if (IsFunctionDefined(ignoreErrors))
        {
            //Check if this is a function redefinition.
            if ( CheckIfFunctionDefined() )
            {
                varSpecified = false;
                if ( !ignoreErrors )
                {
                    PrintIssue(2216, true, false,
                               "Function %s has already been defined", tmpBuffer->cStr());
                    return ERROR_TOKEN;
                }
            }
        }

        if (IsVariableDefined(ignoreErrors))
        {
            varSpecified = false;
            if ( !ignoreErrors )
            {
                PrintIssue(2219, true, false,
                           "Variable %s has already been defined", tmpBuffer->cStr());
                return ERROR_TOKEN;
            }
        }

        if ( !ignoreErrors )
        {
            varSpecified = false;
            //Check if an attempt to create a function without specifying an open paren?
            TokenTypes next = PeekNextTokenType();
            if ( next == CLOSE_PAREN )
            {
                return ERROR_TOKEN;
            }
        }

        varSpecified = false;
        //Define Variable determines if this is actually a variable or a function definition.
        //tmp buffer has the short name but not the full scope path name
        //modifier has the scope level
        return VARIABLE_DEF;
    }

    //A variable or function is being accessed.
    //Scope modifiers need to be checked as they can only be specified when
    //the variable or function is being defined.
    if ( type == LOCAL || type == GLOBAL || type == SCRIPT || type == CONST )
    {
        PrintIssue(2222, true, false, "Variable modifiers can only be specified at variable creation");
        return ERROR_TOKEN;
    }

    //See if this is a script function parameter name.
    if (definingFunctionsParameters)
    {
        U8String name = U8String(tmpBuffer);
        GetFullName(&name, TMLocalScope);
        if ( variables.Exists(&name) )
        {
            if ( !ignoreErrors )
            {
                PrintIssue(2225, true, false, "Parameter %s has already been defined",
                           tmpBuffer->cStr());
                return ERROR_TOKEN;
            }
        }

        return FUNCTION_PARAMETER;
    }

    //This is either a function call or
    if (IsFunctionDefined(ignoreErrors))
    {
        return FUNCTION_CALL;
    }

    //Check if this is a variable access.
    if (IsVariableDefined(ignoreErrors))
    {
        return VARIABLE_VALUE;
    }

    //Else this identifier is not a valid program construct.
    if ( !ignoreErrors )
    {
        PrintIssue(2228, true, false, "Unknown identifier %s", tmpBuffer->cStr());
    }

    return ERROR_TOKEN;
}

/// \desc Checks if the named function has already been defined.
/// \param ignoreErrors True if being called recursively like in function definition, else false.
/// \return True if the function has been defined, else false.
/// \remark The full function name is returned in fullFunName.
bool Lexer::IsFunctionDefined(bool ignoreErrors)
{
    U8String tmp;
    tmp.CopyFrom(tmpBuffer);

    //Built in and external functions do not have any scope and override all other functions
    //though they can't be overridden like a global function which can be overridden by
    //a script level function for the script it is defined in.
    if ( standardFunctions.Exists(tmpBuffer) )
    {
        fullFunName.CopyFrom(tmpBuffer);
        return true;
    }

    GetFullName(&tmp, TMScriptScope);
    if ( functions.Exists(&tmp) )
    {
        fullFunName.CopyFrom(&tmp);
        return true;
    }
    tmp.CopyFrom(tmpBuffer);
    GetFullName(&tmp, TMGlobalScope);
    if ( functions.Exists(&tmp) )
    {
        return true;
    }

    return false;
}

/// \desc Check if the function is actually defined or if this is a pass 1 prototype only.
/// \remark When the function is actually defined in pass 2 the prototype is replaced with
///         the functions actual definition.
bool Lexer::CheckIfFunctionDefined()
{
    if ( !varSpecified )
    {
        return true;
    }

    Token *t = functions.Get(&fullFunName);
    if ( t == nullptr )
    {
        return false;
    }
    return t->type == FUNCTION_DEF_BEGIN;
}

/// \desc Generates the full variable or function name. The full name includes
///       the scope, module, [current function if local], and variable or function name.
/// \param fullName The buffer to receive the full path name of the variable or function.
/// \param scope The scope to use for generating the name.
/// \remark The functions and variables hashmaps always use the full name as a key.
/// \remark Standard functions which include external functions do not have an associated
///         scope and are called directly.
void Lexer::GetFullName(U8String *fullName, TokenModifiers scope)
{
    switch( scope )
    {
        case TMLocalScope:
            fullName->Clear();
            fullName->CopyFromCString("TMLocalScope.");
            fullName->push_back(&module);
            fullName->push_back('.');
            fullName->push_back(&currentFunction);
            fullName->push_back('.');
            fullName->push_back(tmpBuffer);
            break;
        case TMScriptScope:
            fullName->Clear();
            fullName->CopyFromCString("TMScriptScope.");
            fullName->push_back(&module);
            fullName->push_back('.');
            fullName->push_back(tmpBuffer);
            break;
        case TMGlobalScope:
            fullName->Clear();
            bool rc = false;
            if ( tmpBuffer->IndexOf('.') == -1 )
            {
                fullName->CopyFromCString("TMGlobalScope.");
                fullName->push_back(&module);
                fullName->push_back('.');
                fullName->push_back(tmpBuffer);
            }
            else
            {
                fullName->CopyFromCString("TMGlobalScope.");
                fullName->push_back(tmpBuffer);
            }
            break;
    }
}

/// \desc variable _n is in fullPathName buffer, Current scope level is defined in currentScope.
/// \return True if the variable has been defined, else false. The full path name of the
///         variable is returned in fullVarName.
/// \remark variable _n is in fullPathName buffer, Current scope level is defined in currentScope.
bool Lexer::IsVariableDefined(bool ignoreErrors)
{
    GetFullName(&fullVarName, TMLocalScope);
    if (variables.Exists(&fullVarName))
    {
        return true;
    }

    GetFullName(&fullVarName, TMScriptScope);
    if (variables.Exists(&fullVarName))
    {
        return true;
    }

    GetFullName(&fullVarName, TMGlobalScope);
    if (variables.Exists(&fullVarName))
    {
        return true;
    }

    return false;
}

/// \desc Get the next non white space character.
/// \return The next non white space character or U8_NULL_CHR if at the end of the _s.
u8chr Lexer::SkipToNextNonWhiteSpaceCh()
{
    locationInfo.Increment(GetCurrent());
    return SkipWhiteSpace();
}

/// \desc Skip ahead to the character or stay at same parse location if the character is not found.
/// \param toCh character to search for.
/// \param beforeCh character that must occur after the to character.
/// \return True if the to character is encountered before the before character, else false.
bool Lexer::SkipToCharacterBeforeCharacter(u8chr toCh, u8chr beforeCh)
{
    LocationInfo saved = locationInfo;
    u8chr ch1;
    do
    {
        ch1 = SkipToNextNonWhiteSpaceCh();
    } while(ch1 != toCh && ch1 != U8_NULL_CHR && ch1 != beforeCh);

    if ( ch1 == toCh && ch1 != beforeCh)
    {
        return true;
    }

    locationInfo = saved;
    return false;
}

void Lexer::Initialize()
{
    modules.Clear();
}

// \desc Translates the list of modules and their associated scripts into tokens.
// \param modules List of modules one per _s _f.
// \param scripts List of _s files to tokenize.
// \return True if no errors or warnings occur, else false.
int64_t Lexer::AddModule(const char *moduleName, const char *file, const char *script)
{
    int64_t id = modules.Count() + 1;
    modules.push_back(new Module(moduleName, file, script));

    return id;
}

/// \desc Processes and translates all of the added modules.
// \return True if no errors or warnings occur, else false.
bool Lexer::Lex()
{
    auto *tmp = new U8String("var");
    auto *name = new U8String(tmpBuffer);

    //Add placeholder tokens for all defined functions.
    for(int64_t ii=0; ii<modules.Count(); ++ii)
    {
        modifier = TMScriptScope;
        varSpecified = false;
        constSpecified = false;
        parseBuffer.CopyFrom(modules[ii]->Script());
        locationInfo.Reset();
        errors = 0;
        warnings = 0;

        module.Clear();
        module.push_back(modules[ii]->Name());
        TokenTypes type = PeekNextTokenType();
        while (type != END_OF_SCRIPT)
        {
            type = GetNextTokenType(true);
            if ( type == STOP || type == LEND )
            {
                break;
            }
            if ( type == VAR )
            {
                varSpecified = true;
                type = GetNextTokenType(true);
                if ( type == GLOBAL )
                {
                    modifier = TMGlobalScope;
                    type = GetNextTokenType(true);
                }
                else if ( type == SCRIPT )
                {
                    modifier = TMScriptScope;
                    type = GetNextTokenType(true);
                }
                if ( type == VARIABLE_DEF )
                {
                    name->CopyFrom(tmpBuffer);
                    type = GetNextTokenType(true);
                    if ( type == OPEN_PAREN )
                    {
                        //found a function definition.
                        tmpBuffer->CopyFrom(name);
                        GetFullName(name, modifier);
                        if ( functions.Exists(name) )
                        {
                            PrintIssue(2231, true, false,
                                       "Function %s already defined in module %s",
                                       name->cStr(), modules[ii]->Name());
                        }
                        else
                        {
                            //Placeholder token when the actual function is defined this is replaced.
                            functions.Set(name, new Token(INVALID_TOKEN));
                        }
                    }
                    modifier = TMScriptScope;
                    varSpecified = false;
                    constSpecified = false;
                }
            }
        }
    }

    if ( fatal || errors != 0 || warnings != 0 )
    {
        return false;
    }

    //All functions found, compile the program.
    for(int64_t ii=0; ii<modules.Count(); ++ii)
    {
        if ( !Lex(ii+1) )
        {
            return false;
        }
    }

    return true;
}

/// \desc Processes and translates the specified module at index.
/// \index Value of the module to lex.
/// \return True if the lex operation is successful, false if an error occurs.
bool Lexer::Lex(int64_t id)
{
    modifier = TMScriptScope;
    varSpecified = false;
    constSpecified = false;

    int64_t start = tokens.Count();

    if ( id < 1 || id > modules.Count() )
    {
        PrintIssue(2234, true, false,
                   "Module does not exist. Modules are added with AddModule");
        return false;
    }

    parseBuffer.CopyFrom(modules[id-1]->Script());

    locationInfo.Reset();
    errors = 0;
    warnings = 0;

    finished = false;

    module.Clear();
    module.push_back(modules[id-1]->Name());

    while(!finished)
    {
        if (locationInfo.location >= parseBuffer.Count())
        {
            finished = true;
            continue;
        }

        TokenTypes type = ReadNextToken();
        if (type == ERROR_TOKEN)
        {
            break;
        }
        if (type == END_OF_SCRIPT)
        {
            finished = true;
            continue;
        }

        type = GenerateTokens(type);
    }

    if ( errors == 0 )
    {
        for(int64_t ii=start; ii<tokens.Count(); ++ii)
        {
            tokens[ii]->value->moduleId = id;
        }
    }

    return errors == 0 && warnings == 0;
}

/// \desc Adds the named function to the standard functions list.
/// \return True if successful or false if an error eom occurs.
bool Lexer::AddStandardFunction(U8String function, int64_t index)
{
    if ( !standardFunctions.Exists(&function))
    {
        auto *token = new Token(FUNCTION_DEF_BEGIN);
        token->identifier = new U8String(&function);
        token->value = new DslValue(JBF, index);
        if (!standardFunctions.Set(token->identifier, token) )
        {
            return false;
        }
    }

    return true;
}

/// \desc   Adds the functions and variables that come standard
///         with the DSL.
/// \remark These functions were built in key words before
///         functions were redesigned to always contain a
///         variable number of arguments.
bool Lexer::AddStandardAssets()
{
    for(int64_t ii=0; ii<totalStandardFunctions; ++ii)
    {
        if( !AddStandardFunction(U8String(standardFunctionNames[ii]), ii) )
        {
            return false;
        }
    }

    //add line feed \n const
    auto newLine = U8String("LF");
    if ( !standardVariables.Exists(&newLine))
    {
        auto *token = new Token(VARIABLE_DEF);
        token->readyOnly = true;
        token->value = new DslValue();
        token->value->type = CHAR_VALUE;
        token->value->cValue = 0x0a;
        token->identifier = new U8String(newLine);
        if (!standardVariables.Set(token->identifier, token) )
        {
            return false;
        }
    }

    return true;
}

/// \desc Reads the next token in the code _s.
/// \return TokenType of the next token read from the parse buffer.
TokenTypes Lexer::ReadNextToken()
{
    tmpBuffer->Clear();
    u8chr ch = SkipWhiteSpace();
    if ( ch == U8_NULL_CHR )
    {
        //End of code _s reached.
        finished = true;
        return END_OF_SCRIPT;
    }

    TokenTypes  type = GetNextTokenType(false);

    if ( fatal )
    {
        finished = true;
        return END_OF_SCRIPT;
    }

    return type;
}

/// \desc GenerateTokens produces the output for the parser.
TokenTypes Lexer::GenerateTokens(TokenTypes type)
{
    switch( type )
    {
        case END_OF_SCRIPT:
            finished = true;
            return type;
        case LEND: case FUNCTION_CALL_BEGIN: case PARAM_BEGIN: case PARAM_END: case INVALID_TOKEN:
        case KEY_BEGIN: case KEY_END: case STOP: case VARIABLE_ADDRESS: case COLLECTION_ADDRESS:
            return type;
        case ERROR_TOKEN:
            return ERROR_TOKEN;
        case CLOSE_BRACE:
            tokens.push_back(new Token(KEY_END));
            return type;
        case OPEN_BRACE:
            tokens.push_back(new Token(KEY_BEGIN));
            return type;
        case OPEN_PAREN: //expression not a function definition.
            ++parenthesis;
            tokens.push_back(new Token(type));
            return type;
        case CLOSE_PAREN: //expression not a function definition.
            --parenthesis;
            if ( parenthesis < 0 )
            {
                PrintIssue(2237, true, false,
                           "Close parenthesis without a matching open parenthesis");
                return ERROR_TOKEN;
            }
            tokens.push_back(new Token(type));
            return type;
        case SEMICOLON:
            //Statement tracking types reset when statement end specified.
            modifier = TMScriptScope;
            varSpecified = false;
            constSpecified = false;
        case BITWISE_SHIFT_LEFT: case BITWISE_SHIFT_RIGHT: case BITWISE_AND: case BITWISE_XOR: case BITWISE_OR:
        case LESS_OR_EQUAL: case GREATER_OR_EQUAL: case EQUAL_TO:
        case NOT_EQUAL_TO: case LOGICAL_AND: case LOGICAL_OR:
        case UNARY_POSITIVE: case UNARY_NEGATIVE: case UNARY_NOT:  case LESS_THAN: case GREATER_THAN: case COMMA:
        case EXPONENT: case MULTIPLY: case DIVIDE: case MODULO: case ADDITION: case SUBTRACTION:
        case FUNCTION_CALL_END:
        case CAST_TO_INT: case CAST_TO_DBL: case CAST_TO_CHR: case CAST_TO_STR: case CAST_TO_BOOL:
            tokens.push_back(new Token(type));
            return type;
            //Prefix inc and dec act like post fix for the variable they increment or decrement.
            //The key difference is where the increment or decrement occur and the parser expression
            //evaluator arranges their order so the inc and dev operation occurs in the correct order.
        case PREFIX_INC:
        {
            GetNextTokenType();
            auto *variable = variables.Get(&fullVarName);
            auto *t = new Token(variable);
            if ( t->modifier == TMLocalScope )
            {
                t->value->opcode = INL;
            }
            else
            {
                t->value->opcode = INC;
            }
            t->type = PREFIX_INC;
            tokens.push_back(t);
            return type;
        }
        case PREFIX_DEC:
        {
            GetNextTokenType();
            auto *variable = variables.Get(&fullVarName);
            auto *t = new Token(variable);
            if ( t->modifier == TMLocalScope )
            {
                t->value->opcode = DEL;
            }
            else
            {
                t->value->opcode = DEC;
            }
            t->type = PREFIX_DEC;
            tokens.push_back(t);
            return type;
        }
        case POSTFIX_INC:
        {
            //postfix needs variable to increment or decrement.
            Token *prevToken = tokens[tokens.Count()-1];

            auto *variable = variables.Get(prevToken->identifier);
            auto *t = new Token(variable);
            t->type = POSTFIX_INC;
            t->value->type = POSTFIX_INC;
            if ( t->modifier == TMLocalScope )
            {
                t->value->opcode = INL;
            }
            else
            {
                t->value->opcode = INC;
            }
            t->type = PREFIX_INC;
            tokens.push_back(t);
            return type;
        }
        case POSTFIX_DEC:
        {
            //postfix needs variable to increment or decrement.
            Token *prevToken = tokens[tokens.Count()-1];
            auto *variable = variables.Get(prevToken->identifier);
            auto *t = new Token(variable);
            t->type = POSTFIX_DEC;
            t->value->type = POSTFIX_DEC;
            if ( t->modifier == TMLocalScope )
            {
                t->value->opcode = DEL;
            }
            else
            {
                t->value->opcode = DEC;
            }
            t->type = PREFIX_DEC;
            tokens.push_back(t);
            return type;
        }
        case ASSIGNMENT: case MULTIPLY_ASSIGNMENT: case DIVIDE_ASSIGNMENT: case MODULO_ASSIGNMENT: case ADD_ASSIGNMENT:
        case SUBTRACT_ASSIGNMENT:
        {
            if (definingFunctionsParameters)
            {
                PrintIssue(2240, true, false, "Function parameters may not be assigned values");
                return ERROR_TOKEN;
            }
            if ( tokens.Count() == 0 )
            {
                PrintIssue(2243, true, false, "Assignment attempted without a variable");
                return ERROR_TOKEN;
            }
            Token *prevOutputToken = tokens[tokens.Count() - 1];
            if ( prevOutputToken->readyOnly )
            {
                PrintIssue(2246, true, false,
                           "%s can't be assigned to as it is read only",
                           prevOutputToken->value->variableName.cStr());
                return ERROR_TOKEN;
            }
            if (!IS_ASSIGN_TYPE(prevOutputToken->type) &&
                prevOutputToken->type != COLLECTION_VALUE &&
                prevOutputToken->type != VARIABLE_ADDRESS &&
                prevOutputToken->type != COLLECTION_ADDRESS)
            {
                PrintIssue(2249, true, false, "Assignment attempted to something other than a variable");
                return ERROR_TOKEN;
            }
            auto *tokenAssign = new Token(prevOutputToken);
            tokenAssign->type = type;
            tokenAssign->value->variableName.CopyFrom(prevOutputToken->identifier);
            tokens.push_back(tokenAssign);
            return type;
        }
        case OPEN_BLOCK: //open blocks can be functions, if, else, elseif, switch, case, and collections.
            if ( definingFunctionsParameters )
            {
                return type;
            }
            if ( definingFunction )
            {
                blockCount++;
                if ( blockCount == 1 )
                {
                    varSpecified = false;
                    return type;
                }
            }
            tokens.push_back(new Token(type));
            return type;
        case CLOSE_BLOCK: //open blocks can be functions, if, else, elseif, switch, case, and collections.
            if ( definingFunctionsParameters )
            {
                return type;
            }
            if ( definingFunction )
            {
                blockCount--;
                if ( blockCount == 0 )
                {
                    definingFunction = false;
                    tokens.push_back(new Token(FUNCTION_DEF_END));
                    return type;
                }
            }
            if (prev == OPEN_BLOCK )
            {
                //if this is nothing more than an open curly brace followed by a close curly brace remove it
                //as it is meaningless.
                tokens.pop_back();
                return type;
            }
            tokens.push_back(new Token(type));
            return type;
        case WHILE:
            if ( !CheckWhileSyntax() )
            {
                return ERROR_TOKEN;
            }
            if ( !DefineWhile() )
            {
                return ERROR_TOKEN;
            }
            return type;
        case IF:
            if ( !DefineIf() )
            {
                return ERROR_TOKEN;
            }
            return type;
        case FOR:
            if ( !DefineFor() )
            {
                return ERROR_TOKEN;
            }
            return type;
        case CASE:
            PrintIssue(2252, true, false, "The case key word can only be used inside a switch statement");
            return ERROR_TOKEN;
        case DEFAULT:
            PrintIssue(2255, true, false, "The default must be used inside a switch statement");
            return type;
        case SWITCH:
            if ( !DefineSwitch() )
            {
                return ERROR_TOKEN;
            }
            return type;
        case RETURN:
            if ( !definingFunction )
            {
                PrintIssue(2258, true, false,
                           "The return statement can only be used inside a function");
                return ERROR_TOKEN;
            }
            if ( PeekNextTokenType() == SEMICOLON )
            {
                PrintIssue(2261, true, false,
                           "The return statement requires a value to return");
                return ERROR_TOKEN;
            }
        case CONTINUE: case BREAK:
            tokens.push_back(new Token(type));
            return type;
        case BOOL_VALUE: case INTEGER_VALUE: case DOUBLE_VALUE: case CHAR_VALUE: case STRING_VALUE:
        {
            tokens.push_back(new Token(tmpValue));
            return type;
        }
        case FUNCTION_CALL:
            if ( CheckFunctionCallSyntax() )
            {
                return AddFunctionCall();
            }
            return type;
        case FUNCTION_DEF_BEGIN:
            PrintIssue(2264, true, false, "Functions cannot be defined within functions");
            return ERROR_TOKEN;
        case MULTI_LINE_COMMENT: case SINGLE_LINE_COMMENT:
            tokens.push_back(new Token(type, tmpBuffer));
            return type;
        case VAR:
            modifier = TMScriptScope;
            varSpecified = true;
            return type;
        case CONST:
            constSpecified = true;
            return type;
        case GLOBAL:
            modifier = TMGlobalScope;
            return type;
        case SCRIPT:
            modifier = TMScriptScope;
            return type;
        case LOCAL:
            modifier = TMLocalScope;
            return type;
        case VARIABLE_DEF:
            //modifier contains the scope level for the variable.
            if ( !DefineVariable() )
            {
                return ERROR_TOKEN;
            }
            return type;
        case VARIABLE_VALUE:
            if ( !AddVariableValue() )
            {
                return ERROR_TOKEN;
            }
            return type;
        case COLON:
        case FALSE: case TRUE:
        case PARAMETER_VALUE: case BRK: case FUNCTION_DEF_END: case FUNCTION_PARAMETER:
        case BLOCK: case ELSE: case IF_COND_BEGIN: case IF_COND_END: case IF_BLOCK_BEGIN: case IF_BLOCK_END:
        case ELSE_BLOCK_BEGIN: case ELSE_BLOCK_END: case WHILE_COND_BEGIN: case WHILE_COND_END: case WHILE_BLOCK_BEGIN:
        case WHILE_BLOCK_END: case FOR_INIT_BEGIN: case FOR_INIT_END: case FOR_COND_BEGIN: case FOR_COND_END:
        case FOR_UPDATE_BEGIN: case FOR_UPDATE_END: case FOR_BLOCK_BEGIN: case FOR_BLOCK_END: case SWITCH_COND_BEGIN:
        case SWITCH_COND_END: case SWITCH_BEGIN: case SWITCH_END: case CASE_COND_BEGIN: case CASE_COND_END:
        case CASE_BLOCK_BEGIN: case CASE_BLOCK_END: case DEFAULT_BLOCK_BEGIN: case DEFAULT_BLOCK_END: case COLLECTION:
        case COLLECTION_BEGIN: case COLLECTION_END: case COLLECTION_VALUE: case INVALID_EXPRESSION:
            return type;
    }

    return type;
}

/// \desc Creates a new lexer with default values.
Lexer::Lexer()
{
    fatal = true;

    //setup tracking items.
    definingFunction = false;
    definingFunctionsParameters = false;
    stackVariables              = 0;
    finished                    = false;
    tmpBuffer = nullptr;
    varSpecified = false;
    modifier = TMScriptScope;
    constSpecified = false;
    blockCount = 0;
    isCollectionElement = false;
    definingAndAssigningVariable = false;
    prev = INVALID_TOKEN;
    last = INVALID_TOKEN;

    locationInfo.Reset();

    //Allocate temporary work buffer for lexer calculated expression handler.
    for(int ii=0; ii<ALLOC_BLOCK_SIZE; ++ii)
    {
        tmpValues.push_back(new DslValue());
    }

    tmpBuffer = new U8String();
    if ( tmpBuffer == nullptr )
    {
        PrintIssue(2267,  true, false, "Error allocating memory for temporary work buffer");
        return;
    }

    tmpValue = new DslValue();
    if ( tmpValue == nullptr )
    {
        PrintIssue(2270, true, false,
                   "Error allocating memory for temporary work value");
        return;
    }

    fatal = false;

    AddStandardAssets();
}

/// \desc Destructor removes allocated resources when the Lexer is deleted.
Lexer::~Lexer()
{
    for(int ii=0; ii<tmpValues.Count(); ++ii)
    {
        delete tmpValues.pop_back();
    }

    delete tmpBuffer;
    delete tmpValue;
}

#pragma clang diagnostic pop