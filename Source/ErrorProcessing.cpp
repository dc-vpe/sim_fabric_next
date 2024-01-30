//
// Created by krw10 on 8/8/2023.
//
#include <cstdio>
#include <cstdarg>
#include "../Includes/ErrorProcessing.h"
#include "../Includes/ParseData.h"
#include "../Includes/CPU.h"

static char szLastErrorMessage[8192] = {'\0'};
static char szErrorMessage[8192] = {'\0'};
static char szMsgBuffer[8192] = {'\0'};

List<U8String *> printedIssues;

/// \desc Prints an issue to the std out.
void PrintIssue(int64_t number, bool error, bool fatalError, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsprintf (szMsgBuffer, format, args);
    va_end(args);
    PrintError(number, szMsgBuffer, error, fatalError);
}

/// \desc Same as PrintError, keeps the sen tool from renumbering this call as the
///       specific error code is passed into the parent method.
void PrintPassedIssue(int64_t number, bool error, bool fatalError, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsprintf (szMsgBuffer, format, args);
    va_end(args);
    PrintError(number, szMsgBuffer, error, fatalError);
}

/// \desc Function that is responsible for printing the formatted error message,
///       updating the error count and setting fatal if a fatal error.
void PrintError(int64_t number, const char *msg, bool error, bool fatalError)
{
    fatal = fatalError;

    if (!error && warningLevel == WarningLevel0 )
    {
        return;
    }

    if ( warningLevel == WarningLevel3 )
    {
        error = true;
    }

    for(int64_t ii=0; ii<printedIssues.Count(); ++ii)
    {
        if ( printedIssues[ii]->IsEqual(msg) )
        {
            return;
        }
    }
    printedIssues.push_back(new U8String(msg));

    //if a run time error no line and column
    if ( number >= 4000 && number <= 5000)
    {
        sprintf(szErrorMessage, "Run Error(%ld): %s\n", (long)number, msg);
        CPU::RaiseError(number, szErrorMessage);
    }
    else
    {
        sprintf(szErrorMessage,
                "%s(%ld): %s at line %ld, column %ld\n", ((error) ? "Error" : "Warning"),
                (long)number, msg, (long)locationInfo.line, (long)locationInfo.column);

        if ( strcmp(szErrorMessage, szLastErrorMessage) != 0 )
        {
            strcpy(szLastErrorMessage, szErrorMessage);
            printf("%s", szErrorMessage);
        }
    }

    if ( error )
    {
        errors++;
    }
    else
    {
        warnings++;
    }
}