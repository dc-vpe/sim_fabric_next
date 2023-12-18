//
// Created by krw10 on 7/31/2023.
//

#ifndef DSL_CPP_WARNING_LEVELS_H
#define DSL_CPP_WARNING_LEVELS_H

/// \desc Defines the warning levels used by the DSL compiler. Specified on the command line
///       with the -W1, -W2, -W3, -W4 flag. Default is W3
enum WarningLevels
{
    /// \desc Warnings are turned off.
    WarningLevel0 = 0, /// \desc Warnings are ignored.
    WarningLevel1 [[maybe_unused]] = 1, /// \desc Displays all warnings, but warnings are not tracked as errors.
    WarningLevel2 = 2, /// \desc Displays all level 2 warnings plus severe warnings that
                       ///       are likely to cause problems.
    WarningLevel3 = 3, /// \desc Warnings are treated as errors.
};
#endif //DSL_CPP_WARNING_LEVELS_H
