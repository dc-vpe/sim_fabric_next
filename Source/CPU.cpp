//before change to param pointer stack

//
// Created by krw10 on 9/7/2023.
//

#include "../Includes/cpu.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
const char *OpCodeNames[] =
{
        "",       //Opcodes start with 1
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
        "DCS",    //Save non-static result in a collection during definition
        "EFI",    //Event function information
        "RFE",    //Return from event.
        "CID"     //Change module id.
};

int64_t  CPU::errorCode;
U8String CPU::szErrorMsg;


/////////////////////////////////////////////////////
/// library allows adding external function calls ///
/////////////////////////////////////////////////////

int64_t totalParameters = -1;

int64_t OpenParameterStack(CPU *cpu)
{
    if ( totalParameters == -1 )
    {
        totalParameters = cpu->params[cpu->top].iValue;
    }

    return totalParameters;
}

void CloseParameterStack(CPU *cpu, DslValue *returnValue)
{
    cpu->top -= totalParameters + 1;
    ++cpu->top;
    cpu->params[cpu->top].LiteCopy(returnValue);

    totalParameters = -1;
}

DslValue *GetParameter(CPU *cpu, int64_t number)
{
    DslValue *v = &cpu->params[(cpu->top - totalParameters) + number];
    return &cpu->params[(cpu->top - totalParameters) + number];
}

/*
 * library: c:\function\math.lib
 * function(param, param)
 */

/////////////////////////////////////////////////////

void CPU::DisplayASMCodeLines(List<DslValue *> &programInstructions)
{
    int64_t addr = 0;

    while(addr < program.Count() )
    {
        addr = DisplayASMCodeLine(programInstructions, addr) + 1;
    }
}

/// \desc Displays the IL Assembly for the program.
int64_t CPU::DisplayASMCodeLine(List<DslValue *> &programInstructions, int64_t addr, bool newline)
{
    DslValue *dslValue = programInstructions[addr];

    printf("%4.4llx\t%s", addr, OpCodeNames[dslValue->opcode]);
    switch(dslValue->opcode)
    {
        case COM:
            printf("%s", dslValue->component->title.cStr());
            break;
        case CID:
            printf("\t%llx", (long long int)dslValue->moduleId);
            break;
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
        case TLE: case AND: case LOR: case ADA: case SUA: case MUA: case DIA: case MOA:
        case RFE:
            break;
        case EFI:
            putchar('\t');
            printf("%llx, %s() = %4.4llx",
                   (long long int)dslValue->moduleId,
                   dslValue->variableScriptName.cStr(),
                   (long long int)dslValue->location);
            break;
        case PVA:
            printf("\t&%s", dslValue->variableName.cStr());
            break;
        case DCS:
            printf("\t&%s[%llx]", dslValue->variableName.cStr(), (long long int)dslValue->iValue);
            break;
        case PCV:
            printf("\t%s[", dslValue->variableName.cStr());
            programInstructions[addr-2]->Print(true);
            printf("]");
            break;
        case DEC: case INC:
            printf("\t%s\t;var %s", dslValue->variableName.cStr(), dslValue->variableName.cStr());
            break;
        case INL: case DEL:
            printf("\t%s\t;param[BP+%llx]", dslValue->variableName.cStr(), (long long int)dslValue->operand);
            break;
        case JTB:
            printf("\tend:%4.4llx, ", (long long int)dslValue->location);
            for (int64_t ii = 0; ii < dslValue->cases.Count(); ++ii)
            {
                DslValue *caseValue = dslValue->cases[ii];
                if ( caseValue->type == DEFAULT )
                {
                    printf("default:");
                }
                else
                {
                    printf("case ");
                    caseValue->Print(true);
                }
                printf(":%4.4llx", (long long int)caseValue->location);
                if ( ii + 1 < dslValue->cases.Count() )
                {
                    printf(", ");
                }
            }
            break;
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
            printf("\t%4.4llx", (long long int)dslValue->location);
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

/// \desc Checks if the ch character is a match for the expression character.
/// \param ch character to check.
/// \param ex expression character that defines how the ch character should be checked.
/// \param caseLessCompare True if the compare should occur in a case agnostic manner,
///                        or false if the compare should occur in a case sensitive manner.
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

/// \desc Gets an expression character from an expression string.
/// \param expression U8String containing the search expression string.
/// \param offset Offset within the expression string at which to get the character.
///               The offset is set to the next expression character upon return.
/// \return The expression character without the leading % escape character.
u8chr CPU::GetExChar(U8String *expression, int64_t &offset)
{
    u8chr ex = expression->get(offset++);
    if ( ex == '%' )
    {
        ex = expression->get(offset++);
    }

    return ex;
}

/// \desc Checks to see if the expression is contained in the search string beginning at the start
///       character location.
/// \param search Pointer to a U8String containing the characters to be searched.
/// \param expression Pointer to a U8String containing the expression that specifies the expression
///                   to be searched for.
/// \param start The character location within the search string at which the search should begin.
/// \returns The character location within search that the expression was found or -1 if the
///          expression was not found beginning at start up to the end of the search string.
/// \remarks The expression string consists of characters with the % character used to
///          indicate a special type of compare.
///          The % character is used in the expression string to indicate that the next character
///          represents one of the character sets in the table shown here:
///             Code	Description	            Finds characters	            C equivalent function
///             %C	    Control characters	    0x00 though 0x1F and 0x7F	    Iscntrl()
///             %B	    Blank characters	    ‘\t’ and ‘ ‘	                Isblank()
///             %S	    Space characters	    ‘\t’, ‘\n’, ‘\f’, ‘\v’, ‘\r’    Isspace()
///             %U	    Upper case letter	    ABCDEFGHIJKLMNOPQRSTUVWXYZ	    Isupper()
///             %u	    Lower case letter	    abcdefghijklmnopqrstuvwxyz	    Islower()
///             %A	    Any letter	ABCDEFGHIJKLMNOPQRSTUVWXYZ
///                                 abcdefghijklmnopqrstuvwxyz	                Isalpha()
///             %D	    Any digit	0123456789	                                Isdigit()
///             %N	    Any letter or digit	ABCDEFGHIJKLMNOPQRSTUVWXYZ
///                                         Abcdefghijklmnopqrstuvwxyz
///                                         0123456789	                        Isalnum()
///             %P	Any punctuation	!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~	        Ispunct()
///             %G	Any character that can be seen when displayed.
///                 !"#$%&'()*+,-.0123456789:;<=>?
///                 @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`
///                 Abcdefghijklmnopqrstuvwxyz{|}~	                            Isgraph()
///             %p	Any printable character	!"#$%&'()*+,-./0123456789
///                 :;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`
///                 Abcdefghijklmnopqrstuvwxyz{|}~	                            Isprint()
///             %X	Any hexadecimal digit	0123456789ABCDEFabcdef.             Isxdigit()
///             %?	Wild card matches any character at its position.
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

/// \desc Reads a file current using the local file system. The file is returned
///       int the A dsl value. In case of an error the A dsl value will contain
///       an error.
/// \param file position to read the file from.
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

/// \desc Creates a sub string from the search string.
/// \param search String containing the characters from which to build the sub string.
/// \param result String that will contain the sub string.
/// \param start Starting character location in the search string.
/// \param length Number of characters to copy from the search string to the result string.
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

/// \desc Calculates the number of characters in a search expression.
/// \param expression Pointer to a U8String that contains the regular expression.
/// \return The number of characters in the search expression string.
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
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
    auto *param2 = GetParameter(this, 1);
    auto *param3 = GetParameter(this, 2);

    //string to search
    param1->Convert(STRING_VALUE);
    U8String search;
    search.CopyFrom(&param1->sValue);

    //expression
    param2->Convert(STRING_VALUE);
    U8String expression;
    expression.CopyFrom(&param2->sValue);

    //start position
    param3->Convert(INTEGER_VALUE);
    int64_t start = param3->iValue;

    A->type = INTEGER_VALUE;
    A->iValue = Find(&search, &expression, start);

    CloseParameterStack(this, A);
}

void CPU::pfn_string_len()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);

    param1->Convert(STRING_VALUE);
    A->iValue = (int64_t)param1->sValue.Count();

    CloseParameterStack(this, A);
}

void CPU::pfn_string_sub()
{
    auto totalParams = OpenParameterStack(this);


    auto *param1 = GetParameter(this, 0);
    auto *param2= GetParameter(this, 1);
    auto *param3= GetParameter(this, 2);

    A->type = STRING_VALUE;
    A->sValue.Clear();

    //string to search
    param1->Convert(STRING_VALUE);
    //start
    param2->Convert(INTEGER_VALUE);
    //length
    param3->Convert(INTEGER_VALUE);
    Sub(&param1->sValue, &A->sValue, param2->iValue, param3->iValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_string_replace()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
    auto *param2= GetParameter(this, 1);
    auto *param3= GetParameter(this, 2);

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

    CloseParameterStack(this, A);
}

void CPU::pfn_string_tolower()
{
    auto totalParams = OpenParameterStack(this);

    //string to search
    auto  *param1 = GetParameter(this, 0);
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

    CloseParameterStack(this, A);
}

void CPU::pfn_string_toupper()
{
    auto totalParams = OpenParameterStack(this);

    //string to search
    auto *param1 = GetParameter(this, 0);
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

    CloseParameterStack(this, A);
}

void CPU::pfn_string_trimEnd()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
    auto *param2= GetParameter(this, 1);

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

    CloseParameterStack(this, A);
}

void CPU::pfn_string_trimStart()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
    auto *param2= GetParameter(this, 1);

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

    CloseParameterStack(this, A);
}

void CPU::pfn_string_toCollection()
{
    auto totalParams = OpenParameterStack(this);

    //String to convert to a collection.
    auto *param1 = GetParameter(this, 0);
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

    CloseParameterStack(this, A);
}

void CPU::pfn_string_fromCollection()
{
    auto totalParams = OpenParameterStack(this);

    A->type = STRING_VALUE;
    GetParameter(this, 0)->AppendAsJsonText(&A->sValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_abs()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);

#ifdef __linux__
    A->dValue = abs(param->dValue);
#else
    A->dValue = std::abs(param->dValue);
#endif

    CloseParameterStack(this, A);
}

void CPU::pfn_acos()
{
    auto totalParams = OpenParameterStack(this);
    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = acos(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_asin()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = asin(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_atan()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = atan(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_atan2()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
    auto *param2= GetParameter(this, 1);

    param1->Convert(DOUBLE_VALUE);
    param2->Convert(DOUBLE_VALUE);
    A->type = DOUBLE_VALUE;
    A->dValue = atan2(param1->dValue, param2->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_cos()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = cos(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_sin()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = sin(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_tan()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = tan(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_cosh()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = cosh(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_sinh()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = sinh(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_tanh()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = tanh(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_exp()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = exp(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_log()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = log(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_log10()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = log10(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_sqrt()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = sqrt(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_ceil()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = ceil(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_fabs()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = fabs(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_floor()
{
    auto totalParams = OpenParameterStack(this);

    A->type = DOUBLE_VALUE;
    auto *param = GetParameter(this, 0);
    param->Convert(DOUBLE_VALUE);
    A->dValue = floor(param->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_fmod()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
    auto *param2= GetParameter(this, 1);
    auto *param3= GetParameter(this, 2);

    param1->Convert(DOUBLE_VALUE);
    param2->Convert(DOUBLE_VALUE);
    A->type = DOUBLE_VALUE;
    A->dValue = fmod(param1->dValue, param2->dValue);

    CloseParameterStack(this, A);
}

void CPU::pfn_print()
{
    DslValue dslValue;

    int64_t totalParams = OpenParameterStack(this);

    for(int64_t ii=0; ii<totalParams; ++ii)
    {
        GetParameter(this, ii)->Print(false);
    }

    A->type = INTEGER_VALUE;
    A->iValue = 0;

    CloseParameterStack(this, A);
}

void CPU::pfn_printf()
{
    pfn_print();
}

void CPU::pfn_input()
{
    auto totalParams = OpenParameterStack(this);

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

    CloseParameterStack(this, A);
}

void CPU::pfn_read()
{
    auto totalParams = OpenParameterStack(this);

    auto *param1 = GetParameter(this, 0);
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

    CloseParameterStack(this, A);
}

void CPU::WriteFile(U8String *fileName, int64_t totalParams)
{
    for(int64_t ii=1; ii<totalParams; ++ii)
    {
        GetParameter(this, ii)->AppendAsJsonText(&A->sValue);
    }

    FILE *fp = fopen(fileName->cStr(), "w+");
    if (fp == nullptr )
    {
        A->sValue.CopyFromCString("Failed");
        A->sValue.Append(" FILE = ");
        A->sValue.Append(fileName);
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
}

void CPU::pfn_write()
{
    auto totalParams = OpenParameterStack(this);

    A->type = STRING_VALUE;
    A->sValue.Clear();

    auto *param1 = GetParameter(this, 0);

    U8String fileName;
    fileName.CopyFrom(&param1->sValue);

    if ( fileName.BeginsWith("https://", false))
    {
        printf("Not Implemented 1.");
    }
    else if ( fileName.BeginsWith("ftps://", false))
    {
        printf("Not Implemented 2.");
    }
    else if ( fileName.BeginsWith("wss://", false) )
    {
        printf("Not Implemented 3.");
    }
    else
    {
        WriteFile(&fileName, totalParams);
    }

    CloseParameterStack(this, A);
}

void CPU::pfn_files()
{
    auto totalParams= OpenParameterStack(this);
#ifndef linux
    bool open = false;
    if ( totalParams >= 2 )
    {
        auto *tmp = GetParameter(this, 1);
        tmp->Convert(BOOL_VALUE);
        open = tmp->bValue;
    }

    auto *param1 = GetParameter(this, 0);

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
                    CloseParameterStack(this, A);
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
                        CloseParameterStack(this, json);
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
#endif
    CloseParameterStack(this, A);
}

void CPU::pfn_delete()
{
    auto totalParams= OpenParameterStack(this);

    A->type = STRING_VALUE;
    A->sValue.CopyFromCString("Not Implemented");
    Error(A);

    CloseParameterStack(this, A);
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

void CPU::JumpToBuiltInFunction(DslValue *dslValue)
{
    (this->*builtInMethods[dslValue->operand])();
}

/// \desc calls a compiled script function.
/// remarks
///locals are set in order encountered after parameters and start at operand == 0 and
///each local adds 1 to the operand.
void CPU::JumpToSubroutine(DslValue *dslValue)
{
    auto totalParams = params[top].iValue;

    int64_t pcReturn = PC;
    int64_t saved = BP;
    BP = top - totalParams;
    PC = dslValue->location;
    RunNoTrace();   //BUG-BUG: no way to run traced when calling a script function
    A->SAV(&params[top]);
    --top;
    BP = saved;
    top -= totalParams + 1;
    params[++top].SAV(A);
    PC = pcReturn;
}

/// \desc Handles a jump table instruction.
/// \param dslValue Pointer to the dsl value containing the information needed to execute the jump
void CPU::ProcessJumpTable(DslValue *dslValue)
{
    DslValue *defaultCase = nullptr;

    for(int64_t ii=0; ii<dslValue->cases.Count(); ++ii)
    {
        if ( dslValue->cases[ii]->type == DEFAULT )
        {
            defaultCase = dslValue->cases[ii];
        }
        else
        {
            if ( params[top].IsEqual(dslValue->cases[ii]) )
            {
                PC = dslValue->cases[ii]->location;
                --top;
                return;
            }
        }
    }

    if ( defaultCase != nullptr )
    {
        PC = defaultCase->location;
        --top;
        return;
    }

    //jump out of switch.
    PC = dslValue->location;
    --top;
}

/// \desc Translates the IL bytes into a runnable program.
///       A program that can be run is a set of dslValues.
void CPU::DeSerializeIL(BinaryFileReader *binaryFileReader)
{
    DslValue *dslValue;

    int64_t lastModId = 1;

    instructions.Clear();

    while(!binaryFileReader->eof() )
    {
        auto opcode = (OPCODES)binaryFileReader->GetInt();
        dslValue = new DslValue(opcode);
        dslValue->moduleId = lastModId;
        switch( opcode )
        {
            case END: case NOP: case PSP: case RFE:
            case SLV: case SAV: case ADA: case SUA: case MUA: case DIA: case MOA: case EXP:
            case MUL: case DIV: case ADD: case SUB: case MOD: case XOR: case BND: case BOR:
            case SVL: case SVR: case TEQ: case TNE: case TGR: case TGE: case TLS: case TLE:
            case AND: case LOR: case NOT: case NEG: case CTI: case CTD: case CTC: case CTS:
            case CTB: case DFL: case RET:
                break;
            case PVA: case PSV: case PSL: case JBF: case PCV:
            case INL: case DEL: case INC: case DEC:
                dslValue->operand = binaryFileReader->GetInt();
                break;
            case JIF: case JIT:
                dslValue->operand = binaryFileReader->GetInt();
                dslValue->location = binaryFileReader->GetInt();
                dslValue->bValue = binaryFileReader->GetInt();
                break;
            case JMP: case JSR:
                dslValue->location = binaryFileReader->GetInt();
                break;
            case EFI:
                dslValue->operand = binaryFileReader->GetInt();
                dslValue->location = binaryFileReader->GetInt();
                dslValue->moduleId = binaryFileReader->GetInt();
                break;
            case DEF: case PSI:
                binaryFileReader->GetValue(dslValue);
                break;
            case JTB:
            {
                int64_t count = binaryFileReader->GetInt();
                for (int ii = 0; ii < count; ++ii)
                {
                    auto *caseValue = new DslValue();
                    binaryFileReader->GetValue(caseValue);
                    if (binaryFileReader->GetInt() == 1)
                    {
                        caseValue->type = DEFAULT;
                    }
                    caseValue->operand = binaryFileReader->GetInt();
                    caseValue->location = binaryFileReader->GetInt();
                    dslValue->cases.push_back(caseValue);
                }
                break;
            }
            case DCS:
                dslValue->operand = binaryFileReader->GetInt();
                dslValue->iValue = binaryFileReader->GetInt();
                break;
            case CID:
                lastModId = binaryFileReader->GetInt();
                dslValue->moduleId = lastModId;
                break;
            case COM:
            {
                dslValue->component = new ComponentData();
                binaryFileReader->GetString(&dslValue->component->title);
                binaryFileReader->GetString(&dslValue->component->toolTip);
                dslValue->component->x = binaryFileReader->GetInt();
                dslValue->component->y = binaryFileReader->GetInt();
                dslValue->component->width = binaryFileReader->GetInt();
                dslValue->component->height = binaryFileReader->GetInt();
                int64_t count = binaryFileReader->GetInt();
                for (int64_t tt = 0; tt < count; ++tt)
                {
                    SlotData slotData = {};
                    slotData.IsInput = binaryFileReader->GetInt();
                    binaryFileReader->GetString(&slotData.color);
                    binaryFileReader->GetString(&slotData.variable);
                    dslValue->component->slots.push_back(new SlotData(slotData));
                }
                break;
            }
        }

        instructions.push_back(dslValue);
    }
}

/// \desc Deserializes the program symbol file and adds those symbols to the
///       internal program.
/// \param binaryFileReader Pointer to the binary file reader to use.
/// \return True if the symbols are added to the program, false if the symbol
///         file does not exist or can't be read.
bool CPU::DeSerializeSymbols(BinaryFileReader *binaryFileReader)
{
    while( !binaryFileReader->eof() )
    {
        int64_t addr = binaryFileReader->GetInt();
        binaryFileReader->GetString(&instructions[addr]->variableName);
        binaryFileReader->GetString(&instructions[addr]->variableScriptName);
    }

    return true;
}

/// \desc Adds the locations used in the program as its symbols. Used when the symbol file
///       can't be read or is not present.
void CPU::SetProgramLocationsAsSymbols()
{
    for(int ii=0; ii<instructions.Count(); ++ii)
    {
        switch( instructions[ii]->opcode )
        {
            case DEF: case DFL: case EFI:
            case PVA: case DCS: case PCV:
            case DEC: case INC:
            case INL: case DEL: case PSV:
            case SAV: case SVL: case PSL:
            case PSP: case CID: case COM:
                instructions[ii]->variableName.printf((char *)"%llx", (long long int)instructions[ii]->operand);
            default:
                break;
        }
    }
}

/// \desc Reads the IL file and translates it into an internal format ready to be run by the runtime.
/// \param ilFile Pointer to a u8String containing the full path name to the program file.
/// \param symFile Pointer to a u8String containing the full path name to the symbol file. If empty
///                then no symbol file is loaded.
/// \returns True if successful or false if an error occurs.
bool CPU::Init(U8String *ilFile, U8String *symFile)
{
    BinaryFileReader binaryFileReader = {};

    binaryFileReader.fread(ilFile);

    DeSerializeIL(&binaryFileReader);

    if ( binaryFileReader.fread(symFile) )
    {
        DeSerializeSymbols(&binaryFileReader);
    }
    else
    {
        SetProgramLocationsAsSymbols();
    }

    return true;
}

bool CPU::Run()
{
    //error handles need setup
    if ( traceInfoLevel == 1 )
    {
        RunTrace();
    }
    else
    {
        RunNoTrace();
    }

    return true;
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
    auto *var = instructions[dslValue->operand];
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

int64_t CPU::GetEventLocation(SystemErrorHandlers errorHandler, int64_t moduleId)
{

//    DslValue *onError = program[base + (10 * (instruction->moduleId-1))];
//    handlerLocation = onError->position;

    return 0;
}

/// \desc calls the on error handler if one exists.
void CPU::JumpToOnErrorHandler()
{
    auto totalParams = params[top].iValue;

    DslValue *instruction = instructions[PC];

    int64_t base = instructions[0]->location;

    int64_t onErrorLocation = GetEventLocation(ON_ERROR, instruction->moduleId);

    if ( onErrorLocation == 0 )
    {
        printf("%s", szErrorMsg.cStr());
        PC = instructions.Count();
        return;
    }

    int64_t pcReturn = PC;
    int64_t saved = BP;
    BP = top - totalParams;
    PC = onErrorLocation;

    while(PC < instructions[0]->location )
    {
        DslValue *dslValue = instructions[PC++];
        //If return from on error handler.
        if ( dslValue->opcode == RET )
        {
            //Check handler return code.
            A->SAV(&params[top]);
            A->Convert(INTEGER_VALUE);
            --top;
            switch( A->iValue )
            {
                case 100:   //return error.continue
                    BP = saved;
                    top -= totalParams + 1;
                    params[++top].SAV(A);
                    PC = pcReturn;
                    return;
                //The rest of these require the sim framework
                case 200: //error.restart
                case 300: //error.reload
                case 400: //error.waitQuit
                case 500: //error.WaitReload
                    break;
                default: //error.quit
                    BP = saved;
                    top -= totalParams + 1;
                    params[++top].SAV(A);
                    PC = pcReturn;
                    return;
            }
        }

        if ( !RunInstruction(dslValue) )
        {
            break;
        }
    }
}

/// \desc calls the on error handler if one exists.
void CPU::JumpToOnTick()
{
    if ( onTickEvent == 0 )
    {
        return;
    }

    if ( clock() < nextTick )
    {
        return;
    }

    nextTick = clock() + TICKS_PER_SECOND;

    List<DslValue> pSave;  //function call parameters stack
    int64_t pcReturn = PC;
    int64_t sBP = BP;
    PC = onTickEvent;
    int64_t stop = top;
    pSave.CopyFrom(&params);

    DslValue *instruction = instructions[PC++];
    while( instruction->opcode != RFE )
    {
        if (!RunInstruction(instruction))
        {
            break;
        }
        instruction = instructions[PC++];
    }
    PC = pcReturn;
    BP = sBP;
    top = stop;
    params.CopyFrom(&pSave);
}

bool CPU::RunInstruction(DslValue *dslValue)
{
   SetTickEvent(dslValue);

    switch( dslValue->opcode )
    {
        case END:
            PC = instructions.Count();
            return false;
        case COM: case CID: case EFI: case DEF: case NOP: case PSP: case RFE:
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
        case DIA:
            params[top-1].elementAddress->DIV(&params[top]);
            --top;
            break;
        case MOA:
            params[top-1].elementAddress->MOD(&params[top]);
            --top;
            break;
        case DCS:
            SetCollectionElementDirect(dslValue);
            break;
        case PVA:
            PushVariableAddress(instructions[dslValue->operand]);
            break;
        case PCV:
            dslValue = GetCollectionElement(instructions[dslValue->operand]);
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
            instructions[dslValue->operand]->INC();
            break;
        case DEC:
            instructions[dslValue->operand]->DEC();
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
            JumpToBuiltInFunction(dslValue);
            break;
        case PSI:
            params[++top].LiteCopy(dslValue);
            break;
        case PSV:
            params[++top].LiteCopy(instructions[dslValue->operand]);
            break;
        case DFL:
            params.push_back(DslValue(DFL, params.Count()));
            break;
        case PSL:
            params[++top].LiteCopy(&params[BP+dslValue->operand]);
            params[top].operand = dslValue->operand;
            break;
        case RET:
            return false;
        case JSR:
            JumpToSubroutine(dslValue);
            break;
        case JTB:
            ProcessJumpTable(dslValue);
            break;
    }

   return true;
}

void CPU::RunNoTrace()
{
    bool errorMode = false;
    bool onTickCalled = false;

    int64_t programEnd = instructions.Count();

    while(PC < programEnd )
    {
        //If an error has been raised, switch to error mode which only processes the
        //on error callback if one exists, if not the error is simply sent to the
        //console and the program exited.
        if ( errorCode != 0 && !errorMode )
        {
            errorMode = true;
            JumpToOnErrorHandler();
            errorCode = 0;
            errorMode = false;
            continue;
        }

        JumpToOnTick();

        if( !RunInstruction(instructions[PC++]) )
        {
            break;
        }
    }
}

void CPU::RunTrace()
{
    DslValue left;
    DslValue right;
    bool errorMode = false;
    bool onTickCalled = false;

    int64_t programEnd = instructions.Count();

    putchar('\n');
    while(PC < programEnd )
    {
        DisplayASMCodeLine(instructions, PC, false);

        DslValue *instruction = instructions[PC++];
        switch( GetInstructionOperands(instruction, &left, &right) )
        {
            case 0:
                break;
            case 1:
                printf(";top = %ld\t", (long)top);
                left.Print(true);
                printf("\n");
                break;
            case 2:
                printf(";top = %ld\t", (long)top);
                left.Print(true);
                printf("\t");
                right.Print(true);
                printf("\n");
                break;
        }

        if ( errorCode != 0 && !errorMode )
        {
            errorMode = true;
            JumpToOnErrorHandler();
            errorCode = 0;
            errorMode = false;
            continue;
        }

        JumpToOnTick();

        if ( !RunInstruction(instruction) )
        {
            break;
        }
    }
}

int64_t CPU::GetInstructionOperands(DslValue *instruction, DslValue *left,  DslValue *right)
{
    int64_t operands = 0;
    right = nullptr;
    left = nullptr;

    switch( instruction->opcode )
    {
        case SLV:
            left = &params[BP+params[top - 1].operand];
            right = &params[top];
            operands = 2;
            break;
        case CID: case SAV: case ADA: case SUA: case MUA: case DIA: case MOA: case EXP:
        case MUL: case DIV: case ADD: case SUB: case MOD: case XOR: case BND: case BOR:
        case SVL: case SVR: case TEQ: case TNE: case TGR: case TGE: case TLS: case TLE:
        case AND: case LOR:
            right = &params[top];
            left = params[top-1].elementAddress;
            operands = 2;
            break;
        case DCS:
        {
            right = &params[top];
            auto *var = instructions[instruction->operand];
            auto *key = var->indexes.keys[instruction->iValue];
            left = ((DslValue *)var->indexes.Get(key)->Data());
            operands = 2;
            break;
        }
        case PVA:
        {
            left = instructions[instruction->operand];
            if (left->type == COLLECTION )
            {
                left = GetCollectionElement(left);
            }
            operands = 1;
            break;
        }
        case PCV:
            left = GetCollectionElement(instructions[instruction->operand]);
            left->elementAddress = left;
            operands = 1;
            break;
        case INL: case DEL:
            left = &params[BP+instruction->operand];
            operands = 1;
            break;
        case INC: case DEC:
            left = instructions[instruction->operand];
            operands = 1;
            break;
        case NOT: case NEG: case CTI: case CTD: case CTC: case CTS: case CTB:
            left = &params[top];
            operands = 1;
            break;
        case JIF: case JIT:
            left = &params[top];
            right = instruction;
            operands = 2;
            break;
        case JBF:
            left = instruction;
            operands = 1;
            break;
        case PSI:
            left = &params[top+1];
            operands = 1;
            break;
        case PSV:
            left = instructions[instruction->operand];
            operands = 1;
            break;
        case DFL:
            left = new DslValue(DFL, params.Count());
            break;
        case PSL:
            left = &params[BP+instruction->operand];
            left->operand = instruction->operand;
            break;
        case JMP: case JSR: case JTB: case NOP: case DEF: case END: case PSP: case EFI: case RFE:
        case RET:
            left = instruction;
            operands = 1;
            break;
        case COM:
            break;
    }

    return operands;
}

/// \desc Ensures that the on tick event pointer is set to the correct module function.
/// \param dslValue Instruction about to be run.
void CPU::SetTickEvent(DslValue *dslValue)
{
    if ( lastModuleId == dslValue->moduleId )
    {
        return;
    }
    else
    {
        lastModuleId = dslValue->moduleId;
        onTickEvent = GetEventLocation(ON_TICK, dslValue->moduleId);
    }
}

/// \desc Class constructor.
CPU::CPU()
{
    PC = 0;
    top = 0;
    BP = 0;
    A = new DslValue();
    params.Clear();
    errorCode = 0;
    nextTick = clock() + TICKS_PER_SECOND;
    lastModuleId = 1;
    onTickEvent = 0;
    instructions.Clear();
}

#pragma clang diagnostic pop