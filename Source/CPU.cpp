//before change to param pointer stack

//
// Created by krw10 on 9/7/2023.
//

#include "../Includes/cpu.h"
#include "../Includes/JsonParser.h"
#include <cmath>

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
       "JSR",    //Jump to _s subroutine.
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
       "ADA",     //Add assign.
       "SUA",    //Subtract assign
       "MUA",    //Multiply Assign
       "DIA",    //Divide Assign
       "MOA"     //Modulo Assign
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
        case TLE: case AND: case LOR: case ADA: case SUA: case MUA: case DIA: case MOA:
            putchar('\t');
            break;
        case PVA:
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

void CPU::pfn_abs()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = abs(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_acos()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = acos(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_asin()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = asin(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_atan()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = atan(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_atan2()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = atan2(params[top-totalParams].dValue, params[top-totalParams+1].dValue);

    top -= totalParams;
}

void CPU::pfn_cos()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = cos(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_sin()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = sin(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_tan()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = tan(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_cosh()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = cosh(params[top-totalParams].dValue);

    top -= totalParams;

}

void CPU::pfn_sinh()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = sinh(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_tanh()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = tanh(params[top-totalParams].dValue);

    top -= totalParams;

}

void CPU::pfn_exp()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = exp(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_log()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = log(params[top-totalParams].dValue);

    top -= totalParams;

}

void CPU::pfn_log10()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = log10(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_sqrt()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = log10(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_ceil()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = ceil(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_fabs()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = fabs(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_floor()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = floor(params[top-totalParams].dValue);

    top -= totalParams;
}

void CPU::pfn_fmod()
{
    auto totalParams = params[top].iValue;

    params[top-totalParams].Convert(DOUBLE_VALUE);
    params[top-totalParams].dValue = atan2(params[top-totalParams].dValue, params[top-totalParams+1].dValue);

    top -= totalParams;
}

void CPU::pfn_print()
{
    DslValue dslValue;

    auto totalParams = params[top].iValue;

    for(int64_t ii=top-totalParams; ii<top; ++ii)
    {
        params[ii].Print(false);
    }
    top -= totalParams + 1;
}

void CPU::pfn_printf()
{
    DslValue dslValue;

    auto totalParams = params[top].iValue;

    for(int64_t ii=top-totalParams; ii<top; ++ii)
    {
        params[ii].Print(false);
    }
    top -= totalParams + 1;
}

void CPU::pfn_input()
{
    auto totalParams = params[top].iValue;
    top -= totalParams + 1;

    printf(">");
    char szBuffer[1024];
    fgets(szBuffer, 1024, stdin);
    char *p = szBuffer+strlen(szBuffer)-1;
    if ( *p == '\r' || *p == '\n' )
    {
        *p = '\0';
    }

    //PSI input value
    A->type = STRING_VALUE;
    A->sValue.CopyFromCString(szBuffer);
    params[++top].LiteCopy(A);
}

/// \desc Reads a file current using the local file system. The file is returned
///       int the A dsl value. In case of an error the A dsl value will contain
///       an error.
/// \param file location to read the file from.
/// \return True if successful, else false if an error occurs.
bool CPU::Read(DslValue *file)
{
    FILE *fp = fopen(file->sValue.cStr(), "r");
    if (fp == nullptr )
    {
        A->sValue.CopyFromCString("FILE: ");
        A->sValue.Append(file->sValue.cStr());
        A->sValue.Append(" error ");
        A->sValue.Append(strerror(errno));
        A->sValue.Append("\n");
        A->type = STRING_VALUE;
        fclose(fp);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *)calloc(1, len+1);
    fread(buffer, 1, len, fp);
    fclose(fp);
    A->sValue.CopyFromCString(buffer);

    return true;
}

/// \desc Raised an error event, currently only prints but will be changed as soon
///       as the eventing system is in place.
void CPU::Error(DslValue *dslError)
{
    dslError->Print(false);
}

void CPU::pfn_read()
{
    auto totalParams = params[top].iValue;
    top -= totalParams;

    params[top].Convert(STRING_VALUE);
    if ( !Read(&params[top]) )
    {
        Error(A);
        params[top].SAV(A);
    }
    else
    {
        JsonParser jp;
        auto *json = jp.From(&params[top].sValue, &A->sValue);
        if ( json->type == ERROR_TOKEN )
        {
            json->type = STRING_VALUE;
            Error(json);
        }
        params[top].SAV(json);
    }
}

void CPU::pfn_write()
{
    auto totalParams = params[top].iValue;
    top -= totalParams;

    U8String out;
    for(int64_t ii=1; ii<totalParams; ++ii)
    {
        params[top+ii].AppendAsJsonText(&out);
    }
    params[top].Convert(STRING_VALUE);
    FILE *fp = fopen(params[top].sValue.cStr(), "w+");
    if (fp == nullptr )
    {
        A->sValue.CopyFromCString("Failed");
        A->sValue.Append(" FILE = ");
        A->sValue.Append(params[top].sValue.cStr());
        A->sValue.Append(" error ");
        A->sValue.Append(strerror(errno));
        A->sValue.Append("\n");
        A->type = STRING_VALUE;
        fclose(fp);
        return;
    }
    fwrite(out.cStr(), 1, out.Count(), fp);
    fclose(fp);
    params[top].sValue.CopyFromCString("Success");
}

void CPU::pfn_files()
{

}

void CPU::pfn_delete()
{

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

    params[top-totalParams].Convert(INTEGER_VALUE);
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

DslValue *CPU::GetCollectionElement(DslValue *dslValue)
{
    auto     totalParams = params[top--].iValue; //subtract out the param count.
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

    top -= totalParams;
    collection->variableAddress = collection;

    return collection;
}

void CPU::PushVariableAddress(DslValue *variable)
{
    DslValue *value = variable;

    if (variable->type == COLLECTION )
    {
        value = GetCollectionElement(variable);
    }
    params[++top].variableAddress = value;
}


void CPU::RunNoTrace()
{
    while(PC < program.Count() )
    {
        DslValue *dslValue = program[PC++];
        switch( dslValue->opcode )
        {
            case DEF: case NOP: case END: case PSP:
                break;
            case SLV:
                params[BP+params[top - 1].operand].SAV(&params[top]);
                top--;
                break;
            case SAV:
                params[top-1].variableAddress->SAV(&params[top]);
                top--;
                top--;
                break;
            case ADA:
                params[top-1].variableAddress->ADD(&params[top]);
                --top;
                break;
            case SUA:
                params[top-1].variableAddress->SUB(&params[top]);
                --top;
                break;
            case MUA:
                params[top-1].variableAddress->MUL(&params[top]);
                --top;
                break;
                break;
            case DIA:
                params[top-1].variableAddress->DIV(&params[top]);
                --top;
                break;
                break;
            case MOA:
                params[top-1].variableAddress->MOD(&params[top]);
                --top;
                break;
                break;
            case PVA:
                PushVariableAddress(program[dslValue->operand]);
                break;
            case PCV:
                dslValue = GetCollectionElement(program[dslValue->operand]);
                params[++top].LiteCopy(dslValue);
                params[top].variableAddress = dslValue;
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
            case DEF: case NOP: case END: case PSP:
                printf(";top = %lld;\tvar %s", top, dslValue->variableName.cStr());
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
                params[top-1].variableAddress->SAV(&params[top]);
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
                params[top-1].variableAddress->SUB(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case MUA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].variableAddress->MUL(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case DIA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].variableAddress->DIV(&params[top]);
                --top;
                PrintResult(&params[top]);
                break;
            case MOA:
                OutputValues(&params[top - 1], &params[top]);
                params[top-1].variableAddress->MOD(&params[top]);
                --top;
                PrintResult(&params[top]);
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
}