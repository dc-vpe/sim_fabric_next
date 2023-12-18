//
// Created by krw10 on 6/22/2023.
//

#ifndef DSL_RESERVED_WORDS_H
#define DSL_RESERVED_WORDS_H

#include "Token.h"

/// \desc Defines the information the lexer needs to know to determine if a
/// key word is present in the source code.
/// \remark The last entry in the table is a 0 length entry which tells the
/// search code there are no more entries to check.
class KeyWord
{
public:
    /// \desc Creates an empty key word.
    KeyWord();

    /// \desc Creates a key word with the supplied cText.
    explicit KeyWord(const char *word, TokenTypes type);

    /// \desc cleans up internal resources.
    ~KeyWord();

    /// \desc checks if the key word matches the supplied u8string value. This is a case sensitive compare.
    bool IsEqual(U8String *value);

    /// \desc Checks if the reserved work token matches the token tokenTypes.
    bool IsEqual(TokenTypes tokenTypes);

    /// \desc Gets the Type of this key word.
    inline TokenTypes Type()
    { return type; }


private:
    /// \desc the cText characters that makeup the key word.
    U8String   *text;
    /// \desc The token Type that defines how the key word is parsed.
    TokenTypes type;
};

extern KeyWord *keyWords[];

#endif //DSL_RESERVED_WORDS_H
