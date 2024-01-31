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
    identifier = new U8String();
    modifier = TMScriptScope;
    location = locationInfo;
    readyOnly = false;
    switchStart = 0;
    switchEnd = 0;
    switchCondEnd = 0;
    switchEnd = 0;
    switchCondStart = 0;
    switchIndex = 0;
    switchCaseIndex = 0;
    switchIndex = 0;
    breakLocations.Clear();
}

Token::Token(Token *token)
{
    type = token->type;
    modifier = token->modifier;
    value = new DslValue(token->value);
    if ( value == nullptr )
    {
        PrintIssue(2504, true, false, "Failed to allocate memory for copy of Token value");
        return;
    }
    identifier = new U8String(token->identifier);
    if ( identifier == nullptr )
    {
        PrintIssue(2506, true, false, "Failed to allocate memory for copy of identifier");
        return;
    }
    location = token->location;
    readyOnly = token->readyOnly;
    switchStart = token->switchStart;
    switchEnd = token->switchEnd;
    switchCondEnd = token->switchCondEnd;
    switchEnd = token->switchEnd;
    switchCondStart = token->switchCondStart;
    switchIndex = token->switchIndex;
    switchCaseIndex = token->switchCaseIndex;
    switchIndex = token->switchIndex;
    breakLocations.CopyFrom(&token->breakLocations);
}

Token::Token(DslValue *dslValue)
{
    readyOnly = false;
    value = new DslValue(dslValue);
    type = value->type;
    identifier = new U8String();
    modifier = TMScriptScope;
    location = locationInfo;
    switchStart = 0;
    switchEnd = 0;
    switchCondEnd = 0;
    switchEnd = 0;
    switchCondStart = 0;
    switchIndex = 0;
    switchCaseIndex = 0;
    switchIndex = 0;
    breakLocations.Clear();
}

Token::Token(TokenTypes tokenTypes)
{
    readyOnly = false;
    type = tokenTypes;
    value = new DslValue();
    identifier = new U8String();
    modifier = TMScriptScope;
    location = locationInfo;
    switchStart = 0;
    switchEnd = 0;
    switchCondEnd = 0;
    switchEnd = 0;
    switchCondStart = 0;
    switchIndex = 0;
    switchCaseIndex = 0;
    switchIndex = 0;
    breakLocations.Clear();
}

Token::Token(TokenTypes tokenTypes, U8String *id)
{
    readyOnly = false;
    type = tokenTypes;
    value = new DslValue();
    identifier = new U8String(id);
    modifier = TMScriptScope;
    location = locationInfo;
    switchStart = 0;
    switchEnd = 0;
    switchCondEnd = 0;
    switchEnd = 0;
    switchCondStart = 0;
    switchIndex = 0;
    switchCaseIndex = 0;
    switchIndex = 0;
    breakLocations.Clear();
}