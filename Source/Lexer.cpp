#include "../Includes/Lexer.h"
#include <cstdio>
#include <cctype>


//Prevents clang from complaining the source is too complex to perform a static analyze on. Warnings
//and errors are still generated this is informational only.
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

/// \desc scope names maps 1 to 1 to the enum. This is used for display of targeted error messages.
const char *Lexer::szScopeNames[] = {  "invalid scope", "block",  "local","_s", "global" };

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

/// \desc Checks if the next token type is the same as the provided type.
/// \param type Token type to check against the next token type.
/// \return True if the next token type is the same as type, or false if not.
/// \remark This method is more limited that get next token type it is used
///         when looking for variable or numeric token types to determine
///         the type of an operator when more information that just the
///         operator symbol is required.
/// \remark The lex location info is not changed with this check.
/// \remark If the type being checked is a variable value fullVarName
///         is overwritten.
bool Lexer::CheckNextTokenType(TokenTypes type, bool ignoreErrors)
{
    LocationInfo saved = locationInfo;

    u8chr ch = GetNextNonCommentCharacter();

    if ( type == VARIABLE_VALUE )
    {
        if (!IS_ID_START(ch))
        {
            locationInfo = saved;
            return false;
        }
        if (!GetIdentifier())
        {
            locationInfo = saved;
            return false;
        }
        locationInfo = saved;
        return IsVariableDefined(true);
    }

    if (type == INTEGER_VALUE || type == DOUBLE_VALUE )
    {
        if (IsNumber(ch))
        {
            if (GetNumber(ignoreErrors))
            {
                locationInfo = saved;
                return tmpValue->type == type;
            }
        }
    }

    locationInfo = saved;
    return false;
}

/// \desc Gets the Type of the token at the tok location. The
///       location is moved to the next parse location.
TokenTypes Lexer::GetOperatorToken(bool ignoreErrors)
{
    u8chr ch = GetCurrent();
    u8chr ch1;

    ch1 = PeekNextChar();

    locationInfo.Increment(ch);
    if ( locationInfo.location > parseBuffer.Count() )
    {
        return END_OF_SCRIPT;
    }

    LocationInfo saved = locationInfo;

    switch((char)ch)
    {
        case '(':
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
            switch(AddOperatorSubType(ch, ch1, ignoreErrors) )
            {
                case TypeError:
                    return ERROR_TOKEN;
                case TypeUnary:
                    return UNARY_POSITIVE;
                case TypeBinary:
                    return ADDITION;
                case TypePrefix:
                    return PREFIX_INC;
                case TypePostfix:
                    return POSTFIX_INC;
            }
            break;
        case '-':
            if (ch1 == '=')
            {
                locationInfo.Increment(ch1);
                return SUBTRACT_ASSIGNMENT;
            }
            switch(AddOperatorSubType(ch, ch1, ignoreErrors) )
            {
                case TypeError:
                    return ERROR_TOKEN;
                case TypeBinary:
                    return SUBTRACTION;
                case TypeUnary:
                    return UNARY_NEGATIVE;
                case TypePrefix:
                    return PREFIX_DEC;
                case TypePostfix:
                    return POSTFIX_DEC;
            }
            break;
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
                PrintIssue(2000, "NOT operator ! is missing a value to negate", true);
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

/// \desc Checks if the u8String contains a string that follows the rules for identifiers.
/// \param u8String Pointer to a U8String containing the characters to check.
/// \return True if the u8String contains a valid identifier, else false.
bool Lexer::CheckIdentifier(U8String *u8String)
{
    for(int64_t ii=0; ii < u8String->Count(); ++ii)
    {
        if (!IS_IDENTIFIER(u8String->get(ii)))
        {
            return false;
        }
    }

    return true;
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

/// \desc Determines what sub operator Type ch is.
/// \param ch Currently read character from the location in the parse buffer.
/// \param ch1 Next character in the parse buffer, only needed for 2 character operators.
/// \return The operator sub Type which can be binary, unary, prefix, or postfix.
/// TypeError is returned if at the end of the source _f or
OperatorSubType Lexer::AddOperatorSubType(u8chr ch, u8chr ch1, bool ignoreErrors)
{
    //if at end of code _s.
    if ( ch1 == '\0' )
    {
        if ( ch == '+')
        {
            if (!ignoreErrors )
            {
                PrintIssue(2001, "Missing value for addition operation", true);
            }
            return TypeError;
        }
        if ( !ignoreErrors )
        {
            PrintIssue(2002, "Missing value for subtraction operation", true);
        }
        return TypeError;
    }
    if ( ch == ch1)
    {
        TokenTypes prev = GetPreviousTokenType();
        if ( prev == VARIABLE_VALUE )
        {
            locationInfo.Increment(ch);
            return TypePostfix;
        }
        LocationInfo s = locationInfo;
        locationInfo.Increment(ch);
        if ( CheckNextTokenType(VARIABLE_VALUE, ignoreErrors) )
        {
            return TypePrefix;
        }
        locationInfo = s;

        if ( ch == '+' )
        {
            if ( !ignoreErrors )
            {
                PrintIssue(2003, true, false,
                           "Can't increment variable %s as it has not been declared.",
                           tmpBuffer->cStr());
            }
        }
        else
        {
            if ( !ignoreErrors )
            {
                PrintIssue(2004, true, false,
                           "Can't decrement variable %s as it has not been declared.",
                           tmpBuffer->cStr());
            }
        }

        return TypeError;
    }

    //The operator is a single + or - either a binary operation or a unary operation.
    //Determine if operator is a binary or unary expression.

    //If no previous token must be unary
    if ( tokens.Count() == 0 )
    {
        return TypeUnary;
    }
    TokenTypes prev = GetPreviousTokenType();
    if ( !IsValidUnaryPreviousToken(prev) )
    {
        return TypeBinary;
    }

    LocationInfo saved = locationInfo;
    TokenTypes next = GetNextTokenType(true);
    locationInfo = saved;
    if ( next == ERROR_TOKEN )
    {
        return TypeError;
    }
    if (IS_VALID_UNARY_TARGET(next))
    {
        return TypeUnary;
    }
    return TypeBinary;
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
        locationInfo.Increment(ch);
        if (!IS_DIGIT(ch) && ch != '.')
        {
            locationInfo.Decrement();
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
                        PrintIssue(1000, "A number can contain only a single period ignoring", false);
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
/// \param ch character at previous pLocation.
/// \return True if successful, false if an error occurs and processing can't continue.
bool Lexer::ProcessEscapeCharacter(u8chr ch, bool ignoreErrors)
{
    if (locationInfo.location >= parseBuffer.Count())
    {
        if ( !ignoreErrors )
        {
            PrintIssue(2005, "Expected a character after \\ escape character in string value", true);
        }
        return false;
    }
    ch = GetCurrent();
    if (!IsValidStringEscape((char) ch))
    {
        if ( !ignoreErrors )
        {
            PrintIssue(1001, false, false,
                       "The %c character is not a valid escape character", (char)ch);
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
    if (!tmpValue->sValue.push_back(ch))
    {
        fatal = true;
        return false;
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
            PrintIssue(2090,
                       "End of script instead of \"$ end of string definition", true, true);
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
        }
    }

    return true;
}

/// \desc Gets the next character token from the source code for a char '' or string ""
/// \return True if successful, false if an error occurs.
bool Lexer::GetCharacterValue(bool ignoreErrors)
{
    u8chr ch = GetCurrent();

    tmpBuffer->Clear();
    if ( ch != '\\')
    {
        if (!tmpBuffer->push_back(ch))
        {
            fatal = true; //out of memory.
            return false;
        }
    }
    else
    {
        if (!ProcessEscapeCharacter(ch, ignoreErrors))
        {
            return false;
        }
    }

    tmpValue->type = CHAR_VALUE;
    tmpValue->cValue = tmpBuffer->get(0);
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
                    PrintIssue(2006, "Local variables can only be created inside of a function definition", true);
                }
                return false;
            }
            if ( tmpBuffer->IndexOf('.') >= 0 )
            {
                if ( !ignoreErrors )
                {
                    PrintIssue(2007,
                               "Local variables can't contain a period as they only exist inside "
                               "the function in which they are declared.", true);
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
                    PrintIssue(2008,
                               "Script variables can't contain a period as they only exist inside "
                               "the script in which they are declared.", true);
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
                        PrintIssue(2009,
                                   "Global variables can only be defined in the module in which they are created.",
                                   true);
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
        SkipToEndOfBlock();
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
        varSpecified = false;
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

        varSpecified = false;

        if ( !DefineVariableAssignment(token, false))
        {
            return false;
        }

        if ( !variables.Set(token->identifier, token) )
        {
            return false;
        }

        if ( !tokens.push_back(token) )
        {
            return false;
        }

        type = PeekNextTokenType();
        if ( type == COMMA )
        {
            type = PeekNextTokenType();
            if ( type == VAR )
            {
                SkipNextTokenType();
            }
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
        PrintIssue(2010, "Missing semicolon at end of expression", true);
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
                PrintIssue(2087, "Expected a name for the parameter", true);
                return PARAM_ERROR;
            }
            return PARAM_CONTINUE;
        }
        if ( type == CLOSE_PAREN )
        {
            return PARAM_DONE;
        }

        PrintIssue(2011, "Expected a comma or close paren after the parameter name", true);
        return PARAM_ERROR;
    }

    PrintIssue(2012, "Expected a close paren or the parameters name after the functions open paren",
               true);
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
        PrintIssue(2013, "Functions can't be declared inside other functions", true);
        return false;
    }

    TokenTypes type = GetNextTokenType(true);

    if ( type != OPEN_PAREN )
    {
        PrintIssue(2014, "Functions require an open paren after the name of the function", true);
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
        PrintIssue(2015,
                   "Missing open curly brace after close parenthesis in function definition", true);
        locationInfo = start;
        return false;
    }

    if (!SkipToEndOfBlock())
    {
        PrintIssue(2016, "Missing close curly brace in function definition", true);
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
    definingFunction = true;
    varSpecified = false;

    auto *funBegin = new Token(token);
    funBegin->type = FUNCTION_DEF_BEGIN;
    tokens.push_back(funBegin);

    if ( !functions.Set(funBegin->identifier, funBegin) )
    {
        SkipToEndOfBlock();
        definingFunction = false;
        return false;
    }

    if ( !DefineFunctionParameters())
    {
        SkipToEndOfBlock();
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
        PrintIssue(2017, "Missing open curly brace in function definition", true);
        SkipToEndOfBlock();
        definingFunction = false;
        return false;
    }

    LocationInfo start = locationInfo;
    if (!SkipToEndOfBlock())
    {
        PrintIssue(2018, "Missing close curly brace in function definition", true);
        SkipToEndOfBlock();
        definingFunction = false;
        return false;
    }

    LocationInfo end = locationInfo;
    locationInfo = start;
    //Skip open curly brace
    SkipNextTokenType();
    start = locationInfo;
    if ( !DefineStatements(end) )
    {
        SkipToEndOfBlock();
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
            SkipToEndOfBlock();
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
            PrintIssue(2019, "Invalid identifier specified for function parameter", true);
            definingFunctionsParameters = false;
            SkipToEndOfBlock();
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
                    PrintIssue(2076, "Divide by zero is undefined", true);
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
                    PrintIssue(2076, "Modulo by zero is undefined", true);
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
                PrintIssue(2079, "Out of memory extending temporary work values buffer", true, true);
                return false;
            }
        }
    }
    values.push_back(tmpValues[top++]);
    return true;
}

/// \desc Scans an expression within a collection definition time and either assigns the result
///       to the token or generates an error if the expression can't be evaluated at lex time.
/// \param dslValue Pointer to the dsl value to be updated with the result of the calculation.
/// \param ignoreErrors If true then normal error processing occurs, else error generation except
///                      for fatal errors are ignored.
/// \param initializeVariable If true a collection or variable is being initialized with an expression
///                           else false.
bool Lexer::ProcessStaticExpression(DslValue *dslValue, bool ignoreErrors, bool initializeVariable)
{
    LocationInfo start;
    LocationInfo end;
    start = locationInfo;
    TokenTypes type = INVALID_TOKEN;
    while( type != END_OF_SCRIPT )
    {
        type = GetNextTokenType(ignoreErrors);
        if ( type == COMMA || type == CLOSE_BLOCK || type == SEMICOLON )
        {
            break;
        }
    }
    end = locationInfo;
    locationInfo = start;

    return ProcessStaticExpression(dslValue, end, ignoreErrors, initializeVariable);
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

    TokenTypes prev = INVALID_TOKEN;
    TokenTypes type = INVALID_TOKEN;
    while( locationInfo.location < end.location)
    {
        prev = type;
        type = GetNextTokenType(ignoreErrors);
        if ( type == UNARY_NEGATIVE || type == UNARY_POSITIVE )
        {
            if (!IsValidUnaryPreviousToken(prev))
            {
                if (type == UNARY_POSITIVE )
                {
                    type = ADDITION;
                }
                else
                {
                    type == SUBTRACTION;
                }
            }
        }
        if ( type == ERROR_TOKEN )
        {
            return false;
        }
        switch( type )
        {
            case END_OF_SCRIPT:
                if ( ignoreErrors )
                {
                    PrintIssue(2081, "Missing close curly brace in collection definition", true);
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
                    PrintIssue(2078,"Expressions used to initialize a variable "
                                    "can only contain values, operators, and collections", true);
                }
                break;
        }
    }
    while ( ops.top() != 0 )
    {
        if ( !ProcessOperation(values, ops.pop_back(), ignoreErrors) )
        {
            return false;
        }
    }

    dslValue->LiteCopy(new DslValue(values.pop_back()));

    return true;
}

/// \desc Defines the statements that are assigned to a variable when it is being defined.
/// \param token Token that contains the variable information the assignment is being applied to.
/// \param isInnerCollection True if this is being called recursively while parsing an inner collection definition.
/// \return True if successful, false if an error occurs.
bool Lexer::DefineVariableAssignment(Token *token, bool isInnerCollection)
{
    LocationInfo start = locationInfo;

    TokenTypes type = INVALID_TOKEN;
    if ( !isInnerCollection )
    {
        type = GetNextTokenType(false);
        if ( type == ERROR_TOKEN )
        {
            return false;
        }
    }

    DslValue dslValue = {};

    //If not a collection, process expression and assign to single variable.
    if ( PeekNextTokenType() != OPEN_BLOCK )
    {
        if ( !ProcessStaticExpression(&dslValue, false, true) )
        {
            return false;
        }
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
        return token;
    }

    token->value->type = COLLECTION;
    token->modifier = modifier;
    U8String key = {};
    int64_t index = 0;
    int64_t open_blocks = locationInfo.openBlocks;
    int64_t collectionEnd;

    TokenTypes prev = type;
    type = GetNextTokenType(false);
    while( locationInfo.Blocks() )
    {
        while( type != CLOSE_BLOCK )
        {
            prev = type;
            LocationInfo save = locationInfo;
            type = GetNextTokenType(true);
            if ( type == ERROR_TOKEN )
            {
                return false;
            }
            u8chr ch = GetNextNonCommentCharacter();
            if (ch == ':')
            {
                if ( type != STRING_VALUE || !CheckIdentifier(&tmpValue->sValue) )
                {
                    PrintIssue(2080, "Invalid key names must follow the same rules as numbers or identifiers",
                               true);
                    return false;
                }
                locationInfo.Increment(ch);
                key.CopyFrom(&tmpValue->sValue);
                prev = type;
                save = locationInfo;
                type = GetNextTokenType(false);
            }
            else
            {
                key.Clear();
                key.push_back(&token->value->variableScriptName);
                key.push_back('.');
                key.Append(index);
            }

            switch (type)
            {
                case END_OF_SCRIPT:
                    PrintIssue(2072, "Missing close curly brace in collection definition", true);
                case INVALID_TOKEN:
                case ERROR_TOKEN:
                    PrintIssue(2075, "Error in collection definition format", true);
                    return false;
                case VAR: case CONST: case GLOBAL: case SCRIPT: case LOCAL: case BLOCK: case VARIABLE_DEF:
                    PrintIssue(2073, "Variables can't be defined inside a collection", true);
                    return false;
                case COMMA:
                    if (prev == COMMA)
                    {
                        PrintIssue(2074, "Missing variable, value, or expression between commas", true);
                        return false;
                    }
                    index++;
                    break;
                case CLOSE_BLOCK:
                    if ( locationInfo.Blocks() != 0 )
                    {
                        return true;
                    }
                    break;
                case OPEN_BLOCK:
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
                    //Back up to where open block started so that collection can be processed correctly.
                    locationInfo = save;
                    if ( !DefineVariableAssignment(inner, true))
                    {
                        return false;
                    }
                    if ( !variables.Set(inner->identifier, inner) )
                    {
                        return false;
                    }

                    if ( !tokens.push_back(inner) )
                    {
                        return false;
                    }
                    token->value->indexes.Set(new U8String(&key), new DslValue(inner->value));
                    break;
                }
                case VARIABLE_VALUE:
                {
                    auto *t = new Token(VARIABLE_VALUE, tmpBuffer);
                    GetFullName(t->identifier, modifier);
                    token->modifier = modifier;
                    t->value->variableName.CopyFrom(t->identifier);
                    auto *v = variables.Get(&t->value->variableName);

                    token->value->indexes.Set(new U8String(&key), new DslValue(v->value));
                    break;
                }
                case COLLECTION:
                case COLLECTION_VALUE:
                case REFERENCE:
                    break;
                case PREFIX_INC: case PREFIX_DEC: case POSTFIX_INC: case POSTFIX_DEC:
                case ASSIGNMENT: case MULTIPLY_ASSIGNMENT: case DIVIDE_ASSIGNMENT:
                case MODULO_ASSIGNMENT: case ADD_ASSIGNMENT: case SUBTRACT_ASSIGNMENT:
                case SEMICOLON:
                case IF: case ELSE: case SWITCH: case CASE: case DEFAULT: case WHILE: case FOR: case BREAK:
                case CONTINUE: case RETURN: case LEND: case IF_COND_BEGIN: case IF_COND_END: case IF_BLOCK_BEGIN:
                case IF_BLOCK_END: case ELSE_BLOCK_BEGIN: case ELSE_BLOCK_END: case WHILE_COND_BEGIN:
                case WHILE_COND_END: case WHILE_BLOCK_BEGIN: case WHILE_BLOCK_END: case FOR_INIT_BEGIN:
                case FOR_INIT_END: case FOR_COND_BEGIN: case FOR_COND_END: case FOR_UPDATE_BEGIN:
                case FOR_UPDATE_END: case FOR_BLOCK_BEGIN:  case FOR_BLOCK_END: case SWITCH_COND_BEGIN:
                case SWITCH_COND_END: case SWITCH_BEGIN: case SWITCH_END: case CASE_COND_BEGIN:
                case CASE_COND_END: case CASE_BLOCK_BEGIN: case CASE_BLOCK_END: case DEFAULT_BLOCK_BEGIN:
                case DEFAULT_BLOCK_END: case FUNCTION_DEF_BEGIN: case FUNCTION_DEF_END: case FUNCTION_PARAMETER:
                case FUNCTION_CALL_BEGIN: case PARAM_BEGIN: case PARAM_END: case FUNCTION_CALL_END:
                case COLLECTION_BEGIN: case COLLECTION_END:
                    PrintIssue(2077, true, false,
                               "Collection definitions can't contain statements.");
                    break;
                case OPEN_PAREN: case CLOSE_PAREN: case OPEN_BRACE: case CLOSE_BRACE:
                case INTEGER_VALUE: case STRING_VALUE: case DOUBLE_VALUE: case CHAR_VALUE: case BOOL_VALUE:
                case FALSE: case TRUE:
                case UNARY_POSITIVE: case UNARY_NEGATIVE: case UNARY_NOT:
                case CAST_TO_INT: case CAST_TO_DBL: case CAST_TO_CHR: case CAST_TO_STR: case CAST_TO_BOOL:
                {
                    if (token->value->indexes.Exists(&key))
                    {
                        PrintIssue(2082, true, false,
                                   "Key %s already exists in the collection",
                                   key.cStr());
                        return false;
                    }
                    LocationInfo next = locationInfo;
                    locationInfo = save;
                    if (!ProcessStaticExpression(&dslValue, false, true))
                    {
                        return false;
                    }
                    locationInfo = next;
                    token->value->indexes.Set(new U8String(&key), new DslValue(&dslValue));
                    //Expect a comma, string, or close curly brace
                    TokenTypes tt = PeekNextTokenType();
                    if ( tt != COMMA && tt != STRING_VALUE && tt != CLOSE_BLOCK )
                    {
                        PrintIssue(2055,
                                   "Missing comma, string key value, or close curly brace in collection definition",
                                   true);
                        return false;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    type = GetNextTokenType(true);
    if ( type != SEMICOLON )
    {
        PrintIssue(2020,
                   "Error in variable definition's assignment expression missing a comma or semicolon",
                   true);
        return false;
    }

    modifier = TMScriptScope;
    varSpecified = false;
    constSpecified = false;

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
        PrintIssue(2020, "a ( must follow the while statement", true);
        locationInfo = save;
        return false;
    }

    TokenTypes prev;
    while( type != END_OF_SCRIPT && type != OPEN_BLOCK )
    {
        prev = type;
        type = GetNextTokenType(false);
    }

    if ( type == END_OF_SCRIPT || (prev != CLOSE_PAREN) )
    {
        PrintIssue(2021, "Missing close paren before start of the statement block", true, false);
        locationInfo = save;
        return false;
    }

    if ( type != OPEN_BLOCK )
    {
        PrintIssue(2022, "Missing open curly brace after while conditional", true, false);
        locationInfo = save;
        return false;
    }

    locationInfo = save;
    if (!SkipToEndOfBlock() )
    {
        PrintIssue(2023, "Missing close curly brace after while conditional", true, false);
        locationInfo = save;
        return false;
    }

    locationInfo = save;
    return true;
}

/// \desc Translates the text for the while code block into the the tokens used by the parser.
bool Lexer::DefineWhile()
{
    LocationInfo whileCondition = locationInfo;

    TokenTypes type = INVALID_TOKEN;

    while( PeekNextTokenType() != OPEN_BLOCK )
    {
        type = GetNextTokenType(false);
    }

    if (!DefineStatementBlock(WHILE_BLOCK_BEGIN, WHILE_BLOCK_END))
    {
        return false;
    }

    LocationInfo whileEnd = locationInfo;

    locationInfo = whileCondition;
    tokens.push_back(new Token(WHILE_COND_BEGIN));

    //Skip open paren as define condition assumes it is present.
    SkipNextTokenType();

    //Generate the conditional statements for the IF statement
    if ( !DefineCond("The ) is missing, the conditional part of the while must be enclosed in ()"))
    {
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
    TokenTypes type = GetNextTokenType(false);
    if ( type != OPEN_PAREN )
    {
        PrintIssue(2024, "a ( must follow the If statement", true);
        return false;
    }
    tokens.push_back(new Token(IF_COND_BEGIN));

    //Generate the conditional statements for the IF statement
    if ( !DefineCond("The ) is missing, the conditional part of an if must be enclosed in ()"))
    {
        return false;
    }

    tokens.push_back(new Token(IF_COND_END));

    if ( PeekNextTokenType() != OPEN_BLOCK )
    {
        PrintIssue(2025, "Missing open curly brace after if loop condition", true);
        return false;
    }

    if (!DefineStatementBlock(IF_BLOCK_BEGIN, IF_BLOCK_END))
    {
        return false;
    }

    //Check for else
    if ( PeekNextTokenType() == ELSE )
    {
        //Skip the else
        SkipNextTokenType();
        //If this is a chained ELSE IF
        if ( PeekNextTokenType() == IF )
        {
            //Skip if so the parse can begin on the condition.
            SkipNextTokenType();

            return DefineIf();
        }

        if ( PeekNextTokenType() != OPEN_BLOCK )
        {
            PrintIssue(2026, "Missing open curly brace after else condition", true);
            return false;
        }

        //else define the else block
        if (!DefineStatementBlock(ELSE_BLOCK_BEGIN, ELSE_BLOCK_END))
        {
            return false;
        }
        if ( PeekNextTokenType() == ELSE )
        {
            PrintIssue(2027, "Else without a matching if", true);
            return false;
        }
    }

    return true;
}

/// \desc Generates the statements for a conditional expression
/// \param errorMsg Error message to display if an error occurs, all tied to error code 2063.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineCond(const char *errorMsg)
{
    int64_t parens = 1;
    while( parens > 0 )
    {
        TokenTypes type = GetNextTokenType(true);
        if ( type == END_OF_SCRIPT )
        {
            PrintIssue(2028, errorMsg, true);
            return false;
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
                break;
            }
        }
        type = GenerateTokens(type);
        if ( type == ERROR_TOKEN )
        {
            return false;
        }
    }

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
bool Lexer::DefineStatementBlock(TokenTypes blockStart, TokenTypes blockEnd)
{
    //Skip the close paren so block parsing works correctly.
    if ( PeekNextTokenType() == CLOSE_PAREN )
    {
        SkipNextTokenType();
    }

    LocationInfo saved = locationInfo;
    //Find the end of the block
    if ( !SkipToEndOfBlock() )
    {
        PrintIssue(2029, "Too many open curly braces in statement block.", true);
        return false;
    }
    LocationInfo end = locationInfo;

    locationInfo = saved;
    //Skip open block
    SkipNextTokenType();
    saved = locationInfo;

    tokens.push_back(new Token(blockStart));

    if ( !DefineStatements(end) )
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

/// \desc Determines where the end of a default or case block is.
/// \param switchEnd Location of the end of the switch statement.
void Lexer::GetEndOfCaseBlock(LocationInfo switchEnd)
{
    LocationInfo saved = locationInfo;
    TokenTypes last = INVALID_TOKEN;
    while( locationInfo.location < switchEnd.location )
    {
        TokenTypes next = PeekNextTokenType();
        if ( next == CASE || next == DEFAULT )
        {
            break;
        }
        saved = locationInfo;
        last = GetNextTokenType(true);
    }
    if ( last == CLOSE_BLOCK )
    {
        locationInfo = saved;
    }
}

/// \desc Processes a block of statements and adds the tokens for those statements to the tokens list.
/// \param end Location of the end of block marker.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineStatements(LocationInfo end)
{
    while( locationInfo.location < end.location )
    {
        TokenTypes type = GetNextTokenType(false);
        if ( type == ERROR_TOKEN )
        {
            return false;
        }
        //Check for empty statement block
        if ( type == CLOSE_BLOCK && locationInfo.Blocks() == 0 )
        {
            break;
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
    TokenTypes type = GetNextTokenType(true);
    if ( type != OPEN_PAREN )
    {
        PrintIssue(2030, "An open parenthesis must follow the for keyword", true);
        return false;
    }

    int64_t defaultCase = -1;
    auto *switchToken = new Token(SWITCH_BEGIN);
    int64_t switchStartIndex = tokens.Count();

    tokens.push_back(switchToken);

    //Generate the conditional for the switch statement
    tokens.push_back(new Token(SWITCH_COND_BEGIN));
    if ( !DefineCond("The ) is missing, switch condition must be enclosed in ()"))
    {
        return false;
    }

    tokens.push_back(new Token(SWITCH_COND_END));

    if ( PeekNextTokenType() != OPEN_BLOCK )
    {
        PrintIssue(2031, "Missing open curly brace after switch condition", true);
        return false;
    }

    LocationInfo saved = locationInfo;
    if ( !SkipToEndOfBlock() )
    {
        PrintIssue(2032, "Missing close curly brace at end of statement", true);
        return false;
    }


    LocationInfo switchEnd = locationInfo;
    locationInfo = saved;

    //Skip the open block.
    SkipNextTokenType();

    while(locationInfo.location < switchEnd.location )
    {
        type = GetNextTokenType(true);
        if ( type == OPEN_BLOCK || type == CLOSE_BLOCK )
        {
            continue;
        }
        if ( type == DEFAULT )
        {
            switchToken->defaultIndex = tokens.Count();
            if ( !DefineDefault(switchEnd) )
            {
                locationInfo = saved;
                return false;
            }
        }
        else if ( type == CASE )
        {
            if (!DefineCase(switchEnd))
            {
                locationInfo = saved;
                return false;
            }
            ++switchToken->totalCases;
        }
        else
        {
            PrintIssue(2033,
                       "Missing the case key word in switch statement",
                       true);
            locationInfo = saved;
            return false;
        }
    }

    //Skip the close block.
    SkipNextTokenType();

    tokens.push_back(new Token(SWITCH_END));
    return true;
}

/// \desc Processes the statements that are part of a switch case.
/// \param switchEnd Location of the end of the switch statement.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineCase(LocationInfo switchEnd)
{
    TokenTypes type;

    DslValue     dslValue;
    LocationInfo end;
    LocationInfo start = locationInfo;

    u8chr ch = GetCurrent();
    while( ch != END_OF_SCRIPT && ch != ':' )
    {
        ch = GetNextNonCommentCharacter();
        if ( ch == ':' )
        {
            break;
        }
        locationInfo.Increment(ch);
    }
    if ( ch == END_OF_SCRIPT )
    {
        PrintIssue(2085, "A colon must follow the value used in a case keyword", true);
    }
    end = locationInfo;
    locationInfo = start;

    if ( !ProcessStaticExpression(&dslValue, end, false, true) )
    {
        return false;
    }

    tokens.push_back(new Token(CASE_COND_BEGIN));
    tokens.push_back(new Token(&dslValue));
    tokens.push_back(new Token(CASE_COND_END));

    //Skip the case : statement terminator.
    locationInfo.Increment(ch);

    if ( PeekNextTokenType() == OPEN_BLOCK )
    {
        //Starting curly braces are optional in case and default statements.
        SkipNextTokenType();
    }

    tokens.push_back(new Token(CASE_BLOCK_BEGIN));
    LocationInfo saved = locationInfo;
    GetEndOfCaseBlock(switchEnd);
    end = locationInfo;
    locationInfo = saved;

    if ( !DefineStatements(end) )
    {
        return false;
    }

    if ( PeekNextTokenType() == CLOSE_BLOCK )
    {
        //Skip the close curly brace.
        SkipNextTokenType();
    }

    tokens.push_back(new Token(CASE_BLOCK_END));
    return true;
}

/// \desc Processes the condition and the block of statements for a default statement.
/// \return True if successful or false if an error occurs.
bool Lexer::DefineDefault(LocationInfo switchEnd)
{
    u8chr ch = GetNextNonCommentCharacter();
    if ( ch != ':' )
    {
        PrintIssue(2036, "A colon must follow the default keyword", true);
        return false;
    }
    locationInfo.Increment(ch);

    tokens.push_back(new Token(DEFAULT_BLOCK_BEGIN));

    LocationInfo save = locationInfo;
    GetEndOfCaseBlock(switchEnd);
    LocationInfo end = locationInfo;
    locationInfo = save;

    if ( !DefineStatements(end) )
    {
        return false;
    }

    if ( PeekNextTokenType() == CLOSE_BLOCK )
    {
        SkipNextTokenType();
    }

    tokens.push_back(new Token(DEFAULT_BLOCK_END));

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
        PrintIssue(2037, "An open parenthesis must follow the for keyword", true);
        locationInfo = save;
        SkipToEndOfBlock();
        return false;
    }

    init = locationInfo;
    if ( !SkipToCharacterBeforeCharacter(';', ')') )
    {
        PrintIssue(2038, "No semicolon between the initialization and conditional sections", true);
        locationInfo = save;
        SkipToEndOfBlock();
        return false;
    }

    SkipNextTokenType();
    cond = locationInfo;
    if (!SkipToCharacterBeforeCharacter(';', ')'))
    {
        PrintIssue(2039, "No semicolon between the conditional and update sections", true);
        locationInfo = save;
        SkipToEndOfBlock();
        return false;
    }

    SkipNextTokenType();
    update = locationInfo;
    if (!SkipToCharacterBeforeCharacter(')', '{'))
    {
        PrintIssue(2040, "No close paren before the open block and after the section", true);
        locationInfo = save;
        SkipToEndOfBlock();
        return false;
    }

    block = locationInfo;
    //Skip the close paren.
    SkipNextTokenType();
    type = GetNextTokenType(false);
    if ( type != OPEN_BLOCK )
    {
        PrintIssue(2041, "Missing open block and after the close paren", true);
        locationInfo = save;
        SkipToEndOfBlock();
        return false;
    }

    locationInfo = save;
    SkipToEndOfBlock();
    SkipNextTokenType(); //Skip the close block as skip to end of block ends before the close block by design.
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
        rc = DefineStatements(end);
        tokens.push_back(new Token(endToken));
    }
    else
    {
        //Skip the close paren.
        rc = DefineStatementBlock(FOR_BLOCK_BEGIN, FOR_BLOCK_END);
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

    if ( !CheckForSyntax(init, cond, update, block, end))
    {
        return false;
    }

    if ( !DefineForSection(FOR_INIT_BEGIN, FOR_INIT_END, init, cond) )
    {
        return false;
    }

    if ( !DefineForSection(FOR_COND_BEGIN, FOR_COND_END, cond, update))
    {
        return false;
    }

    if ( !DefineForSection(FOR_BLOCK_BEGIN, FOR_BLOCK_END, block, end))
    {
        return false;
    }

    if ( !DefineForSection(FOR_UPDATE_BEGIN, FOR_UPDATE_END, update, block))
    {
        return false;
    }

    locationInfo = end;
    return true;
}

/// \desc Skips to the end of a code block.
bool Lexer::SkipToEndOfBlock()
{
    int64_t curleyBraces = 0;

    u8chr ch = INVALID_UTF_CHAR;
    while( ch != '{' )
    {
        ch = GetNextNonCommentCharacter();
        if ( ch == U8_NULL_CHR )
        {
            return false;
        }
        locationInfo.Increment(ch);
    }
    curleyBraces = 1;
    ch = GetCurrent();
    if ( ch != '}' )
    {
        locationInfo.Increment(ch);
    }

    while(ch != U8_NULL_CHR )
    {
        LocationInfo saved = locationInfo;
        ch = GetNextNonCommentCharacter();
        if ( ch == U8_NULL_CHR )
        {
            break;
        }
        if ( ch == '{' )
        {
            ++curleyBraces;
        }
        if ( ch == '}' )
        {
            --curleyBraces;
            if ( curleyBraces == 0 )
            {
                locationInfo = saved;
                return true;
            }
        }
        locationInfo.Increment(ch);
    }

    return false;
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
    //If accessing a field in a collection.
    if ( PeekNextTokenType() == OPEN_BRACE )
    {
        if ( strict && token->value->type != COLLECTION )
        {
            PrintIssue(2083, true, false,
                       "Variable %s is not a collection",
                       fullVarName.cStr());
            delete token;
            return false;
        }
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
            PrintIssue(2079, "Out of memory allocating token for collection index",
                       true, true);
            return false;
        }
        token->value->opcode = PCV;
    }
    //Assignments need the address of the variable not the value.
    if (IS_ASSIGNMENT_TOKEN(PeekNextTokenType()))
    {
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
        PrintIssue(2042, "Expected an open paren after the name of the function call", true);
        locationInfo = start;
        return false;
    }

    while( locationInfo.Parens() > 0 && type != END_OF_SCRIPT )
    {
        type = GetNextTokenType(true);
    }

    if ( type == END_OF_SCRIPT )
    {
        PrintIssue(2043, "Too many open parenthesis in function call", true);
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
    PrintIssue(2044,
               "Function call does not end with a semicolon, or an operator that would allow the "
               "expression to be evaluated.", true);

    locationInfo = start;
    return false;
}

/// \desc Counts how many parameters there are in a function call. Expects the
///       location to be at the open paren of the function call.
/// \return Number of parameters sent to the function.
int64_t Lexer::CountFunctionArguments()
{
    int64_t arguments = 0;
    TokenTypes type = INVALID_TOKEN;
    LocationInfo saved = locationInfo;
    int64_t parens = 0;
    bool empty = true;
    bool done = false;

    U8String tmpFullFunctionName;

    tmpFullFunctionName.CopyFrom(&fullFunName);

    while( !done )
    {
        type = GetNextTokenType(true);
        switch( type )
        {
            case FUNCTION_CALL:
                //Skip inner function call.
                {
                    int64_t p = 1;
                    SkipNextTokenType();
                    empty = false;
                    while( p > 0 )
                    {
                        type = GetNextTokenType(true);
                        switch(type)
                        {
                            case OPEN_PAREN:
                                ++p;
                                break;
                            case CLOSE_PAREN:
                                --p;
                                break;
                            case END_OF_SCRIPT:
                                PrintIssue(2089, "Missing close paren in function call", true, false);
                                return -1;
                            default:
                                break;
                        }
                    }
                    break;
                }
                break;
            default:
                empty = false;
                break;
            case OPEN_BRACE:
                {
                    int64_t braces = 1;
                    while( braces > 0 )
                    {
                        type = GetNextTokenType(true);
                        switch(type)
                        {
                            case OPEN_BRACE:
                                ++braces;
                                break;
                            case CLOSE_BRACE:
                                --braces;
                                break;
                            case END_OF_SCRIPT:
                                PrintIssue(2047, "Missing close brace in function call", true, false);
                                return -1;
                            default:
                                break;
                        }
                    }
                    break;
                }
            case INVALID_TOKEN:
                PrintIssue(2046, true, false,
                           "Error in the %lld parameter in function %s", arguments, currentFunction.cStr());
            case END_OF_SCRIPT:
                arguments = -1;
                break;
            case COMMA:
                ++arguments;
                break;
            case OPEN_PAREN:
                ++parens;
                break;
            case CLOSE_PAREN:
                --parens;
                if ( parens == 0 )
                {
                    done = true;
                }
                break;
        }
    }

    if ( !empty )
    {
        ++arguments;
    }

    if ( type == END_OF_SCRIPT )
    {
        PrintIssue(2043, "Too many open parenthesis in function call", true);
    }
    else
    {
        locationInfo = saved;
    }

    fullFunName.CopyFrom(&tmpFullFunctionName);

    return arguments;
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
            PrintIssue(2088, true, false,
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
    locationInfo.Increment(ch);
    ch = GetCurrent();
    if ( !tmpBuffer->push_back(ch) )
    {
        locationInfo.Decrement();
        return false; //fatal error condition out of memory.
    }

    while(locationInfo.location < parseBuffer.Count())
    {
        locationInfo.Increment(ch);
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
        locationInfo.Increment(ch);
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

    PrintIssue(2048, "Closing */ not found before end of _s.", true);
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

/// \desc Gets the previous token Type.
/// \return The previous token or INVALID_TOKEN if there is no previous token.
TokenTypes Lexer::GetPreviousTokenType()
{
    if (tokens.Count() > 0 )
    {
        Token *token = tokens[tokens.Count()-1];
        if ( token != nullptr )
        {
            return token->type;
        }
    }
    return INVALID_TOKEN;
}

/// \desc Gets the next token to be read without moving the lexing location.
/// \return The next token type to be read.
TokenTypes Lexer::PeekNextTokenType()
{
    LocationInfo saved = locationInfo;
    auto *tmp = new U8String(tmpBuffer);
    TokenTypes type = GetNextTokenType(true);
    tmpBuffer->CopyFrom(tmp);
    delete tmp;
    locationInfo = saved;

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
void Lexer::SkipNextTokenType()
{
    (void) GetNextTokenType(true);
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
                TokenTypes prev = GetPreviousTokenType();

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
TokenTypes Lexer::GetNextTokenType(bool ignoreErrors)
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
        return CHAR_VALUE; //Attempt to continue
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
        return INVALID_TOKEN; //attempt to continue
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
            PrintIssue(2049,
                       "Unexpected character, expected a key word,"
                       "variable, function, value or operator.",
                       true);
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
        tmpValue->bValue = (type == TRUE) ? true : false;
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
            PrintIssue(2051, "Expected a semi-colon after break", true);
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
            if ( !ignoreErrors )
            {
                PrintIssue(2050, true, false,
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
                    PrintIssue(2053, true, false,
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
                PrintIssue(2054, true, false,
                           "Variable %s has already been defined", tmpBuffer->cStr());
                return ERROR_TOKEN;
            }
        }

        if ( !ignoreErrors )
        {
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
        PrintIssue(2075, "Variable modifiers can only be specified at variable creation", true);
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
                PrintIssue(2052, true, false, "Parameter %s has already been defined",
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
        PrintIssue(2045, true, false, "Unknown identifier %s", tmpBuffer->cStr());
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
                            PrintIssue(2058, true, false,
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

    if ( id < 1 || id > modules.Count() )
    {
        PrintIssue(2059, "Module does not exist. Modules are added with AddModule", true, true);
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
        if (type == END_OF_SCRIPT)
        {
            finished = true;
            continue;
        }

        type = GenerateTokens(type);
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
        case LEND:
        case FUNCTION_CALL_BEGIN:
        case PARAM_BEGIN:
        case PARAM_END:
        case INVALID_TOKEN:
        case KEY_BEGIN:
        case KEY_END:
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
                PrintIssue(2060, "Close parenthesis without a matching open parenthesis", true);
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
        case PREFIX_INC: case PREFIX_DEC:
        {
            if ( !AddVariableValue() )
            {
                return ERROR_TOKEN;
            }

            //Skip variable as its already been assigned
            SkipNextTokenType();
        }
        case POSTFIX_INC: case POSTFIX_DEC:
        {
            //postfix needs variable to increment or decrement.
            Token *prev = tokens[tokens.Count()-1];
            auto *t = new Token(prev);
            t->type = type;
            tokens.push_back(t);
            return type;
        }
        case ASSIGNMENT: case MULTIPLY_ASSIGNMENT: case DIVIDE_ASSIGNMENT: case MODULO_ASSIGNMENT: case ADD_ASSIGNMENT:
        case SUBTRACT_ASSIGNMENT:
        {
            if (definingFunctionsParameters)
            {
                PrintIssue(2061, "Function parameters may not be assigned values", true);
                return ERROR_TOKEN;
            }
            if ( tokens.Count() == 0 )
            {
                PrintIssue(2062, "Assignment attempted without a variable", true);
                return ERROR_TOKEN;
            }
            Token *prev = tokens[tokens.Count()-1];
            if ( prev->readyOnly )
            {
                PrintIssue(2086, true, false,
                           "%s can't be assigned to as it is read only",
                           prev->value->variableName.cStr());
                return ERROR_TOKEN;
            }
            if ( !IS_ASSIGN_TYPE(prev->type) && prev->type != COLLECTION_VALUE )
            {
                PrintIssue(2063, "Assignment attempted to something other than a variable", true);
                return ERROR_TOKEN;
            }
            auto *tokenAssign = new Token(prev);
            tokenAssign->type = type;
            tokenAssign->value->variableName.CopyFrom(prev->identifier);
            tokens.push_back(tokenAssign);
            return type;
        }
        case OPEN_BLOCK: //openBlocks can be functions, if, else, elseif, switch, case, and collections.
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
        case CLOSE_BLOCK: //openBlocks can be functions, if, else, elseif, switch, case, and collections.
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
            if (GetPreviousTokenType() == OPEN_BLOCK )
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
                SkipToEndOfBlock();
                return ERROR_TOKEN;
            }
            if ( !DefineWhile() )
            {
                SkipToEndOfBlock();
                return ERROR_TOKEN;
            }
            return type;
        case IF:
            if ( !DefineIf() )
            {
                SkipToEndOfBlock();
                return ERROR_TOKEN;
            }
            return type;
        case FOR:
            if ( !DefineFor() )
            {
                SkipToEndOfBlock();
                return ERROR_TOKEN;
            }
            return type;
        case CASE:
            PrintIssue(2064, "The case key word can only be used inside a switch statement", true);
            return ERROR_TOKEN;
        case DEFAULT:
            PrintIssue(2065, "Default must be used inside a switch statement", true);
            return type;
        case SWITCH:
            if ( !DefineSwitch() )
            {
                SkipToEndOfBlock();
                return ERROR_TOKEN;
            }
            return type;
        case RETURN:
            if ( !definingFunction )
            {
                PrintIssue(2056, "Return statement can only be used inside a function", true);
                return ERROR_TOKEN;
            }
            if ( PeekNextTokenType() == SEMICOLON )
            {
                PrintIssue(2057, "Return statement requires a value to return", true);
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
            PrintIssue(2066, "Functions cannot be defined within functions.", true);
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
        case FALSE: case TRUE:
        case PARAMETER_VALUE: case BRK: case FUNCTION_DEF_END: case FUNCTION_PARAMETER:
        case BLOCK: case ELSE: case IF_COND_BEGIN: case IF_COND_END: case IF_BLOCK_BEGIN: case IF_BLOCK_END:
        case ELSE_BLOCK_BEGIN: case ELSE_BLOCK_END: case WHILE_COND_BEGIN: case WHILE_COND_END: case WHILE_BLOCK_BEGIN:
        case WHILE_BLOCK_END: case FOR_INIT_BEGIN: case FOR_INIT_END: case FOR_COND_BEGIN: case FOR_COND_END:
        case FOR_UPDATE_BEGIN: case FOR_UPDATE_END: case FOR_BLOCK_BEGIN: case FOR_BLOCK_END: case SWITCH_COND_BEGIN:
        case SWITCH_COND_END: case SWITCH_BEGIN: case SWITCH_END: case CASE_COND_BEGIN: case CASE_COND_END:
        case CASE_BLOCK_BEGIN: case CASE_BLOCK_END: case DEFAULT_BLOCK_BEGIN: case DEFAULT_BLOCK_END: case COLLECTION:
        case REFERENCE: case COLLECTION_BEGIN: case COLLECTION_END: case COLLECTION_VALUE:
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

    locationInfo.Reset();

    //Allocate temporary work buffer for lexer calculated expression handler.
    for(int ii=0; ii<ALLOC_BLOCK_SIZE; ++ii)
    {
        tmpValues.push_back(new DslValue());
    }

    tmpBuffer = new U8String();
    if ( tmpBuffer == nullptr )
    {
        PrintIssue(2067, "Error allocating memory for temporary work buffer", true, true);
        return;
    }

    tmpValue = new DslValue();
    if ( tmpValue == nullptr )
    {
        PrintIssue(2068, "Error allocating memory for temporary work value", true, true);
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