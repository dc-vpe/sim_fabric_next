//
// Created by krw10 on 8/8/2023.
//

#ifndef DSL_CPP_ERROR_PROCESSING_H
#define DSL_CPP_ERROR_PROCESSING_H

#include "WarningLevels.h"
#include "LocationInfo.h"

/// \desc Prints error and warning messages along with the tok parse line and column,
/// and updates error and warning variables.
/// \param number Error or warning number.
/// \param error True if this is an error, false if it is a warning.
/// \param fatalError True if this error force the lex or compile to stop.
/// \param format CString printf style format string message describing the error.
/// \param args one or more optional arguments as specified in the format string.
/// \remark The msg format should be formatted as the beginning of the sentence. For example,
///         "Expected a semi-colon after break"
/// \remark This method calls the other print issue to display the issue.
void PrintIssue(int64_t number, bool error, bool fatalError, const char *format, ...);

/// \desc Prints error and warning messages along with the tok parse line and column,
/// and updates error and warning variables.
/// \param number Error or warning number.
/// \param msg CString message describing the error
/// \param error True if this is an error, false if it is a warning.
/// \param fatalError True if this error force the lex or compile to stop.
/// \remark The msg format should be formatted as the beginning of the sentence. For example,
///         "Expected a semi-colon after break"
/// \remark If error is false the issue is a warning, if warning level is 3 or more then
///         the warning is treated like an error. If warning level is 0 no messages is
///         printed, if level is 1 or 2 then then the warning is printed but not treated
///         like an error.
void PrintIssue(int64_t number, const char *msg, bool error, bool fatalError = false);

#endif //DSL_CPP_ERROR_PROCESSING_H
