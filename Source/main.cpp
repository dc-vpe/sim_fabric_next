//
// Created by krw10 on 9/7/2023.
//

// Main compiler and developmental test driver

/// \desc If false development direct call test is run, else the compiler is generated.
/// \remark Once we reach feature complete the test path will be removed. Its present
///         only for development.

#include "../Includes/parser.h"
#include "../Includes/CPU.h"
#include "../Includes/JsonParser.h"
#include <iostream>
#include <unistd.h>

bool debugMode = false;

/// \desc array of token names arranged in ID order used as debug aid to display a token _n
/// in error and warning messages.
const char *tokenNames[] =
{
    "VALID_TOKEN",
    "OPEN_BLOCK",
    "CLOSE_BLOCK",
    "OPEN_PAREN",
    "CLOSE_PAREN",
    "OPEN_BRACE",
    "CLOSE_BRACE",
    "EXPONENT",
    "PREFIX_INC",
    "PREFIX_DEC",
    "UNARY_POSITIVE",
    "UNARY_NEGATIVE",
    "UNARY_NOT",
    "CAST_TO_INT",
    "CAST_TO_DBL",
    "CAST_TO_CHR",
    "CAST_TO_STR",
    "CAST_TO_BOOL",
    "MULTIPLY",
    "DIVIDE",
    "MODULO",
    "ADDITION",
    "SUBTRACTION",
    "BITWISE_SHIFT_LEFT",
    "BITWISE_SHIFT_RIGHT",
    "LESS_THAN",
    "LESS_OR_EQUAL",
    "GREATER_THAN",
    "GREATER_OR_EQUAL",
    "EQUAL_TO",
    "NOT_EQUAL_TO",
    "BITWISE_AND",
    "BITWISE_XOR",
    "BITWISE_OR",
    "LOGICAL_AND",
    "LOGICAL_OR",
    "ASSIGNMENT",
    "MULTIPLY_ASSIGNMENT",
    "DIVIDE_ASSIGNMENT",
    "MODULO_ASSIGNMENT",
    "ADD_ASSIGNMENT",
    "SUBTRACT_ASSIGNMENT",
    "POSTFIX_INC",
    "POSTFIX_DEC",
    "COMMA",
    "SEMICOLON",
    "END_OF_SCRIPT",
    "ERROR_TOKEN",
    "VAR",
    "CONST",
    "GLOBAL",
    "SCRIPT",
    "LOCAL",
    "BLOCK",
    "IF",
    "ELSE",
    "SWITCH",
    "CASE",
    "DEFAULT",
    "WHILE",
    "FOR",
    "BREAK",
    "CONTINUE",
    "RETURN",
    "INTEGER_VALUE",
    "DOUBLE_VALUE",
    "CHAR_VALUE",
    "BOOL_VALUE",
    "STRING_VALUE",
    "MULTI_LINE_COMMENT",
    "SINGLE_LINE_COMMENT",
    "VARIABLE_DEF",
    "VARIABLE_VALUE",
    "BRK",
    "FALSE",
    "TRUE",
    "LEND",
    "IF_COND_BEGIN",
    "IF_COND_END",
    "IF_BLOCK_BEGIN",
    "IF_BLOCK_END",
    "ELSE_BLOCK_BEGIN",
    "ELSE_BLOCK_END",
    "WHILE_COND_BEGIN",
    "WHILE_COND_END",
    "WHILE_BLOCK_BEGIN",
    "WHILE_BLOCK_END",
    "FOR_INIT_BEGIN",
    "FOR_INIT_END",
    "FOR_COND_BEGIN",
    "FOR_COND_END",
    "FOR_UPDATE_BEGIN",
    "FOR_UPDATE_END",
    "FOR_BLOCK_BEGIN",
    "FOR_BLOCK_END",
    "SWITCH_COND_BEGIN",
    "SWITCH_COND_END",
    "SWITCH_BEGIN",
    "SWITCH_END",
    "CASE_COND_BEGIN",
    "CASE_COND_END",
    "CASE_BLOCK_BEGIN",
    "CASE_BLOCK_END",
    "DEFAULT_BLOCK_BEGIN",
    "DEFAULT_BLOCK_END",
    "FUNCTION_CALL",
    "FUNCTION_DEF_BEGIN",
    "FUNCTION_DEF_END",
    "FUNCTION_PARAMETER",
    "PARAMETER_VALUE",
    "FUNCTION_CALL_BEGIN",
    "PARAM_BEGIN",
    "PARAM_END",
    "FUNCTION_CALL_END",
    "COLLECTION",
    "COLLECTION_BEGIN",
    "COLLECTION_END",
    "COLLECTION_VALUE",
    "KEY_BEGIN",
    "KEY_END",
    "STOP",
    "VARIABLE_ADDRESS",
    "COLLECTION_ADDRESS",
    "COLON",
    "INVALID_EXPRESSION"
};

/// \desc Displays the token type along with its index and _n.
/// \param index Value of the token.
/// \param type Type of the token.
void DisplayTokenAsText(int64_t index, TokenTypes type)
{
    const char *name = tokenNames[(int64_t)type & 0xFF];
    printf("%4d     0x%08x     %s", (int)index, type, name);
    printf("\n");
}

/// \desc command line arguments.
enum CMD_ARGS
{
      FileArg        = 0
    , DisplayArgZero = 1
    , DisplayArgOne  = 2
    , DisplayArgTwo  = 3
    , HelpArg        = 4
    , WarningsZero   = 5
    , WarningsOne    = 6
    , WarningsTwo    = 7
    , WarningsThree  = 8
    , LexerZero      = 9
    , LexerOne       = 10
    , ParserZero     = 11
    , ParserOne      = 12
    , ParserTwo      = 13
    , RunZero        = 14
    , RunOne         = 15
    , RunTwo         = 16
    , TraceZero      = 17
    , TraceOne       = 18
    , OutputFile     = 19
};

/// \desc parses the input string and returns the command line argument.
CMD_ARGS GetCommand(char *arg)
{
    size_t len = strlen(arg);
    if ( len < 1 )
    {
        return HelpArg;
    }
    if ( arg[0] != '-' )
    {
        return FileArg;
    }

    if ( len < 2 )
    {
        return HelpArg;
    }
    switch( arg[1] )
    {
        case 'd':
        case 'D':
            if (len < 3)
            {
                return DisplayArgOne;
            }
            switch (arg[2])
            {
                default:
                case '0':
                    return DisplayArgZero;
                case '1':
                    return DisplayArgOne;
                case '2':
                    return DisplayArgTwo;
            }
        case 'h':
        case 'H':
            return HelpArg;
        case 'l':
        case 'L':

            switch (arg[2])
            {
                default:
                case '0':
                    return LexerZero;
                case '1':
                    return LexerOne;
            }
        case 'o':
        case 'O':
            return OutputFile;
        case 'p':
        case 'P':
            if (len < 3)
            {
                return ParserZero;
            }
            switch (arg[2])
            {
                default:
                case '0':
                    return ParserZero;
                case '1':
                    return ParserOne;
                case '2':
                    return ParserTwo;
            }
        case 'r':
        case 'R':
            if (len < 3)
            {
                return RunZero;
            }
            switch (arg[2])
            {
                default:
                case '0':
                    return RunZero;
                case '1':
                    return RunOne;
                case '2':
                    return RunTwo;
            }
        case 't':
        case 'T':
            if (len < 3)
            {
                return TraceZero;
            }
            switch (arg[2])
            {
                default:
                case '0':
                    return TraceZero;
                case '1':
                    return TraceOne;
            }
        case 'w':
        case 'W':
            if (len < 3)
            {
                return WarningsThree;
            }
            switch (arg[2])
            {
                case '0':
                    return WarningsZero;
                case '1':
                    return WarningsOne;
                case '2':
                    return WarningsTwo;
                default:
                case '3':
                    return WarningsThree;
            }
        default:
            return HelpArg;
    }
}

/// \desc Displays the internal help page.
void Help()
{
    printf("--------------------------------------Help page--------------------------------------\n");
    printf("use:    dsl options files\n");
    printf("Note:   Command lines options are not case sensitive.\n");
    printf("Note:   Any command line entry that is not an option is considered to be a script file.\n");
    printf("--------------------------------------defaults---------------------------------------\n");
    printf("default -d0 -l0 -p0 -r0 -t0 -w3\n");
    printf("display off, Run time, lexer, parser, trace information are not displayed.\n");
    printf("Warning Treated as error.\n");
    printf("---------------------------------------options---------------------------------------\n");
    printf("-d0     Do not display the time the script takes to run. Default option.\n");
    printf("-d1     Display the time the script takes to run in seconds.\n");
    printf("-d2     Display the time the script takes to run in milliseconds.\n");
    printf("-h      This help page.\n");
    printf("-l0     Hide lexer token output. Default option.\n");
    printf("-l1     Show lexer token output.\n");
    printf("-p0     Hide parser output. Default option.\n");
    printf("-p1     Show parser generated code.\n");
    printf("-p2     Show parser generated code and expression parsing stack.\n");
    printf("-r0     Run lexer, parser, and CPU runtime. Default option.\n");
    printf("-r1     Run lexer and parser do not run CPU runtime.\n");
    printf("-r2     Run lexer do not run parser or CPU runtime.\n");
    printf("-s0     Strict mode off. Default option. Variables are auto cast and promoted to collections.\n");
    printf("-s1     Strict mode. Variables cannot be promoted to collections.\n");
    printf("-t0     No trace information. Default option.\n");
    printf("-t1     Run time trace information.\n");
    printf("-w0     Ignore all warnings.\n");
    printf("-w1     Ignore informational warnings.\n");
    printf("-w2     Show all warnings.\n");
    printf("-w3     Warnings are treated as errors. Default option.\n");
    printf("-o name Set the name of the generated IL code program file.");
}

/// \desc Gets the file name from the file path name.
void GetFileName(char *path, char *fileName)
{
    char *e = path + strlen(path);
    while( e > path )
    {
        if ( *e == '\\' || *e == '/' )
        {
            ++e;
            break;
        }
        --e;
    }

    strcpy(fileName, e);
}

/// \desc Gets the module name from the file name.
void GetModuleName(char *fileName, char *moduleName)
{
    strcpy(moduleName, fileName);
    char *p = strrchr(moduleName, '.');
    if ( p != nullptr )
    {
        *p = '\0';
    }
}

/// \desc Gets the current working directory. Works on linux, osx, and windows.
char *cwd()
{
    static char buffer[FILENAME_MAX];
    if (getcwd(buffer, FILENAME_MAX) != nullptr)
    {
    }
    return buffer;
}

void AddValue(List<Byte> &output, int64_t value, bool addComma)
{
    char tmp[256];

    sprintf(tmp, "%lld", value);
    char *p1 = tmp;

    while( *p1 )
    {
        output.push_back((Byte)*p1++);
    }

    if ( addComma )
    {
        output.push_back(',');
    }
}

void AddString(List<Byte> &output, const char *value, bool addComma)
{
    char *p1 = (char *)value;

    while( *p1 )
    {
        output.push_back((Byte)*p1++);
    }
    if ( addComma )
    {
        output.push_back(',');
    }
}

void Serialize(U8String &output, DslValue *dslValue)
{
    U8String tmp = {};

    if (dslValue->opcode != EFI || dslValue->moduleId > 0 )
    {
        tmp.printf((char *)"%d", dslValue->opcode);
        output.push_back(&tmp);
    }

    switch( dslValue->opcode )
    {
        case END: case NOP: case PSP: case RFE:
        case SLV: case SAV: case ADA: case SUA: case MUA: case DIA: case MOA: case EXP:
        case MUL: case DIV: case ADD: case SUB: case MOD: case XOR: case BND: case BOR:
        case SVL: case SVR: case TEQ: case TNE: case TGR: case TGE: case TLS: case TLE:
        case AND: case LOR: case INL: case DEL: case INC: case DEC: case NOT: case NEG:
        case CTI: case CTD: case CTC: case CTS: case CTB: case DFL: case RET:
            break;
        case PVA: case PSV: case PSL:
            tmp.printf((char *)",%d", dslValue->operand);
            output.push_back(&tmp);
            if ( debugMode )
            {
                tmp.printf((char *)",%s,%s", dslValue->variableName.cStr(),
                              dslValue->variableScriptName.cStr());
                output.push_back(&tmp);
            }
            break;
        case JBF:
        case PCV:
            tmp.printf((char *)",%d", dslValue->operand);
            output.push_back(&tmp);
            break;
        case JIF: case JIT:
            tmp.printf((char *)",%lld,%lld,%d", dslValue->operand, dslValue->location, dslValue->bValue);
            output.push_back(&tmp);
            break;
        case JMP: case JSR:
            tmp.printf((char *)",%d", dslValue->location);
            output.push_back(&tmp);
            break;
        case EFI:
            if ( dslValue->moduleId > 0 )
            {
                tmp.printf((char *)",%d,%d", dslValue->operand, dslValue->location);
                output.push_back(&tmp);
                if ( debugMode )
                {
                    tmp.printf((char *)",%s,%s", dslValue->variableName.cStr(),
                               dslValue->variableScriptName.cStr());
                    output.push_back(&tmp);
                }
            }
            break;
        case DEF:
        {
            U8String buffer = {};
            tmp.printf((char *)",%s", dslValue->GetValueAsString(&buffer, false, false));
            output.push_back(&tmp);
            if ( debugMode )
            {
                tmp.printf((char *)",%s,%s", dslValue->variableName.cStr(),
                              dslValue->variableScriptName.cStr());
                output.push_back(&tmp);
            }
            break;
        }
        case PSI:
        {
            U8String buffer = {};
            tmp.printf((char *)",%s",
                          dslValue->GetValueAsString(&buffer, false, false));
            output.push_back(&tmp);
            break;
        }
        case JTB:
            tmp.printf((char *)",%lld", dslValue->cases.Count());
            output.push_back(&tmp);
            for(int ii=0; ii<dslValue->cases.Count(); ++ii)
            {
                U8String buffer = {};
                tmp.printf((char *)",%c,%lld,%lld,%s",
                              dslValue->cases[ii]->type == DEFAULT ? 'D' :'C',
                              dslValue->cases[ii]->operand,
                              dslValue->cases[ii]->location,
                              dslValue->cases[ii]->GetValueAsString(&buffer,
                                     false, false), false);
                output.push_back(&tmp);
            }
            break;
        case DCS:
            tmp.printf((char *)",%lld,%lld", dslValue->operand, dslValue->iValue);
            output.push_back(&tmp);
            break;
        case CID:
            tmp.printf((char *)",%lld", dslValue->moduleId);
            output.push_back(&tmp);
            break;
    }
}

void WriteProgram(const char *file)
{
    U8String output;

    for(int ii=0; ii<program.Count(); ++ii)
    {
        Serialize(output, program[ii]);

        if ( ii + 1 < program.Count() )
        {
            output.push_back('\n');
        }
    }

    output.fwrite(file, true);
}

void ReadProgram(const char *file)
{

}

/// \desc Entry point for the compiler.
int main(int argc, char *argv[])
{
    printf("DSL Version 0.8.5 (Alpha)\n");
    printf("Working Folder %s\n", cwd());

    if ( argc < 2 )
    {
        Help();
        return -1;
    }

    List<U8String *> files;

    warningLevel = WarningLevel3;
    lexerInfoLevel = 0;
    parserInfoLevel = 0;
    traceInfoLevel = 0;

    int64_t runLevel = 0;
    int64_t displayLevel = 0;

    char outputFile[1024];
    strcpy(outputFile, "output.csv");

    for(int ii=1; ii<argc; ++ii)
    {
        switch(GetCommand(argv[ii]) )
        {
            case FileArg:
            {
                char szBuffer[1024];
                strcpy(szBuffer, argv[ii]);
                if ( strchr(szBuffer, '.') == nullptr)
                {
                    strcat(szBuffer, ".dsl");
                }
                printf(">>> %s >>>\n", szBuffer);
                files.push_back(new U8String(szBuffer));
                break;
            }
            case DisplayArgZero:
                displayLevel = 0;
                break;
            case DisplayArgOne:
                displayLevel = 1;
                break;
            case DisplayArgTwo:
                displayLevel = 2;
                break;
            case HelpArg:
                Help();
                return -1;
            case WarningsZero:
                warningLevel = WarningLevels::WarningLevel0;
                break;
            case WarningsOne:
                warningLevel = WarningLevels::WarningLevel1;
                break;
            case WarningsTwo:
                warningLevel = WarningLevels::WarningLevel2;
                break;
            case WarningsThree:
                warningLevel = WarningLevels::WarningLevel3;
                break;
            case LexerZero:
                lexerInfoLevel = 0;
                break;
            case LexerOne:
                lexerInfoLevel = 1;
                break;
            case OutputFile:
                ++ii;
                strcpy(outputFile, argv[ii]);
                break;
            case ParserZero:
                parserInfoLevel = 0;
                break;
            case ParserOne:
                parserInfoLevel = 1;
                break;
            case ParserTwo:
                parserInfoLevel = 2;
                break;
            case RunZero:
                runLevel = 0;
                break;
            case RunOne:
                runLevel = 1;
                break;
            case RunTwo:
                runLevel = 2;
                break;
            case TraceZero:
                traceInfoLevel = 0;
                break;
            case TraceOne:
                traceInfoLevel = 1;
                break;
        }
    }

    if ( files.Count() < 1 )
    {
        printf("No Files to compile.\n");
        return -2;
    }

    auto *lexer = new Lexer();
    Lexer::Initialize();

    List<char *> scripts;

    for(int ii=0; ii<files.Count(); ++ii)
    {
        FILE *fp = fopen(files[ii]->cStr(), "r");
        if( !fp )
        {
            PrintIssue(2800, true, true, "Can't open file %s", files[ii]->cStr());
            delete lexer;
            return -3;
        }

        fseek(fp, 0, SEEK_END);
        size_t len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        //printf("\n\n%lld\n\n", len);
        scripts.push_back((char *)calloc(1, len+1));
        fread(scripts[ii], 1, len, fp);
        fclose(fp);
        //printf("%s", scripts[0]);
        char fileName[512];
        GetFileName((char *)files[ii]->cStr(), fileName);
        char moduleName[512];
        GetModuleName(fileName, moduleName);
        Lexer::AddModule(moduleName, fileName, scripts[ii]);
    }

    if ( !lexer->Lex() )
    {
        delete lexer;
        return -3;
    }

    if ( errors > 0 || warnings > 0 )
    {
        delete lexer;
        return -3;
    }

    if ( lexerInfoLevel == 1 )
    {
        printf("\n<<<< Lexer Output >>>>\n");
        for(int64_t ii=0; ii<tokens.Count(); ++ii)
        {
            DisplayTokenAsText(ii, tokens[ii]->type);
        }
    }

    delete lexer;

    if ( runLevel < 2 )
    {
        auto *parser = new Parser();

        bool rc = parser->Parse();

        if ( parserInfoLevel > 0 )
        {
            printf("\n<<<< Parser Generated Code >>>>\n");
            CPU::DisplayASMCodeLines();
        }

        if ( !rc )
        {
            delete parser;
            return -3;
        }
        WriteProgram(outputFile);
    }

    if ( runLevel == 0 )
    {
        auto *cpu = new CPU();

        double start = (double)clock()/(double)CLOCKS_PER_SEC;
        cpu->Run();
        double end = (double)clock()/(double)CLOCKS_PER_SEC;
        switch(displayLevel)
        {
            default:
            case 0:
                break;
            case 1:
                printf("\nRun Time : %f\n", end - start);
                break;
            case 2:
                printf("\nRun Time : %f\n", (end - start) * 1000);
                break;
        }

        delete cpu;
    }

    printf("\n");

    return 0;
}