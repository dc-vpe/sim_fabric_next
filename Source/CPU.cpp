//before change to param pointer stack

//
// Created by krw10 on 9/7/2023.
//

#include "../Includes/cpu.h"
#include "../Includes/JsonParser.h"
#include <cmath>
#include <cctype>
#include <dirent.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
const char *OpCodeNames[] =
{
        "NOP",    //No operation  NOP
        "DEF",    //Define a variable
        "SAV",    //Save to variable
        "EXP",    //Exponent
        "MUL",    //multiply
        "DIV",    //divide
        "ADD",    //add
        "SUB",    //subtract
        "MOD",    //modulo
        "XOR",    //exclusive or
        "BND",    //bitwise and
        "BOR",    //bitwise or
        "INC",    //increment
        "DEC",    //decrement
        "NOT",    //invert
        "NEG",    //multiply by -1
        "SVL",    //shift left
        "SVR",    //shift right
        "CTI",    //convert to integer
        "CTD",    //convert to double
        "CTC",    //convert to char
        "CTS",    //convert to string
        "CTB",    //convert to bool
        "JMP",    //Jump always
        "JIF",    //Jump if bValue is false
        "JIT",    //Jump if bValue is true
        "JBF",    //Jump to native subroutine.
        "JSR",    //Jump to script subroutine.
        "RET",    //return to caller
        "PSI",    //Push value
        "PSV",    //push variable
        "END",    //End program
        "TEQ",    //test if equal
        "TNE",    //test if not equal
        "TGR",    //test if greater than
        "TGE",    //test if greater than or equal
        "TLS",    //test if less than
        "TLE",    //test if less than or equal
        "AND",    //logical and
        "LOR",    //logical or
        "JTB",    //Switch jump table, operand contains total entries.
        "DFL",    //Define local variable
        "PSL",    //Push local variable
        "SLV",    //Save local variable
        "PSP",    //Push parameter
        "INL",    //Increment local variable
        "DEL",    //Decrement local variable
        "PCV",    //Push collection value
        "PVA",    //Push variable address (works for variables and collections)
        "ADA",    //Add assign.
        "SUA",    //Subtract assign
        "MUA",    //Multiply Assign
        "DIA",    //Divide Assign
        "MOA",    //Modulo Assign
        "ERH",    //Error handler for script file
        "DCS"     //Save non-static result in a collection during definition
};

void CPU::DisplayASMCodeLines()
{
    int64_t addr = 0;

    while(addr < program.Count() )
    {
        addr = DisplayASMCodeLine(addr) + 1;
    }
}

int64_t CPU::DisplayASMCodeLine(int64_t addr, bool newline)
{
    DslValue *dslValue = program[addr];

    printf("%4.4lld\t%s", addr, OpCodeNames[dslValue->opcode]);
    switch(dslValue->opcode)
    {
        case DEF:
        {
            printf("\t%s\t;var %s", dslValue->variableName.cStr(), dslValue->variableName.cStr());
            break;
        }
        case DFL:
        {
            printf("\t%s\t;var local %s", dslValue->variableName.cStr(), dslValue->variableName.cStr());
            break;
        }
        case NOP: case XOR: case BND: case BOR: case NEG:
        case NOT: case SVL: case SVR: case CTI: case CTD: case CTC: case CTS:
        case CTB: case RET: case END:
        case EXP: case MUL: case DIV: case SUB: case MOD: case ADD:
        case TEQ:  case TNE: case TGR:  case TGE: case TLS:
        case TLE: case AND: case LOR: case ADA: case SUA: case MUA: case DIA: case MOA: case ERH:
            putchar('\t');
            break;
        case PVA: case DCS:
            printf("\t&%s", dslValue->variableName.cStr());
            break;
        case PCV:
            printf("\t%s[", dslValue->variableName.cStr());
            program[addr-2]->Print(true);
            printf("]");
            break;
        case DEC: case INC:
            printf("\t%s\t;var %s", dslValue->variableName.cStr(), dslValue->variableName.cStr());
            break;
        case INL: case DEL:
            printf("\t%s\t;param[BP+%lld]", dslValue->variableName.cStr(), dslValue->operand);
            break;
        case JTB:
        {
            int64_t startAddr = addr;
            printf("\t\t;switch\n");
            ++addr; //skip JTB
            for (int64_t ii = 0; ii < dslValue->operand; ++ii)
            {
                DslValue *caseValue = program[addr];
                printf("%4.4lld\tJMP\t%4.4lld\t;case ", addr, caseValue->location);
                caseValue->Print(true);
                printf("\n");
                ++addr;
            }
            printf("%4.4lld\tJMP\t%4.4lld;\t;default\n", startAddr, dslValue->location);
            return addr - 1;
        }
        case PSI:
            printf("\t");
            dslValue->Print(true);
            break;
        case PSV: case SAV: case SLV: case PSL: case PSP:
            printf("\t%s", dslValue->variableName.cStr());
            break;
        case JBF:
            printf("\t%s", standardFunctionNames[dslValue->operand]);
            break;
        case JIF: case JIT:
        case JMP: case JSR:
            printf("\t%4.4lld", dslValue->location);
            break;
    }
    if (newline)
    {
        printf("\n");
    }

    return addr;
}

/// \desc Raised an error event, currently only prints but will be changed as soon
///       as the eventing system is in place.
void CPU::Error(DslValue *dslError)
{
    dslError->Print(false);
}

bool CPU::IsMatch(u8chr ch, u8chr ex, bool caseLessCompare)
{
    switch( ex )
    {
        case 'C': return iscntrl((int)ch);
        case 'B': return isblank((int)ch);
        case 'S': return isspace((int)ch);
        case 'U': return isupper((int)ch);
        case 'u': return islower((int)ch);
        case 'A': return isalpha((int)ch);
        case 'D': return isdigit((int)ch);
        case 'N': return isalnum((int)ch);
        case 'P': return ispunct((int)ch);
        case 'G': return isgraph((int)ch);
        case 'p': return isprint((int)ch);
        case 'X': return isxdigit((int)ch);
        case '?': return true;
        case '%': return ch == '%';
        case U8_NULL_CHR: return true;
        default: return ch == ex;
    }
}

u8chr CPU::GetExChar(U8String *expression, int64_t &offset)
{
    u8chr ex = expression->get(offset++);
    if ( ex == '%' )
    {
        ex = expression->get(offset++);
    }

    return ex;
}

int64_t CPU::Find(U8String *search, U8String *expression, int64_t start)
{
    if ( start < 0 || start >= search->Count() )
    {
        return -1;
    }

    int64_t offset = 0;
    bool caselessCompare = false;
    int64_t location = -1;
    int64_t current = start;
    auto end = (int64_t)search->Count();
    while( current < end)
    {
        u8chr ch = search->get(current++);
        u8chr ex = GetExChar(expression, offset);
        if ( IsMatch(ch, ex, caselessCompare) )
        {
            location = current-1;
            int64_t pos = current;
            while( offset < expression->Count() )
            {
                ch = search->get(pos++);
                ex = GetExChar(expression, offset);
                if ( ex == U8_NULL_CHR )
                {
                    return location;
                }
                if ( ch == U8_NULL_CHR )
                {
                    return -1;
                }
                if (IsMatch(ch, ex, caselessCompare))
                {
                    continue;
                }
                location = -1;
                offset = 0;
                break;
            }
            if ( location > -1 )
            {
                return location;
            }
        }
        else
        {
            location = -1;
            offset = 0;
        }
    }

    return -1;
}

/// \desc Gets the total parameters sent to a function.
/// \return The total parameters sent to a function.
/// \remark The last value pushed onto the param stack is always the
///         number of parameters passed to the function. In the DSL
///         all functions accept variable number of parameters.
int64_t CPU::GetTotalParams()
{
    return params[top].iValue;
}

/// \desc Returns the result of a built in function. Functions always return a result.
/// The result is placed on the top of the stack.
void CPU::ReturnResult(int64_t totalParams, DslValue *returnValue)
{
    top -= totalParams + 1;
    params[++top].LiteCopy(returnValue);
}

/// \desc Gets a single parameter.
/// \param totalParams total parameters this is the same value provided to Get Total Parameters.
/// \param index Index of the parameter to get. The first parameter is param index 0.
/// \return Pointer to the parameter.
DslValue *CPU::GetParam(int64_t totalParams, int64_t index)
{
    return &params[(top-totalParams) + index];
}

/// \desc Reads a file current using the local file system. The file is returned
///       int the A dsl value. In case of an error the A dsl value will contain
///       an error.
/// \param file location to read the file from.
/// \param output U8String to write the contents of the file to.
/// \return True if successful, else false if an error occurs.
bool CPU::ReadFile(U8String *file, U8String *output)
{
    FILE *fp = fopen(file->cStr(), "r");
    if (fp == nullptr )
    {
        output->CopyFromCString("FILE: ");
        output->Append(file->cStr());
        output->Append(" error ");
        output->Append(strerror(errno));
        output->Append("\n");
        fclose(fp);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *)calloc(1, len+1);
    fread(buffer, 1, len, fp);
    fclose(fp);
    output->CopyFromCString(buffer);

    return true;
}

void CPU::Sub(U8String *search, U8String *result, int64_t start, int64_t length)
{
    if ( start >= 0 && start < search->Count() )
    {
        for(int64_t ii=0; ii<length; ++ii)
        {
            u8chr ch = search->get(start+ii);
            if ( ch == U8_NULL_CHR )
            {
                break;
            }
            result->push_back(ch);
        }
    }

}

/// \desc Gets the character length of the expression.
int64_t CPU::ExpressionLength(U8String *expression)
{
    int64_t length = 0;
    u8chr ex = ' ';
    int64_t offset = 0;
    while( ex != U8_NULL_CHR )
    {
        ex = GetExChar(expression, offset);
        if ( ex != U8_NULL_CHR )
        {
            ++length;
        }
    }

    return length;
}

// Region builtin callable functions.
void CPU::pfn_string_find()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    auto *param2 = GetParam(totalParams, 0);
    auto *param3 = GetParam(totalParams, 0);

    //string to search
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    //expression
    param2->Convert(STRING_VALUE);
    U8String expression;
    expression.CopyFrom(&param2->sValue);

    //start location
    param3->Convert(INTEGER_VALUE);
    int64_t start = param3->iValue;

    A->type = INTEGER_VALUE;
    A->iValue = Find(&search, &expression, start);

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_len()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);

    param1->Convert(STRING_VALUE);
    A->iValue = (int64_t)param1->sValue.Count();

    ReturnResult(totalParams, A);
}
void CPU::pfn_string_sub()
{
    auto totalParams = GetTotalParams();


    auto *param1 = GetParam(totalParams, 0);
    auto *param2= GetParam(totalParams, 1);
    auto *param3= GetParam(totalParams, 2);

    A->type = STRING_VALUE;
    A->sValue.Clear();

    //string to search
    param1->Convert(STRING_VALUE);
    //start
    param2->Convert(INTEGER_VALUE);
    //length
    param3->Convert(INTEGER_VALUE);
    Sub(&param1->sValue, &A->sValue, param2->iValue, param3->iValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_replace()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    auto *param2= GetParam(totalParams, 1);
    auto *param3= GetParam(totalParams, 2);

    //string to search
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    //expression
    param2->Convert(STRING_VALUE);
    U8String expression;
    expression.CopyFrom(&param2->sValue);

    //replace
    param3->Convert(STRING_VALUE);
    U8String replace;
    replace.CopyFrom(&param3->sValue);

    int64_t start = 0;
    int64_t location = 0;
    A->type = STRING_VALUE;
    A->sValue.Clear();

    while( location != -1 )
    {
        location = Find(&search, &expression, start);
        if ( location != -1 )
        {
            U8String tmp;
            int64_t length = ExpressionLength(&expression);
            Sub(&search, &tmp, start, location-start);
            A->sValue.Append(&tmp);
            A->sValue.Append(&replace);
            location += length - 1; //skip the replaced expression.
            start = location;
        }
        else
        {
            U8String tmp;
            Sub(&search, &tmp, start, (int64_t)search.Count());
            A->sValue.Append(&tmp);
        }
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_tolower()
{
    auto totalParams = GetTotalParams();

    //string to search
    auto  *param1 = GetParam(totalParams, 0);
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    A->type = STRING_VALUE;
    A->sValue.Clear();

    for(int64_t ii=0; ii<search.Count(); ++ii)
    {
        u8chr ch = search.get(ii);
        ch = tolower((int)ch);
        A->sValue.push_back(ch);
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_toupper()
{
    auto totalParams = GetTotalParams();

    //string to search
    auto *param1 = GetParam(totalParams, 0);
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    A->type = STRING_VALUE;
    A->sValue.Clear();

    for(int64_t ii=0; ii<search.Count(); ++ii)
    {
        u8chr ch = search.get(ii);
        ch = toupper((int)ch);
        A->sValue.push_back(ch);
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_trimEnd()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    auto *param2= GetParam(totalParams, 1);

    //string to search
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    //expression
    param2->Convert(STRING_VALUE);
    U8String expression;
    expression.CopyFrom(&param2->sValue);

    int64_t offset = 0;
    u8chr ex = GetExChar(&expression, offset);
    bool copy = false;
    A->type = STRING_VALUE;
    A->sValue.Clear();

    for(int64_t ii=(int64_t)search.Count()-1; ii>=0; --ii)
    {
        u8chr ch = search.get(ii);
        if ( copy )
        {
            A->sValue.push_back(ch);
            continue;
        }
        if (IsMatch(ch, ex, false))
        {
            continue;
        }
        copy = true;
        A->sValue.push_back(ch);
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_trimStart()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    auto *param2= GetParam(totalParams, 1);

    //string to search
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    //expression
    param2->Convert(STRING_VALUE);
    U8String expression;
    expression.CopyFrom(&param2->sValue);

    int64_t offset = 0;
    u8chr ex = GetExChar(&expression, offset);
    bool copy = false;
    A->type = STRING_VALUE;
    A->sValue.Clear();

    for(int64_t ii=0; ii<search.Count(); ++ii)
    {
        u8chr ch = search.get(ii);
        if ( copy )
        {
            A->sValue.push_back(ch);
            continue;
        }
        if (IsMatch(ch, ex, false))
        {
            continue;
        }
        copy = true;
        A->sValue.push_back(ch);
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_toCollection()
{
    auto totalParams = GetTotalParams();

    //String to convert to a collection.
    auto *param1 = GetParam(totalParams, 0);
    param1->Convert(STRING_VALUE);
    U8String jsonString;
    jsonString.CopyFrom(&param1->sValue);

    JsonParser jp;
    auto *json = jp.From(&params[top].variableScriptName, &jsonString);
    if ( json->type == ERROR_TOKEN )
    {
        json->type = STRING_VALUE;
        Error(json);
    }

    A->SAV(json);

    ReturnResult(totalParams, A);
}

void CPU::pfn_string_fromCollection()
{
    auto totalParams = GetTotalParams();

    A->type = STRING_VALUE;
    GetParam(totalParams, 0)->AppendAsJsonText(&A->sValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_abs()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = abs(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_acos()
{
    auto totalParams = GetTotalParams();
    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = acos(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_asin()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = asin(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_atan()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = atan(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_atan2()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    auto *param2= GetParam(totalParams, 1);

    param1->Convert(DOUBLE_VALUE);
    param2->Convert(DOUBLE_VALUE);
    A->type = DOUBLE_VALUE;
    A->dValue = atan2(param1->dValue, param2->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_cos()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = cos(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_sin()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = sin(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_tan()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = tan(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_cosh()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = cosh(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_sinh()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = sinh(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_tanh()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = tanh(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_exp()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = exp(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_log()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = log(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_log10()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = log10(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_sqrt()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = sqrt(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_ceil()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = ceil(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_fabs()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = fabs(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_floor()
{
    auto totalParams = GetTotalParams();

    A->type = DOUBLE_VALUE;
    auto *param = GetParam(totalParams, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = floor(param->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_fmod()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    auto *param2= GetParam(totalParams, 1);
    auto *param3= GetParam(totalParams, 2);

    param1->Convert(DOUBLE_VALUE);
    param2->Convert(DOUBLE_VALUE);
    A->type = DOUBLE_VALUE;
    A->dValue = fmod(param1->dValue, param2->dValue);

    ReturnResult(totalParams, A);
}

void CPU::pfn_print()
{
    DslValue dslValue;

    auto totalParams = GetTotalParams();

    for(int64_t ii=0; ii<totalParams; ++ii)
    {
        GetParam(totalParams, ii)->Print(false);
    }

    A->type = INTEGER_VALUE;
    A->iValue = 0;

    ReturnResult(totalParams, A);
}

void CPU::pfn_printf()
{
    pfn_print();
}

void CPU::pfn_input()
{
    auto totalParams = GetTotalParams();

    printf(">");
    char szBuffer[1024];
    fgets(szBuffer, 1024, stdin);
    char *p = szBuffer+strlen(szBuffer)-1;
    if ( *p == '\r' || *p == '\n' )
    {
        *p = '\0';
    }

    A->type = STRING_VALUE;
    A->sValue.CopyFromCString(szBuffer);

    ReturnResult(totalParams, A);
}

void CPU::pfn_read()
{
    auto totalParams = GetTotalParams();

    auto *param1 = GetParam(totalParams, 0);
    param1->Convert(STRING_VALUE);

    U8String output = {};
    if ( !ReadFile(&param1->sValue, &output) )
    {
        Error(A);
    }
    else
    {
        JsonParser jp;
        auto *json = jp.From(&param1->sValue, &output);
        if ( json->type == ERROR_TOKEN )
        {
            json->type = STRING_VALUE;
            Error(json);
        }
        A->SAV(json);
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_write()
{
    auto totalParams = GetTotalParams();

    A->type = STRING_VALUE;
    A->sValue.Clear();

    auto *param1 = GetParam(totalParams, 0);

    U8String fileName;
    fileName.CopyFrom(&param1->sValue);

    for(int64_t ii=1; ii<totalParams; ++ii)
    {
        GetParam(totalParams, ii)->AppendAsJsonText(&A->sValue);
    }

    FILE *fp = fopen(fileName.cStr(), "w+");
    if (fp == nullptr )
    {
        A->sValue.CopyFromCString("Failed");
        A->sValue.Append(" FILE = ");
        A->sValue.Append(&fileName);
        A->sValue.Append(" error ");
        A->sValue.Append(strerror(errno));
        A->sValue.Append("\n");
        A->type = STRING_VALUE;
        fclose(fp);
    }
    else
    {
        fwrite(A->sValue.cStr(), 1, A->sValue.Count(), fp);
        fclose(fp);
        A->sValue.CopyFromCString("Success");
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_files()
{
    auto totalParams= GetTotalParams();

    bool open = false;
    if ( totalParams >= 2 )
    {
        auto *tmp = GetParam(totalParams, 1);
        tmp->Convert(BOOL_VALUE);
        open = tmp->bValue;
    }

    auto *param1 = GetParam(totalParams, 0);

    param1->Convert(STRING_VALUE);
    DIR *dir = opendir(param1->sValue.cStr());
    auto *files = new DslValue();
    if ( dir == nullptr )
    {
        files->type = STRING_VALUE;
        files->sValue.CopyFromCString("Failed, directory does not exit.");
    }
    else
    {
        dirent *dp;
        files->type = COLLECTION;
        files->indexes.Clear();

        files->variableName.Clear();
        files->variableName.Append(dir->dd_name);

        while( (dp = readdir(dir)) != nullptr )
        {
            auto *key = new U8String(dp->d_name);
            auto *dslValue = new DslValue();
            dslValue->type = INTEGER_VALUE;
            dslValue->iValue = dp->d_namlen;
            files->indexes.Set(key, dslValue);
        }
        closedir(dir);
    }

    if ( open )
    {
        List<KeyData *> keyData = files->indexes.GetKeyData();
        for (int ii = 0; ii < keyData.Count(); ++ii)
        {
            auto *file = (U8String *) &((DslValue *)keyData[ii]->Data())->sValue;
            if ( file->Count() > 2 )
            {
                U8String text = {};
                if ( !ReadFile(file, &text) )
                {
                    A->type = STRING_VALUE;
                    A->sValue.CopyFrom(&text);
                    Error(A);
                    ReturnResult(totalParams, A);
                    return;
                }
                if (file->EndsWith(".json", false))
                {
                    JsonParser jp;
                    auto *json = jp.From(file, &text);
                    if ( json->type == ERROR_TOKEN )
                    {
                        json->type = STRING_VALUE;
                        Error(json);
                        ReturnResult(totalParams, json);
                        return;
                    }
                    files->indexes.Set(keyData[ii]->Key(), json);
                }
                else
                {
                    auto *value = new DslValue();
                    value->type = STRING_VALUE;
                    value->sValue.CopyFrom(&text);

                    files->indexes.Set(keyData[ii]->Key(), value);
                }
            }
        }
    }

    ReturnResult(totalParams, A);
}

void CPU::pfn_delete()
{
    auto totalParams= GetTotalParams();

    A->type = STRING_VALUE;
    A->sValue.CopyFromCString("Not Implemented");
    Error(A);

    ReturnResult(totalParams, A);
}

//Random number generator constants.
static uint64_t       state      = 0x4d595df4d0f33173;		// Or something seed-dependent
static uint64_t const multiplier = 6364136223846793005u;
static uint64_t const increment  = 1442692040788163497u;	// Or an arbitrary odd constant

static uint32_t RotateRight32(uint32_t x, unsigned r)
{
    return x >> r | x << (-r & 31);
}

void CPU::pfn_random()
{
    uint64_t x = state;
    auto count = (uint64_t)(x >> 59); // 59 = 64 - 5

    state = x * multiplier + increment;
    x ^= x >> 18; // 18 = (64 - 27)/2
    int64_t m = RotateRight32((uint32_t)(x >> 27), count); // 27 = 32 - 5

    auto totalParams = params[top].iValue;
    params[top-totalParams].Convert(INTEGER_VALUE);
    int64_t low = params[top-totalParams].iValue;

    params[top-totalParams+1].Convert(INTEGER_VALUE);
    int64_t high = params[top-totalParams+1].iValue;

    int64_t delta = (high + 1) - low;

    A->type = INTEGER_VALUE;
    A->iValue = low + (m % delta);
    params[++top].LiteCopy(A);

    top -= totalParams + 1;
}

void CPU::pfn_seed()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(INTEGER_VALUE);
    state = params[top-totalParams].iValue;

    top -= totalParams;
}

CPU::method_function builtInMethods[] =
 {
          &CPU::pfn_string_find,
          &CPU::pfn_string_len,
          &CPU::pfn_string_sub,
          &CPU::pfn_string_replace,
          &CPU::pfn_string_tolower,
          &CPU::pfn_string_toupper,
          &CPU::pfn_string_trimEnd,
          &CPU::pfn_string_trimStart,
          &CPU::pfn_string_toCollection,
          &CPU::pfn_string_fromCollection,
         &CPU::pfn_abs,
         &CPU::pfn_acos,
         &CPU::pfn_asin,
         &CPU::pfn_atan,
         &CPU::pfn_atan2,
         &CPU::pfn_cos,
         &CPU::pfn_sin,
         &CPU::pfn_tan,
         &CPU::pfn_cosh,
         &CPU::pfn_sinh,
         &CPU::pfn_tanh,
         &CPU::pfn_exp,
         &CPU::pfn_log,
         &CPU::pfn_log10,
         &CPU::pfn_sqrt,
         &CPU::pfn_ceil,
         &CPU::pfn_fabs,
         &CPU::pfn_floor,
         &CPU::pfn_fmod,
         &CPU::pfn_print,
         &CPU::pfn_printf,
         &CPU::pfn_input,
         &CPU::pfn_read,
         &CPU::pfn_write,
         &CPU::pfn_files,
         &CPU::pfn_delete,
         &CPU::pfn_random,
         &CPU::pfn_seed
 };

void CPU::JumpToBuiltInFunctionNoTrace(DslValue *dslValue)
{
    (this->*builtInMethods[dslValue->operand])();
}

void CPU::JumpToBuiltInFunctionTrace(DslValue *dslValue)
{
    DslValue dslValue1;

    if ( traceInfoLevel == 1)
    {
        printf(";top = %lld\t%lld", top, params[top].iValue);
        for(int64_t ii=top-params[top].iValue; ii<top; ++ii)
        {
            putchar(',');
            params[ii].Print(true);
        }
        printf("\n>>>> Print: >>>>\n");
    }

    (this->*builtInMethods[dslValue->operand])();
}

/// \desc calls a compiled script function.
/// remarks
///locals are set in order encountered after parameters and start at operand == 0 and
///each local adds 1 to the operand.
void CPU::JumpToSubroutineNoTrace(DslValue *dslValue)
{
    auto totalParams = params[top].iValue;

    int64_t pcReturn = PC;
    int64_t saved = BP;
    BP = top - totalParams;
    PC = dslValue->location;
    Run();
    A->SAV(&params[top]);
    --top;
    BP = saved;
    top -= totalParams + 1;
    params[++top].SAV(A);
    PC = pcReturn;
}

/// \desc calls a compiled script function.
/// remarks
///locals are set in order encountered after parameters and start at operand == 0 and
///each local adds 1 to the operand.
void CPU::JumpToSubroutineTrace(DslValue *dslValue)
{
    printf(";top = %lld, params = %lld, location = %4.4lld, ", top, params[top].iValue, dslValue->location);
    for(int64_t ii=top-params[top].iValue; ii<top; ++ii)
    {
        params[ii].Print(true);
        putchar('\t');
    }

    auto totalParams = params[top].iValue;

    int64_t pcReturn = PC;
    int64_t saved = BP;
    BP = top - totalParams;
    PC = dslValue->location;
    Run();
    A->SAV(&params[top]);
    --top;
    BP = saved;
    top -= totalParams + 1;
    params[++top].SAV(A);
    PC = pcReturn;
}

void CPU::ProcessJumpTableNoTrace(DslValue *dslValue)
{
    for(int64_t ii=0; ii<dslValue->operand; ++ii)
    {
        if ( params[top].IsEqual(program[PC + ii]) )
        {
            PC = program[PC + ii]->location;
            --top;
            return;
        }
    }

    //jump to default or out of switch.
    PC = dslValue->location;
    --top;
}

void CPU::ProcessJumpTableTrace(DslValue *dslValue)
{
    for(int64_t ii=0; ii<dslValue->operand; ++ii)
    {
        putchar('\t');
        putchar(';');
        printf("if ( ");
        params[top].Print(true);
        printf(" == ");
        program[PC + ii]->Print(true);
        printf(") ");
        printf("%lld", program[PC + ii]->location);

        if ( params[top].IsEqual(program[PC + ii]) )
        {
            PC = program[PC + ii]->location;
            --top;
            return;
        }
    }

    for(int64_t ii=0; ii<dslValue->operand; ++ii)
    {
        if ( params[top].IsEqual(program[PC + ii]) )
        {
            PC = program[PC + ii]->location;
            --top;
            return;
        }
    }

    //jump to default or out of switch.
    PC = dslValue->location;
    --top;
}

void CPU::OutputValues(DslValue *left, DslValue *right) const
{
    printf(";top = %lld\t", top);
    left->Print(true);
    printf("\t");
    right->Print(true);
}

void CPU::PrintResult(DslValue *result)
{
    printf(" = ");
    result->Print(true);
}

void CPU::Run()
{
    if ( traceInfoLevel == 0 )
    {
        RunNoTrace();
        return;
    }

    RunTrace();
}

void CPU::ExtendCollection(DslValue *collection, int64_t newEnd)
{
    for(int64_t ii=collection->indexes.keys.Count(); ii<=newEnd; ++ii)
    {
        U8String key;
        key.Clear();
        key.push_back(&collection->variableScriptName);
        key.push_back('.');
        key.Append(ii);
        collection->indexes.Set(new U8String(&key), new DslValue());
    }
}

void CPU::SetCollectionElementDirect(DslValue *dslValue)
{
    auto *var = program[dslValue->operand];
    U8String *key = var->indexes.keys[dslValue->iValue];
    auto *collection = ((DslValue *)var->indexes.Get(key)->Data());
    collection->SAV(&params[top]);
    --top;
}

DslValue *CPU::GetCollectionElement(DslValue *dslValue)
{
    auto     totalParams = params[top].iValue; //subtract out the param count.
    DslValue *collection = dslValue;

    U8String *key;
    for(int64_t ii=top-totalParams; ii<top; ++ii)
    {
        if ( params[ii].type != STRING_VALUE )
        {
            params[ii].Convert(INTEGER_VALUE);
            if ( params[ii].iValue >= collection->indexes.keys.Count() )
            {
                ExtendCollection(collection, params[ii].iValue);
            }
            key = collection->indexes.keys[params[ii].iValue];
        }
        else
        {
            key = &params[ii].sValue;
        }
        if( collection->indexes.Exists(key) )
        {
            collection = ((DslValue *)collection->indexes.Get(key)->Data());
        }
        else
        {
            collection->indexes.Set(new U8String(key), new DslValue());
            collection = ((DslValue *)collection->indexes.Get(key)->Data());
        }
    }

    top -= totalParams + 1;
    collection->elementAddress = collection;

    return collection;
}

void CPU::PushVariableAddress(DslValue *variable)
{
    DslValue *value = variable;

    if (variable->type == COLLECTION )
    {
        value = GetCollectionElement(variable);
    }
    params[++top].elementAddress = value;
}


void CPU::RunNoTrace()
{
    while(PC < program.Count() )
    {
        DslValue *dslValue = program[PC++];
        switch( dslValue->opcode )
        {
            case DEF: case NOP: case END: case PSP: case ERH:
                break;
            case SLV:
                params[BP+params[top - 1].operand].SAV(&params[top]);
                top--;
                break;
            case SAV:
                params[top-1].elementAddress->SAV(&params[top]);
                top--;
                top--;
                break;
            case ADA:
                params[top-1].elementAddress->ADD(&params[top]);
                --top;
                break;
            case SUA:
                params[top-1].elementAddress->SUB(&params[top]);
                --top;
                break;
            case MUA:
                params[top-1].elementAddress->MUL(&params[top]);
                --top;
                break;
                break;
            case DIA:
                params[top-1].elementAddress->DIV(&params[top]);
                --top;
                break;
                break;
            case MOA:
                params[top-1].elementAddress->MOD(&params[top]);
                --top;
                break;
            case DCS:
                SetCollectionElementDirect(dslValue);
                break;
            case PVA:
                PushVariableAddress(program[dslValue->operand]);
                break;
            case PCV:
                dslValue = GetCollectionElement(program[dslValue->operand]);
                params[++top].LiteCopy(dslValue);
                params[top].elementAddress = dslValue;
                break;
            case EXP:
                params[top - 1].EXP(&params[top]);
                top--;
                break;
            case MUL:
                params[top - 1].MUL(&params[top]);
                top--;
                break;
            case DIV:
                params[top - 1].DIV(&params[top]);
                top--;
                break;
            case ADD:
                params[top - 1].ADD(&params[top]);
                top--;
                break;
            case SUB:
                params[top - 1].SUB(&params[top]); top--;
                break;
            case MOD:
                params[top - 1].MOD(&params[top]); top--;
                break;
            case XOR:
                params[top - 1].XOR(&params[top]); top--;
                break;
            case BND:
                params[top - 1].BND(&params[top]); top--;
                break;
            case BOR:
                params[top - 1].BOR(&params[top]); top--;
                break;
            case SVL:
                params[top - 1].SVL(&params[top]); top--;
                break;
            case SVR:
                params[top - 1].SVR(&params[top]); top--;
                break;
            case TEQ:
                params[top - 1].TEQ(&params[top]); top--;
                break;
            case TNE:
                params[top - 1].TNE(&params[top]); top--;
                break;
            case TGR:
                params[top - 1].TGR(&params[top]); top--;
                break;
            case TGE:
                params[top - 1].TGE(&params[top]); top--;
                break;
            case TLS:
                params[top - 1].TLS(&params[top]); top--;
                break;
            case TLE:
                params[top - 1].TLE(&params[top]); top--;
                break;
            case AND:
                params[top - 1].AND(&params[top]); top--;
                break;
            case LOR:
                params[top - 1].LOR(&params[top]); top--;
                break;
            case INL:
                params[BP+dslValue->operand].INC();
                break;
            case DEL:
                params[BP+dslValue->operand].DEC();
                break;
            case INC:
                program[dslValue->operand]->INC();
                break;
            case DEC:
                program[dslValue->operand]->DEC();
                break;
            case NOT:
                params[top].NOT();
                break;
            case NEG:
                params[top].NEG();
                break;
            case CTI:
                params[top].Convert(INTEGER_VALUE);
                break;
            case CTD:
                params[top].Convert(DOUBLE_VALUE);
                break;
            case CTC:
                params[top].Convert(CHAR_VALUE);
                break;
            case CTS:
                params[top].Convert(STRING_VALUE);
                break;
            case CTB:
                params[top].Convert(BOOL_VALUE);
                break;
            case JIF:
                PC = (params[top].bValue) ? PC : dslValue->location; top--;
                break;
            case JIT:
                PC = (params[top].bValue) ? dslValue->location : PC; top--;
                break;
            case JMP:
                PC = dslValue->location;
                break;
            case JBF:
                JumpToBuiltInFunctionNoTrace(dslValue);
                break;
            case PSI:
                params[++top].LiteCopy(dslValue);
                break;
            case PSV:
                params[++top].LiteCopy(program[dslValue->operand]);
                break;
            case DFL:
                params.push_back(DslValue(DFL, params.Count()));
                break;
            case PSL:
                params[++top].LiteCopy(&params[BP+dslValue->operand]);
                params[top].operand = dslValue->operand;
                break;
            case RET:
                return;
            case JSR:
                JumpToSubroutineNoTrace(dslValue);
                break;
            case JTB:
                ProcessJumpTableNoTrace(dslValue);
                break;
        }
    }
}

void CPU::RunTrace()
{
    putchar('\n');
    while(PC < program.Count() )
    {
        DisplayASMCodeLine(PC, false);
        putchar('\t');
        DslValue *dslValue = program[PC++];
        switch( dslValue->opcode )
        {
            case DEF:
                printf(";top = %lld;\tvar %s", top, dslValue->variableName.cStr());
                break;
            case NOP: case END: case PSP: case ERH:
                break;
            case SLV:
            {
                OutputValues(&params[BP+params[top - 1].operand], &params[top]);
                params[BP+params[top - 1].operand].SAV(&params[top]);
                top--;
                PrintResult(&params[top]);
                break;
            }
            case SAV:
                OutputValues(program[dslValue->operand], &params[top]);
                params[top-1].elementAddress->SAV(&params[top]);
                top--;
                top--;
                PrintResult(program[dslValue->operand]);
                break;
            case ADA:
                OutputValues(&params[top - 1], &params[top]);
                params[top+1].SAV(&params[top]);
                ++top;
                PrintResult(&params[top]);
                break;
            case SUA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].elementAddress->SUB(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case MUA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].elementAddress->MUL(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case DIA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].elementAddress->DIV(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case MOA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].elementAddress->MOD(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case DCS:
                break;
            case PVA:
                PrintResult(&params[top]);
                PushVariableAddress(program[dslValue->operand]);
                PrintResult(&params[top]);
                break;
            case PCV:
                PrintResult(&params[top]);
                dslValue = GetCollectionElement(dslValue);
                params[++top].LiteCopy(dslValue);
                PrintResult(&params[top]);
                break;
            case EXP:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].EXP(&params[top]);
                top--;
                PrintResult(&params[top]);
                break;
            case MUL:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].MUL(&params[top]);
                top--;
                PrintResult(&params[top]);
                break;
            case DIV:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].DIV(&params[top]);
                top--;
                PrintResult(&params[top]);
                break;
            case ADD:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].ADD(&params[top]);
                top--;
                PrintResult(&params[top]);
                break;
            case SUB:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].SUB(&params[top]); top--;
                break;
            case MOD:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].MOD(&params[top]); top--;
                break;
            case XOR:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].XOR(&params[top]); top--;
                break;
            case BND:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].BND(&params[top]); top--;
                break;
            case BOR:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].BOR(&params[top]); top--;
                break;
            case SVL:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].SVL(&params[top]); top--;
                break;
            case SVR:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].SVR(&params[top]); top--;
                break;
            case TEQ:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].TEQ(&params[top]); top--;
                break;
            case TNE:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].TNE(&params[top]); top--;
                break;
            case TGR:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].TGR(&params[top]); top--;
                break;
            case TGE:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].TGE(&params[top]); top--;
                break;
            case TLS:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].TLS(&params[top]); top--;
                break;
            case TLE:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].TLE(&params[top]); top--;
                break;
            case AND:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].AND(&params[top]); top--;
                break;
            case LOR:
                OutputValues(&params[top - 1], &params[top]);
                params[top - 1].LOR(&params[top]); top--;
                break;
            case INL:
                printf(";top = %lld\t", top);
                params[BP+dslValue->operand].Print(true);
                params[BP+dslValue->operand].INC();
                printf(";top = %lld\t", top);
                params[BP+dslValue->operand].Print(true);
                break;
            case DEL:
                printf(";top = %lld\t", top);
                params[BP+dslValue->operand].Print(true);
                params[BP+dslValue->operand].DEC();
                printf(";top = %lld\t", top);
                params[BP+dslValue->operand].Print(true);
                break;
            case INC:
                printf(";top = %lld\t", top);
                program[dslValue->operand]->Print(true);
                program[dslValue->operand]->INC();
                printf(";top = %lld\t", top);
                program[dslValue->operand]->Print(true);
                break;
            case DEC:
                printf(";top = %lld\t", top);
                program[dslValue->operand]->Print(true);
                program[dslValue->operand]->DEC();
                printf(";top = %lld\t", top);
                program[dslValue->operand]->Print(true);
                break;
            case NOT:
                OutputValues(&params[top - 1], &params[top]);
                params[top].NOT();
                break;
            case NEG:
                OutputValues(&params[top - 1], &params[top]);
                params[top].NEG();
                break;
            case CTI:
                OutputValues(&params[top - 1], &params[top]);
                params[top].Convert(INTEGER_VALUE);
                break;
            case CTD:
                OutputValues(&params[top - 1], &params[top]);
                params[top].Convert(DOUBLE_VALUE);
                break;
            case CTC:
                OutputValues(&params[top - 1], &params[top]);
                params[top].Convert(CHAR_VALUE);
                break;
            case CTS:
                OutputValues(&params[top - 1], &params[top]);
                params[top].Convert(STRING_VALUE);
                break;
            case CTB:
                OutputValues(&params[top - 1], &params[top]);
                params[top].Convert(BOOL_VALUE);
                break;
            case JIF:
                OutputValues(&params[top - 1], &params[top]);
                PC = (params[top].bValue) ? PC : dslValue->location; top--;
                break;
            case JIT:
                OutputValues(&params[top - 1], &params[top]);
                PC = (params[top].bValue) ? dslValue->location : PC; top--;
                break;
            case JMP:
                OutputValues(&params[top - 1], &params[top]);
                PC = dslValue->location;
                break;
            case JBF:
                JumpToBuiltInFunctionTrace(dslValue);
                break;
            case PSI:
                OutputValues(&params[top + 1], dslValue);
                params[++top].LiteCopy(dslValue);
                break;
            case PSV:
                OutputValues(&params[top + 1], program[dslValue->operand]);
                params[++top].LiteCopy(program[dslValue->operand]);
                break;
            case DFL:
                params.push_back(DslValue(DFL, params.Count()));
                OutputValues(&params[top - 1], &params[top - 1]);
                break;
            case PSL:
                OutputValues(&params[top + 1], &params[BP+dslValue->operand]);
                params[++top].LiteCopy(&params[BP+dslValue->operand]);
                params[top].operand = dslValue->operand;
                printf("\toperand = %lld\toperand = %lld", dslValue->operand, params[top].operand);
                break;
            case RET:
                return;
            case JSR:
                JumpToSubroutineTrace(dslValue);
                break;
            case JTB:
                ProcessJumpTableTrace(dslValue);
                break;
        }
        putchar('\n');
    }
}

CPU::CPU()
{
    PC = 0;
    top = 0;
    BP = 0;
    SP.clear();
    A = new DslValue();
    params.Clear();
}

#pragma clang diagnostic pop