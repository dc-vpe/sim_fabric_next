//
// Created by krw10 on 6/14/2023.
//

#include "../Includes/Token.h"
#include "../Includes/ParseData.h"

/// Frees the memory used by a token.
Token::~Token()
{
    delete identifier;
    delete value;
}

Token::Token()
{
    type = INVALID_TOKEN;
    modifier = TMScriptScope;
    value = new DslValue();
    identifier = nullptr;
    modifier = TMScriptScope;
    location = locationInfo;
    readyOnly = false;
    defaultIndex = -1;
    totalCases   = 0;
    definedParameters = 0;
}

Token::Token(Token *token)
{
    type = token->type;
    modifier = token->modifier;
    value = new DslValue(token->value);
    if ( value == nullptr )
    {
        PrintIssue(2069, "Failed to allocate memory for copy of Token value", true, true);
        return;
    }
    identifier = new U8String(token->identifier);
    if ( identifier == nullptr )
    {
        PrintIssue(2070, "Failed to allocate memory for copy of identifier", true, true);
        return;
    }
    location = token->location;
    readyOnly = token->readyOnly;
    defaultIndex = token->defaultIndex;
    totalCases   = token->totalCases;
    definedParameters = token->definedParameters;
}

Token::Token(DslValue *dslValue)
{
    readyOnly = false;
    value = new DslValue();

    value->type = dslValue->type;
    type = value->type;
    switch(dslValue->type)
    {
        default:
            break;
        case INTEGER_VALUE:
            value->iValue = dslValue->iValue;
            break;
        case DOUBLE_VALUE:
            value->dValue = dslValue->dValue;
            break;
        case CHAR_VALUE:
            value->cValue = dslValue->cValue;
            break;
        case STRING_VALUE:
            value->sValue.CopyFrom(&dslValue->sValue);
            break;
        case BOOL_VALUE:
            value->bValue = dslValue->bValue;
            break;
    }

    identifier = new U8String();
    modifier = TMGlobalScope;
    location = locationInfo;
    defaultIndex = -1;
    totalCases = 0;
    definedParameters = 0;
}

Token::Token(TokenTypes tokenTypes)
{
    readyOnly = false;
    type = tokenTypes;
    value = new DslValue();
    identifier = new U8String();
    modifier = TMScriptScope;
    location = locationInfo;
    defaultIndex = -1;
    totalCases = 0;
    definedParameters = 0;
}

Token::Token(TokenTypes tokenTypes, U8String *id)
{
    readyOnly = false;
    type = tokenTypes;
    value = new DslValue();
    identifier = new U8String(id);
    modifier = TMScriptScope;
    location      = locationInfo;
    defaultIndex = -1;
    totalCases   = 0;
    definedParameters = 0;
}