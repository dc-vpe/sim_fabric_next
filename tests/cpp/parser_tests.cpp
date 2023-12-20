//
// Created by krw10 on 8/21/2023.
//

#include <ctime>
#include "../../Includes/Lexer.h"
#include "../../Includes/parser.h"
#include "../../Includes/CPU.h"

extern void DisplayTokens();

bool RunTest(const char *testScript,
             const char *testName,
             bool expectedResult = true,
             bool lexOnly = false,
             bool showTokenList = false,
             bool showCompiledCode = false)
{
    total_run++;

    printf(">>>>> Running Test <<<<<\n");
    printf("Program Script:\n");
    printf("%s\n", testScript);

    printf("Test: %s\n", testName);

    auto *lexer = new Lexer();

    tokens.Clear();
    variables.Clear();
    functions.Clear();

    bool rc;
    bool result;
    Lexer::Initialize();
    Lexer::AddModule("parser", "parser_tests.cpp", testScript);
    result = lexer->Lex();

    if ( showTokenList )
    {
        DisplayTokens();
    }

    delete lexer;

    if ( result && errors == 0 && warnings == 0 )
    {
        if ( !lexOnly )
        {
            auto *parser = new Parser();
            result = parser->Parse();
            CPU *cpu = new CPU();

            if ( showCompiledCode )
            {
                printf("Program Code:\n");

                CPU::DisplayASMCodeLines();
            }

            cpu->Run();

            delete cpu;
        }
    }

    if ( result == expectedResult && errors == 0 && warnings == 0 )
    {
        rc = true;
        printf("Result: Test Succeeded.\n");
    }
    else
    {
        rc = false;
        printf("Result: Test Failed.\n");
    }

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

[[maybe_unused]] void Tests()
{
    RunTest(R"(print(20 + (32 - 5) + 10/2,"\n");)", "Check expression with ()", true);
    RunTest(R"(var myVariable = 10 + 32; print("myVariable = ", myVariable, "\n");)", "Check create variable with assignment", true);
    RunTest(R"(var myVariable = 10; var foo = 100; myVariable += foo; print("myVariable = ", myVariable, "\n");)", "Check create add assignment with variables", true);
    RunTest(R"(print("The answer is ", 20 + 32 - 5 + 10/2, "\n");)", "Check expression without ()");
}

extern void DisplayTokenAsText(int64_t index, TokenTypes type);

/// \desc Runs a compiler test case.
/// \param script script to run
/// \param displayInfo True if lexing information should be displayed when lexing has no errors or warnings.
/// \param runParser True if the lexed result should be processed by the parser.
/// \param runCpu True if the program should be executed if it parses without error or warnings.
/// \return True if no errors or warnings occur, else false.
bool RunTest(const char *script, bool displayInfo = true, bool runParser = true, bool runCpu = true)
{
    total_run++;

    auto *lexer = new Lexer();
    Lexer::Initialize();
    Lexer::AddModule("parser", "parser_tests.cpp", script);

    printf("\n<<<< Script >>>>\n");
    printf("\n%s\n", script);

    lexer->Lex();

    if ( displayInfo )
    {
        printf("\n<<<< Lexer Output >>>>\n");
        for(int64_t ii=0; ii<tokens.Count(); ++ii)
        {
            DisplayTokenAsText(ii, tokens[ii]->type);
        }
    }

    if ( errors > 0 || warnings > 0 )
    {
        total_failed++;
        delete lexer;
        return false;
    }

    delete lexer;

    if ( runParser )
    {
        auto *parser = new Parser();

        if ( !parser->Parse() )
        {
            total_failed++;
            return false;
        }

        if ( displayInfo )
        {
            printf("\n<<<< Parser Output >>>>\n");
            CPU::DisplayASMCodeLines();
        }

        delete parser;
    }

    if ( errors > 0 || warnings > 0 )
    {
        total_failed++;
        return false;
    }

    if ( runCpu )
    {
        printf("\n<<<< Run >>>>\n");
        auto *cpu = new CPU();

        double start = (double)clock()/(double)CLOCKS_PER_SEC;
        cpu->Run();
        double end = (double)clock()/(double)CLOCKS_PER_SEC;
        printf("\nRun Time : %f\n", end - start);

        delete cpu;

        printf("\n");
    }

    total_passed++;

    return true;
}

void RunAllParserTests()
{
    warningLevel = WarningLevel2;

//    RunTest(R"(var a = 9; ++a; if ( a < 10 ) { print("a < 10", "\n"); } else if ( a == 9 ) { print("a != 10\n"); } else if ( a >= 25 ) { --a; } )");
//    RunTest(R"(var a; print(a, "\n"); while( ++a < 1000000 ) {    } print(a);)");
//    RunTest(R"(var global a = 1, local b = 2; ++b; ++a; var c = a + b;)");
//    RunTest(R"(var a; a = a++ + 3; print(a);)");
//    RunTest(R"(for(var ii=0; ii<10; ii++) { print(a); } )");
//    RunTest(R"(var limit = 1000000; var counter = 0; while( counter < limit) { counter++; } print(counter); )" );
//    RunTest(R"(var a = 0; while( ++a < 1000000) {  } print(a); )" );
//    RunTest(R"(var a; a = 10 + 10;)");
//    RunTest(R"(for(var ii; ii<1000000; ++ii) { } print(ii); })");
//    RunTest(R"(var a = 1; switch( a - 3 ) { default: print("other", " ", a - 3); case 1: { print("one"); } case 2: { print("two"); } } )");
//
//    RunTest(R"(test(); var test() { print("Hello World\n"); })", true, true, true);
//
//    RunTest(R"(var msg = 1+5; var w = msg + 2; print(w);)", true, true, true);
//    RunTest(R"(test(44, 2); var test(c, d) { print(d); })", true, true, true);
//    RunTest(R"(var a = 10; print(a); a += 10; print(a); )");
//    RunTest(R"(if( test() == true) { print("hello"); } else { print("good bye"); } var test() { return true; } )", true, true, true);
//    RunTest(R"(var a = 0; while( a < 10 ) { ++a; print(a, "\n"); } )", true, true, true);
//    RunTest(R"(var a = 0; while( a < 10 ) { ++a; print(a, "\n"); } )", true, true, true);
//    RunTest(R"(test(1, 2); var test(p1, p2) { --p2; print(p2); })", true, true, true);

//    RunTest(R"(for(var ii=0; ii<10; ++ii) { print(ii, "\n"); })", true, true, false);
//    RunTest(R"(var x; var a = 10; var b = x + a; print(b); )", true, true, true);

//    RunTest(R"(for(var ii=0; ii<10; ++ii) { print(ii, "\n"); })", true, true, true);
//    RunTest(R"(var a = 11; switch( a ) { default: { for(var ii=0; ii<10; ++ii) { print(ii, "\n"); } } case 1: { print("one"); } case 2: { print("two"); } } )");
//    RunTest(R"(var a = 1; if ( a == 1 ) { if ( a == 1) { for(var ii=0; ii<10; ++ii) { print(ii, "\n"); } } })");
//    RunTest(R"(var a = "1"; switch(a) { case 1: print("uno"); case 2: print("dos"); default: print("other"); } )");
//    RunTest(R"(var a = 1; var b = 2; switch(a) { case 1: switch(b) { case 2: { print("a = 1, b = 2"); } } } )");
//    RunTest(R"(test(); var test() { var a = 10; print(a); } )");
//    RunTest(R"(test("hello"); var test(msg) { var local w = msg + " world"; print(w); })", true, true, true);
//    RunTest(R"(test(); var test() { var local w = 21; print(w); })", true, true, true);

//    RunTest(R"(for(var ii=0; ii<10; ++ii) { print(ii, "\n"); })", true, true, true);

    RunTest(R"(var fn() { })", false, true, true);
}