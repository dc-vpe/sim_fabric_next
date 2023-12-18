//
// Created by krw10 on 6/22/2023.
//
#include "../Includes/KeyWords.h"

/// \desc key word table. This list defines the key words or statements
/// that are defined for the DSL. It is arranged in order from longest to shortest
/// as this allows simplistic matching when lexing the source code.
/// \remark The last entry in the table is a 0 length entry which tells the
/// search code there are no more entries to check.
KeyWord *keyWords[] =
{
    new KeyWord( "continue", CONTINUE),
    new KeyWord( "(double)", CAST_TO_DBL),
    new KeyWord( "(string)", CAST_TO_STR),
    new KeyWord( "default", DEFAULT),
    new KeyWord( "global", GLOBAL),
    new KeyWord( "script", SCRIPT),
    new KeyWord( "return", RETURN),
    new KeyWord( "switch", SWITCH),
    new KeyWord( "(bool)", CAST_TO_BOOL),
    new KeyWord( "(char)", CAST_TO_CHR),
    new KeyWord("(int)", CAST_TO_INT),
    new KeyWord("while", WHILE),
    new KeyWord("const", CONST),
    new KeyWord("break", BREAK),
    new KeyWord("local", LOCAL),
    new KeyWord("block", BLOCK),
    new KeyWord("false", FALSE),
    new KeyWord("true", TRUE),
    new KeyWord("else", ELSE),
    new KeyWord("case", CASE),
    new KeyWord("stop", STOP),
    new KeyWord("var", VAR),
    new KeyWord("for", FOR),
    new KeyWord("brk", BRK),
    new KeyWord("end", LEND),
    new KeyWord("if", IF),
    new KeyWord("", INVALID_TOKEN)
};

KeyWord::KeyWord()
{
    type = INVALID_TOKEN;
    text = new U8String();
    if ( text == nullptr )
    {
        return;
    }
}

KeyWord::~KeyWord()
{
    if ( text != nullptr )
    {
        delete text;
        text = nullptr;
    }
}

KeyWord::KeyWord(const char *word, TokenTypes type)
{
    this->type = type;
    text = new U8String();
    if ( text == nullptr )
    {
        return;
    }

    text->CopyFromCString(word);
}

bool KeyWord::IsEqual(U8String *value)
{
    return text->IsEqual(value);
}

bool KeyWord::IsEqual(TokenTypes tokenTypes)
{
    return this->type == tokenTypes;
}

