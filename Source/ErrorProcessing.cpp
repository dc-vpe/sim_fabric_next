//
// Created by krw10 on 8/8/2023.
//
#include <cstdio>
#include <cstdarg>
#include "../Includes/ErrorProcessing.h"
#include "../Includes/ParseData.h"
#include "../Includes/CPU.h"

static char szLastErrorMessage[2048] = {'\0'};
static char szErrorMessage[2048] = {'\0'};

List<U8String *> printedIssues;

void PrintIssue(int64_t number, bool error, bool fatalError, const char *format, ...)
{
    char szBuffer[1024];

    va_list args;
    va_start (args, format);
    vsprintf (szBuffer, format, args);
    va_end(args);
    PrintIssue(number, szBuffer, error, fatalError);
}

void PrintIssue(int64_t number, const char *msg, bool error, bool fatalError)
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
        sprintf(szErrorMessage, "Run Error(%lld): %s\n", number, msg);
        CPU::RaiseError(szErrorMessage);
    }
    else
    {
        sprintf(szErrorMessage,
                "%s(%lld): %s at line %lld, column %lld\n", ((error) ? "Error" : "Warning"),
                number, msg, locationInfo.line, locationInfo.column);

        if ( strcmp(szErrorMessage, szLastErrorMessage) != 0 )
        {
            strcpy_s(szLastErrorMessage, szErrorMessage);
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