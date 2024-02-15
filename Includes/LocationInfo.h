//
// Created by krw10 on 7/26/2023.
//

#ifndef DSL_C_LOCATIONINFO_H
#define DSL_C_LOCATIONINFO_H

#include "dsl_types.h"

/// \desc This structure contains the information about the position in the
///       source code being parsed.
class LocationInfo
{
public:
    /// \desc Current parse position in the source code.
    int64_t location;

    /// \desc Current line in the source code.
    int64_t line;

    /// \desc Current column in the source code.
    int64_t column;

    /// \desc Number of open parenthesis encountered at this point in the source file.
    int64_t openParens;

    /// \desc Number of close parenthesis encountered at this point in the source file.
    int64_t closeParens;

    /// \desc Last number of openBlocks before the position was incremented.
    int64_t openBlocks;

    /// \desc Last number of openBlocks before the position was incremented.
    int64_t closeBlocks;

    /// \desc Returns the difference between open parenthesis and close parenthesis at this source position.
    [[nodiscard]] int64_t Parens() const { return openParens - closeParens; }

    /// \desc Returns the difference between open blocks and close blocks at this source position.
    [[nodiscard]] int64_t Blocks() const { return openBlocks - closeBlocks; }

    /// \desc Sets the position to the start of the source.
    void Reset()
    {
        location = 0;
        line = 1;
        column = 1;
        openParens = 0;
        closeParens = 0;
        openBlocks = 0;
        closeBlocks = 0;
    }

    /// \desc Creates a new locationInfo set to the start of the source code file being lexed.
    LocationInfo() : location(0),
                     line(1),
                     column(1),
                     openParens(0),
                     closeParens(0),
                     openBlocks(0),
                     closeBlocks(0)
    {
    }

    /// \desc Moves the position forward 1 character position, line and column are updated
    ///       as necessary to track with the position being parsed.
    /// \param ch Current character read, the position info uses this character to process
    ///           columns, lines, parens, and braces.
    void Increment(u8chr ch)
    {
        ++location;
        if ((char) ch == '(' )
        {
            ++openParens;
        }
        if ((char)ch == ')' )
        {
            ++closeParens;
        }
        if ((char) ch == '{' )
        {
            ++openBlocks;
        }
        if ((char)ch == '}' )
        {
            ++closeBlocks;
        }
        if ((char) ch == '\n')
        {
            ++line;
            column = 1;
        }
        else
        {
            ++column;
        }
    }
};

#endif //DSL_C_LOCATIONINFO_H
