//
// Created by krw10 on 6/22/2023.
//
#include <cstdio>
#include "../Includes/Lexer.h"
#include <cstring>
#include <malloc.h>

#ifdef END_TOKEN
#undef END_TOKEN
#endif
#define END_TOKEN INVALID_TOKEN

extern void DisplayTokens();
extern void DisplayTokenInfo(int64_t index);


void DisplayCheckTokens([[maybe_unused]] Lexer *lexer)
{
    char *buffer = (char *)calloc(1, 4096);
    buffer[0] = '\0';
    strcat(buffer, "TokenTypes checkTokens[] =\n{\n");
    for(int64_t ii=0; ii<Lexer::TotalTokens(); ++ii)
    {
        TokenTypes type = Lexer::GetTokenType(ii);
        const char *name = tokenNames[(int64_t)type & 0xFF];
        sprintf(buffer+strlen(buffer), "    %s,\n", name);
    }
    strcat(buffer, "    END_TOKEN\n");
    strcat(buffer, "};");
    printf( "%s", buffer);

    free(buffer);
}

///
/// \desc Creates a lexer and processes a specific test _s, then validates and reports the results.
/// \param testScript CString containing the test _s to process.
/// \param checkTokens If NULL this is ignored, else it should contain a list of the tokens expected to be generated.
/// \param testName CString containing the _n of the test.
/// \param expectedResult True if the lexer should be expected to succeed, or false if the not.
/// \param validate If NULL this is ignored, else it should contain a pointer to a function that
/// is used to valid the lexer post run test results. If present the return value from this
/// function overrides the normal test processing.
/// \return True if the test succeeds and false if it fails.
/// \param index Optional int64_t passed to the validate function.
/// \param intValue Optional integer value passed to validate function.
/// \param doubleValue Optional double value passed to validate function.
/// \param stringValue Optional string value passed to validate function.
/// \return True if the test passes or false if it fails validation.
bool RunTest(const char *testScript,
             const TokenTypes *checkTokens,
             const char *testName,
             bool expectedResult,
             bool (*validate)(Lexer *lexer, int64_t index, int64_t intValue, double doubleValue, const char *stringValue),
             int64_t index,
             int64_t intValue,
             double doubleValue,
             const char *stringValue,
             bool displayCheckTokens = false,
             bool isFile = false, //DEPRECATED, not used.
             const char *fileName = "lexer")
{
    total_run++;

    printf(">>>>> Running Test <<<<<\n");
    printf("Program Script:\n");

    auto *lexer = new Lexer();

    tokens.Clear();
    variables.Clear();
    functions.Clear();

    if (isFile)
    {
        printf("DEPRECATED\n"); //not used.
    }
    else
    {
        printf("%s\n", testScript);
    }

    bool rc;
    printf("Test: %s\n", testName);
    bool result;
    if ( isFile )
    {
        printf("DEPRECATED\n");
    }
    else
    {
        Lexer::Initialize();
        int64_t id = Lexer::AddModule("test", "lexer_tests.cpp", testScript);
        result = lexer->Lex();
    }
    printf("Lexer status = %s\n", (result) ? "true" : "false");
    printf("Errors %d Warnings %d\n", errors, warnings);
    if ( checkTokens != nullptr )
    {
        for(int64_t ii=0; checkTokens[ii] != END_TOKEN; ++ii)
        {
            if ( Lexer::GetTokenType(ii) != checkTokens[ii] )
            {
                printf("Tokens do not match at index = %d\n", ii);
                DisplayTokenInfo(ii);
                result = false;
                break;
            }
        }
    }
    if ( result && validate != nullptr)
    {
        result = validate(lexer, index, intValue, doubleValue, stringValue);
    }
    if ( result == expectedResult )
    {
        rc = true;
        printf("Result: Test Succeeded.\n");
    }
    else
    {
        rc = false;
        printf("Result: Test Failed.\n");
    }

    if ( displayCheckTokens )
    {
        DisplayCheckTokens(lexer);
    }
    else
    {
        DisplayTokens();
    }

    delete lexer;

    if ( rc )
    {
        total_passed++;
    }
    else
    {
        total_failed++;
    }

    return rc;
}

void PrintModifier([[maybe_unused]] Lexer *lexer, int64_t index)
{
    if ( Lexer::TotalTokens() > index )
    {
        switch(Lexer::GetTokenModifier(index))
        {
            case TMScriptScope:
                printf("TMScriptScope\n");
                break;
            case TMGlobalScope:
                printf("TMGlobalScope\n");
                break;
            case TMLocalScope:
                printf("TMLocalScope\n");
                break;
            default:
                printf("%d\n",Lexer::GetTokenModifier(index));
                break;
        }
    }
}

/// \desc Checks if the const modifier has been applied. If so the token will be marked read only.
bool CheckConst([[maybe_unused]] Lexer *lexer, int64_t index, __attribute__((unused)) int64_t iValue, __attribute__((unused)) double dValue, __attribute__((unused)) const char *sValue)
{
    if ( Lexer::TotalTokens() > index )
    {
        return Lexer::IsReadOnly(index) == true;
    }

    return false;
}

/// \desc Validates if the scope _s modifier is applied correctly.
/// \param self Passed in pointer to the lexer.
/// \return True if the test should succeed, else false.
bool CheckModifier(Lexer *lexer, int64_t index, int64_t iValue, __attribute__((unused)) double dValue, __attribute__((unused)) const char *sValue)
{
    PrintModifier(lexer, index);
    if ( Lexer::TotalTokens() > index )
    {
        return Lexer::GetTokenModifier(index) == iValue;
    }

    return false;
}

bool CheckModifierAndWarnings(Lexer *lexer, int64_t index, int64_t iValue, double dValue, const char *sValue)
{
    if (CheckModifier(lexer, index, iValue, dValue, sValue))
    {
        return false;
    }

    return warnings == 1;
}

static char empty[] = { "" };

//various variable tests, also tests identifiers.
__attribute__((unused)) bool RunVariableDeclarationTests()
{

    //valid change scope to global.
    RunTest("var global myGlobalVar;", nullptr, "Successfully create variable and modify scope to global.", true,
            CheckModifier, 0, TMGlobalScope, 0.0, empty, false);

    //valid change scope to local.
    RunTest("var local _a100z5;", nullptr, "Successfully create variable and modify scope to local.", true,
            CheckModifier, 0, TMLocalScope, 0.0, empty, false);

    //valid change variable to const must not produce error.
    RunTest("var const cVar;", nullptr, "Create variable and set it to const without a specified value.", true,
            CheckConst, 0, 0, 0.0, empty, false);

    //valid change variable to const must not produce error.
    RunTest("var const local cVar;", nullptr, "Create variable at local scope and set it to const without a specified value.", true,
            CheckConst, 0, 0, 0.0, empty, false);

    //Check that value can be assigned to a variable when it is created.
    TokenTypes ttResultAssign10[] =
    {
            VARIABLE_DEF,
            ASSIGNMENT,
            INTEGER_VALUE,
            SEMICOLON,
            END_TOKEN
    };
    RunTest("var result = 10;",ttResultAssign10, "Successfully create variable and assign integer number 10.", true, nullptr, 0, 0, 0.0, empty, false);


    return true;

}

bool ValidateIdentifier([[maybe_unused]] Lexer *lexer, int64_t index, const char *string)
{
    if( !tokens[index]->identifier->IsEqual(string) )
    {
        printf("Expected variable %s received %s\n", string, tokens[index]->identifier->cStr());
        return false;
    }

    return true;
}

bool ValidateStringValue(Lexer *lexer, int64_t index, const char *string)
{
    if (Lexer::GetTokenValueType(index) != STRING_VALUE )
    {
        printf("Value is not a string value.\n");
        return false;
    }

    return true;
}

__attribute__((unused)) bool CheckAssignSimpleString(Lexer *lexer,
                             __attribute__((unused)) int64_t index,
                             __attribute__((unused)) int64_t iValue,
                             __attribute__((unused)) double dValue,
                             __attribute__((unused)) const char *sValue)
{
    if ( !ValidateIdentifier(lexer, 0, "myString"))
    {
        return false;
    }

    if ( !ValidateStringValue(lexer, 2, "Hello World!") )
    {
        return false;
    }

    return true;
}

__attribute__((unused)) bool PrintAndValidateString(Lexer *lexer, int64_t index,
                            __attribute__((unused)) int64_t iValue,
                            __attribute__((unused)) double dValue,
                            const char *sValue)
{
    printf("[%s]\n", tokens[index]->value->sValue.cStr());
    return tokens[index]->value->sValue.IsEqual(sValue);
}

__attribute__((unused)) void RunTestsWithStringValues()
{
   TokenTypes checkTokens[] =
   {
           VARIABLE_DEF,
           ASSIGNMENT,
           STRING_VALUE,
           SEMICOLON,
           END_TOKEN
   };
   RunTest("var myString = \"Hello World!\";", checkTokens, "Successfully create simple string value.",
           true, CheckAssignSimpleString, 0, 0, 0.0, empty, false);

    //Note: Have to add two sets of slashes to get // in a quoted string this is specific to test case only.
    RunTest(R"(var myString = " \\ ";)", checkTokens, "Successfully create string with single \\.", true, PrintAndValidateString,
            2, 0, 0.0, " \\ ", false);

    RunTest(R"(var myString = " \' ";)", checkTokens, "Successfully create string with single quote.", true, PrintAndValidateString,
            2, 0, 0.0, " \' ", false);

    RunTest(R"(var myString = "Line1\nLine2";)", checkTokens, "Successfully create string with new line.", true, PrintAndValidateString,
        2, 0, 0.0, "Line1\nLine2", false);
    RunTest(R"(var myString = "Line1\rLineX";)", checkTokens, "Successfully create string with carriage return.", true,
            PrintAndValidateString, 2, 0, 0.0, "Line1\rLineX", false);
    RunTest(R"(var myString = "X\tY";)", checkTokens, "Successfully create string with tab.", true,
            PrintAndValidateString, 2, 0, 0.0, "X\tY", false);
    RunTest(R"(var myString = "X\bY";)", checkTokens, "Successfully create string with backspaces.", true,
        PrintAndValidateString, 2, 0, 0.0, "X\bY", false);
    RunTest(R"(var myString = "X\fY";)", checkTokens, "Successfully create string with backspace.", true,
            PrintAndValidateString, 2, 0, 0.0, "X\fY", false);
    RunTest(R"(var myString = " \{ ";)", checkTokens, "Successfully create string with open curly brace.", true,
            PrintAndValidateString, 2, 0, 0.0, " { ", false);
    RunTest(R"(var myString = " \} ";)", checkTokens, "Successfully create string with close curly brace.", true,
            PrintAndValidateString, 2, 0, 0.0, " } ", false);
    RunTest(R"(var myString = " \126 ";)", checkTokens, "Successfully create string with ~.", true,
            PrintAndValidateString, 2, 0, 0.0, " ~ ", false);
    RunTest(R"(var myString = " \x7E ";)", checkTokens, "Successfully create string with ~.", true,
            PrintAndValidateString, 2, 0, 0.0, " ~ ", false);
    RunTest(R"(var myString = " \X7E ";)", checkTokens, "Successfully create string with ~.", true,
            PrintAndValidateString, 2, 0, 0.0, " ~ ", false);

    TokenTypes checkTokens1[] =
    {
        VARIABLE_DEF,
        ASSIGNMENT,
        INTEGER_VALUE,
        SEMICOLON,
        VARIABLE_DEF,
        ASSIGNMENT,
        STRING_VALUE,
        SEMICOLON,
        END_TOKEN
    };

    RunTest("var vx = 10; var myString = \" {vx}\";", checkTokens1,  "Successfully create string with variable value inline.",
            true, PrintAndValidateString, 6, 0, 0.0, " {vx}", false);

    TokenTypes checkTokensStringAddAssignment[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               STRING_VALUE,
                               SEMICOLON,
                               VARIABLE_VALUE,
                               ADD_ASSIGNMENT,
                               STRING_VALUE,
                               SEMICOLON,
                               END_TOKEN
                       };

    RunTest(R"(var abc = "Hello"; abc += " foo";)", checkTokensStringAddAssignment, "Test value assign string variable and add equal another string.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest(R"(var abc = "Hello"; var abc += " foo";)", checkTokensStringAddAssignment, "Fail with already defined with add equal string value.",
            false, nullptr, 0, 0, 0.0, empty, false);
}

__attribute__((unused)) bool RunTestWithTokens()
{
    TokenTypes checkTokens[] =
    {
            FUNCTION_CALL_BEGIN,
            PARAM_BEGIN,
            INTEGER_VALUE,
            ADDITION,
            INTEGER_VALUE,
            MULTIPLY,
            INTEGER_VALUE,
            PARAM_END,
            FUNCTION_CALL_END,
            SEMICOLON,
            END_TOKEN
    };

    return RunTest("print(10+5*3);",checkTokens, "Successfully create print function call with expression.", true, nullptr, 0, 0, 0.0, empty, false);
}

__attribute__((unused)) void FunctionDeclarationTests()
{
    TokenTypes checkTokens[] =
    {
            FUNCTION_DEF_BEGIN,
            FUNCTION_DEF_END,
            END_TOKEN
    };

    RunTest("fun Func() { }", checkTokens, "Function with no parameters default scope.", true,
            CheckModifier, 0, TMScriptScope, 0.0, empty, true);

    RunTest("fun _s Func() { }",checkTokens, "Function with no parameters _s scope.", true,
            CheckModifier, 0, TMScriptScope, 0.0, empty, false);

    RunTest("fun global Func() { }",checkTokens, "Function with no parameters global scope.", true,
            CheckModifier, 0, TMGlobalScope, 0.0, empty, false);

    RunTest("fun local Func() { }", checkTokens, "Function with no parameters local scope.", false,
            CheckModifierAndWarnings, 0, TMLocalScope, 0.0, empty, false);

    RunTest("fun block Func() { }", nullptr, "Function with no parameters block scope.", false,
            CheckModifierAndWarnings, 0, TMScriptScope, 0.0, empty, false);

    TokenTypes checkTokens1[] =
    {
            FUNCTION_DEF_BEGIN,
            FUNCTION_DEF_END,
            END_TOKEN
    };

    WarningLevels saved = warningLevel;
    warningLevel = WarningLevel0;
    warningLevel = saved;

    RunTest("fun Func(global a) { }", nullptr, "Function with parameters with scope modifiers.", false,
             nullptr, 0, 0, 0.0, empty, false);

    RunTest("fun Func(a, ) { }", nullptr, "Function with parameters missing last parameter.", false,
            nullptr, 0, 0, 0.0, empty, false);

   RunTest("fun Func(, bValue, cValue) { }", nullptr, "Function with parameters missing first parameter.", false,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("fun Func, ) { }", nullptr, "Function missing open parenthesis.", false,
            nullptr, 0, 0, 0.0, empty, false);

    //functions with stuff tests.

}

__attribute__((unused)) bool TestProcessFile()
{
    return RunTest("_f.dsl", nullptr, "Test read and process _f", true,
            nullptr, 0, 0, 0.0, empty, false, true);
}

__attribute__((unused)) void RunCommentTests()
{
    RunTest("var a = 1;\n/* This is a multi-line comment\n*\n*\n*/\nvar bValue = a + 1;", nullptr, "Multi-line comment.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = \n/* This is a multi-line comment. */ 10;", nullptr, "Multi-line comment.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = 1; //This is a single line comment.\nvar bValue = a;", nullptr, "Single line comment.", true,
            nullptr, 0, 0, 0.0, empty, false);

    TokenTypes checkTokens[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               MULTI_LINE_COMMENT,
                               INTEGER_VALUE,
                               SEMICOLON,
                               END_TOKEN
                       };
    RunTest("var a = \n/* This is a multi-line comment. */ 10;", checkTokens, "Multi-line comment within expression.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("/* Outside /* This is an inner multi-line comment. */ comment. */", nullptr,
            "Multi-line comment /**//**/within Multi-line comment.", true,
            nullptr, 0, 0, 0.0, empty, false);
}

__attribute__((unused)) void QuickTest(const char *src, const char *cmt)
{
    RunTest(src, nullptr, cmt, true,
            nullptr, 0, 0, 0.0, empty, false);
}

__attribute__((unused)) void RunIncDecTests()
{
    TokenTypes checkTokensPreInc[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               INTEGER_VALUE,
                               SEMICOLON,
                               PREFIX_INC,
                               VARIABLE_VALUE,
                               SEMICOLON,
                               END_TOKEN
                       };

    RunTest("var a = 5;\n++a;", checkTokensPreInc, "Test valid pre increment.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = 5;\n++cValue;", nullptr, "Test invalid pre increment.", false,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = 5;\n++;", nullptr, "Test invalid pre increment.", false,
            nullptr, 0, 0, 0.0, empty, false);

    TokenTypes checkTokensPreDec[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               INTEGER_VALUE,
                               SEMICOLON,
                               PREFIX_DEC,
                               VARIABLE_VALUE,
                               SEMICOLON,
                               END_TOKEN
                       };

    TokenTypes checkTokensPostDec[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               INTEGER_VALUE,
                               SEMICOLON,
                               VARIABLE_VALUE,
                               POSTFIX_DEC,
                               SEMICOLON,
                               END_TOKEN
                       };

    RunTest("var a = 5;\n--a;", checkTokensPreDec, "Test valid pre decrement.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = 5;\na--;", checkTokensPostDec, "Test valid post decrement.", true,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = 5;\n--5;", nullptr, "Test invalid pre decrement number.", false,
            nullptr, 0, 0, 0.0, empty, false);

    RunTest("var a = 5;\n--cValue;", nullptr, "Test invalid pre decrement variable.", false,
            nullptr, 0, 0, 0.0, empty, false);
}

__attribute__((unused)) void RunIfElseTests()
{
    TokenTypes checkTokens[] =
    {
            VARIABLE_DEF,
            ASSIGNMENT,
            INTEGER_VALUE,
            SEMICOLON,
            IF,
            OPEN_PAREN,
            VARIABLE_VALUE,
            EQUAL_TO,
            INTEGER_VALUE,
            CLOSE_PAREN,
            OPEN_BLOCK,
            FUNCTION_CALL_BEGIN,
            PARAM_BEGIN,
            STRING_VALUE,
            PARAM_END,
            FUNCTION_CALL_END,
            SEMICOLON,
            CLOSE_BLOCK,
            ELSE,
            OPEN_BLOCK,
            FUNCTION_CALL_BEGIN,
            PARAM_BEGIN,
            STRING_VALUE,
            PARAM_END,
            FUNCTION_CALL_END,
            SEMICOLON,
            CLOSE_BLOCK,
            END_TOKEN
    };

    RunTest("var a = 5;\nif ( a == 5 )\n    {\n    print(\"a is 5.\");\n    }\n    else\n    {\n        print(\"a is not 5.\");\n    }",
            checkTokens, "Test assignment and if else.", true,
            nullptr, 0, 0, 0.0, empty, false);

}

__attribute__((unused)) void RunLogicInstructionsTests()
{
    TokenTypes checkTokens[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               INTEGER_VALUE,
                               LOGICAL_AND,
                               INTEGER_VALUE,
                               SEMICOLON,
                               END_TOKEN
                       };
    RunTest("var a = 1 && 1;",
            checkTokens, "Test assignment with and.", true,
            nullptr, 0, 0, 0.0, empty, false);

    TokenTypes checkTokens1[] =
                       {
                               VARIABLE_DEF,
                               ASSIGNMENT,
                               INTEGER_VALUE,
                               LOGICAL_OR,
                               INTEGER_VALUE,
                               SEMICOLON,
                               END_TOKEN
                       };
    RunTest("var a = 1 || 1;",
            checkTokens1, "Test assignment with or.", true,
            nullptr, 0, 0, 0.0, empty, false);

}

__attribute__((unused)) void RunForTests()
{
    TokenTypes checkTokens[] =
    {
            FOR,
            OPEN_PAREN,
            VARIABLE_DEF,
            ASSIGNMENT,
            SEMICOLON,
            VARIABLE_VALUE,
            LESS_THAN,
            INTEGER_VALUE,
            SEMICOLON,
            PREFIX_INC,
            VARIABLE_VALUE,
            CLOSE_PAREN,
            OPEN_BLOCK,
            FUNCTION_CALL_BEGIN,
            PARAM_BEGIN,
            STRING_VALUE,
            COMMA,
            PARAM_END,
            PARAM_BEGIN,
            VARIABLE_VALUE,
            COMMA,
            PARAM_END,
            PARAM_BEGIN,
            STRING_VALUE,
            PARAM_END,
            FUNCTION_CALL_END,
            SEMICOLON,
            CLOSE_BLOCK,
            END_TOKEN
    };

    RunTest(R"(for(var ii = a; ii < 10; ++ii) { print("ii = ", ii, "\n"); })",
            checkTokens, "For loop with print inside block.", true,
            nullptr, 0, 0, 0.0, empty, false);
}

void RunLexerTests()
{

    RunVariableDeclarationTests();
    RunTestsWithStringValues();
    RunCommentTests();
    FunctionDeclarationTests();
    RunIncDecTests();
    RunIfElseTests();
    RunForTests();
    RunLogicInstructionsTests();

//    QuickTest("var global myGlobalVar = 10;", "test");

/*

*/
    //        -a = 10;

    //parse collections.

//    QuickTest("var a = 10;\n", "test");


    //QuickTest("switch(1)\n{\n    case 1:\n        print(\"1\");\n        break;\n    case 2:\n        print(\"2\"); break; }", "test");

    printf("\nTotal Lexer Tests Run: %d, Total Passed: %d, Total Failed: %d\n", total_run, total_passed, total_failed);

}