#include "../Includes/Opcodes.h"
#include "../Includes/parser.h"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

extern void DisplayTokenAsText(int64_t index, TokenTypes type);

/// \desc Peeks ahead the number of Tokens in the lexed token list.
Token *Parser::Peek(int64_t offset)
{
    int64_t pos = position + offset;
    if ( !IsPositionInRange(pos) )
    {
        return &end;
    }

    return tokens[pos];
}

/// \desc Gets the information about the variable named in the token. This call should always succeed.
Token *Parser::GetVariableInfo(Token *token)
{
    Token *variable = variables.Get(token->identifier);
    if ( variable == nullptr )
    {
        PrintIssue(3000,
                   true, true,
                   "Parser error, variable %s does not exist, this indicates an error in the lexer "
                   "as it did not catch the undefined variable.",
                   token->identifier);
    }

    return variable;
}

/// \desc For instructions that require a count like collection element references or
///       function calls this method outputs the instruction specifying the count.
/// \return The filled in instruction.
DslValue *Parser::OutputCount(int64_t count, int64_t moduleId)
{
    auto *tokenCount = new Token(new DslValue((int64_t)count));
    tokenCount->value->moduleId = moduleId;
    return OutputCode(tokenCount, PSI);
}

/// \desc Outputs the instruction code. All of the generated output from the parser
///       flows though this and the output code functions.
/// \return The filled in instruction.
DslValue *Parser::OutputCode(Token *token, OPCODES opcode)
{
    DslValue *value;
    Token *variable;
    Token *funInfo;

    switch(opcode)
    {
        case PSI:
            value = new DslValue(token->value);
            value->opcode = PSI;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case DCS:
            variable = GetVariableInfo(token);
            if ( variable == nullptr )
            {
                return nullptr;
            }
            value = new DslValue(variable->value);
            value->opcode = token->value->opcode;
            value->iValue = token->value->iValue;
            value->moduleId = token->value->moduleId;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case PVA:
            if (token->value->type == COLLECTION )
            {
                value = new DslValue(token->value);
                value->opcode = PVA;
                if ( !program.push_back(value) )
                {
                    return nullptr;
                }
            }
            else
            {
                variable = GetVariableInfo(token);
                if ( variable == nullptr )
                {
                    return nullptr;
                }
                value = new DslValue(variable->value);
                value->opcode = token->value->opcode;
                value->moduleId = token->value->moduleId;
                if ( !program.push_back(value) )
                {
                    return nullptr;
                }
            }
            break;
        case PSL: case PSV: case PCV:
            variable = GetVariableInfo(token);
            if ( variable == nullptr )
            {
                return nullptr;
            }
            value = new DslValue(variable->value);
            value->opcode = token->value->opcode;
            //value->operand = token->value->operand;
            value->moduleId = token->value->moduleId;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case DFL: case DEF:
            value = new DslValue(token->value);
            value->opcode = opcode;
            value->operand = program.Count();
            value->variableName.CopyFrom(&token->value->variableName);
            value->elementAddress = value;
            value->address = value;
            value->moduleId = token->value->moduleId;
            token->value->operand = value->operand;
            variables.Set(&token->value->variableName, token);
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case JBF:
            value = new DslValue(JBF, standardFunctions.Get(token->identifier)->value->operand);
            value->moduleId = token->value->moduleId;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case JSR:
            funInfo = functions.Get(token->identifier);
            value = new DslValue(JSR);
            value->variableName.CopyFrom(token->identifier);
            value->location = funInfo->value->location;
            value->moduleId = token->value->moduleId;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case JTB:
        case NOP: case RET: case JMP: case JIF: case JIT: case END: case RFE:
        case EXP: case NEG: case NOT: case MUL: case DIV: case MOD: case ADA: case SUA:
        case MUA: case DIA: case MOA: case ADD: case SUB: case SVL: case SVR: case TLS:
        case TLE: case TGR: case TGE: case TEQ: case TNE: case BND: case XOR: case BOR:
        case AND: case LOR: case CTI: case CTD: case CTC: case CTS: case CTB: case SAV:
        case INC: case DEC: case INL: case DEL: case CID: case SLV:
        case PSP:
            value = new DslValue(token->value);
            value->opcode = opcode;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case EFI:
            value = new DslValue(EFI);
            value->operand = token->value->operand; //1 though 9 are system types 10 + are user types.
            value->moduleId = token->value->moduleId;
            value->location = 0;
            funInfo = functions.Get(token->identifier);
            value->variableName.CopyFrom(&funInfo->value->variableName);
            value->variableScriptName.CopyFrom(&funInfo->value->variableScriptName);
            value->location = funInfo->value->location;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
        case COM:
            value = new DslValue(token->value);
            value->opcode = COM;
            funInfo = functions.Get(&token->value->component->function);
            if ( funInfo != nullptr )
            {
                value->variableName.CopyFrom(&funInfo->value->variableName);
                value->variableScriptName.CopyFrom(&funInfo->value->variableScriptName);
                value->location = funInfo->value->location;
            }
//            for(int64_t vv=0; vv<value->component->slots.Count(); ++vv)
//            {
//                Token *v = variables.Get(&value->component->slots[vv]->variable);
//                value->operand = v->value->operand;
//            }
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
    }

    return value;
}

/// \desc Generates the instruction to push the value or variable onto the stack.
/// \param token Token containing the information on what the value is.
/// \return Pointer to the last token processed or nullptr if an error out of memory occurs.
/// \remark If a variable is pushed, its address is contained in operand.
void Parser::PushValue(Token *token)
{
    //If is a numeric value
    if ( token->type != VARIABLE_VALUE && token->type != VARIABLE_ADDRESS &&
         token->type != COLLECTION_VALUE && token->type != COLLECTION_ADDRESS )
    {
        OutputCode(token, PSI);
        return;
    }

    //This is a variable or collection.
    //Note: Variable has the position of the defined variable in the generated program.
    //This is true for collections as well. Event in the non-static initialization case
    //the collection element will have a default value added as a placeholder.
    OutputCode(token, token->value->opcode);
}

/// \desc Creates a variable definition within the output IL and sets the tokens operand 1 to the position
///       of the variable within the programs IL.
/// \param token Token containing the information about the variable.
void Parser::CreateVariable(Token *token)
{
    auto *var = new DslValue(token->value);
    if ( token->modifier == TMLocalScope )
    {
        OutputCode(token, DFL);
    }
    OutputCode(token, DEF);
}

/// \desc Writes out an algebraic, conversion or logical instruction.
/// \param Token containing the information to be used to construct the instruction.
/// \returns The
void Parser::CreateOperation(Token *token)
{
    switch( token->type )
    {
        default: break;
        case EXPONENT: OutputCode(token, EXP); break;
        case UNARY_NEGATIVE: OutputCode(token, NEG); break;
        case UNARY_NOT: OutputCode(token, NOT); break;
        case MULTIPLY: OutputCode(token, MUL); break;
        case DIVIDE: OutputCode(token, DIV); break;
        case MODULO: OutputCode(token, MOD); break;
        case ADD_ASSIGNMENT: OutputCode(token, ADA); break;
        case SUBTRACT_ASSIGNMENT: OutputCode(token, SUA); break;
        case MULTIPLY_ASSIGNMENT: OutputCode(token, MUA); break;
        case DIVIDE_ASSIGNMENT: OutputCode(token, DIA); break;
        case MODULO_ASSIGNMENT: OutputCode(token, MOA); break;
        case ADDITION: OutputCode(token, ADD); break;
        case SUBTRACTION: OutputCode(token, SUB); break;
        case BITWISE_SHIFT_LEFT: OutputCode(token, SVL); break;
        case BITWISE_SHIFT_RIGHT: OutputCode(token, SVR); break;
        case LESS_THAN: OutputCode(token, TLS); break;
        case LESS_OR_EQUAL: OutputCode(token, TLE); break;
        case GREATER_THAN: OutputCode(token, TGR); break;
        case GREATER_OR_EQUAL: OutputCode(token, TGE); break;
        case EQUAL_TO: OutputCode(token, TEQ); break;
        case NOT_EQUAL_TO: OutputCode(token, TNE); break;
        case BITWISE_AND: OutputCode(token, BND); break;
        case BITWISE_XOR: OutputCode(token, XOR); break;
        case BITWISE_OR: OutputCode(token, BOR); break;
        case LOGICAL_AND: OutputCode(token, AND); break;
        case LOGICAL_OR: OutputCode(token, LOR); break;
        case CAST_TO_INT: OutputCode(token, CTI); break;
        case CAST_TO_DBL: OutputCode(token, CTD); break;
        case CAST_TO_CHR: OutputCode(token, CTC); break;
        case CAST_TO_STR: OutputCode(token, CTS); break;
        case CAST_TO_BOOL: OutputCode(token, CTB); break;
    }
}

/// \desc Entry point, parses the lexed tokens into an IL program.
bool Parser::Parse()
{
    if ( errors > 0 )
    {
        return false;
    }

    if ( warningLevel >= 3 && warnings != 0 )
    {
        return false;
    }

    program.Clear();

    position = 0;

    auto *token = new Token();
    token->type = INVALID_TOKEN;
    token->value->moduleId = 1;
    Token *tmp;

    if ( tokens.Count() == 0 )
    {
        OutputCode(token, CID);
    }
    else
    {
        OutputCode(token, CID);
        token = tokens[0];
    }

    Expression(tokens.Count());

    OutputCode(token, END);
    program[0]->location = program.Count(); //end of program instructions.

    FixUpFunctionCalls();
    FixUpJumpsToEnd();

    for(int64_t ii=0; ii<modules.Count(); ++ii)
    {
        //Add any system events.
        for(int64_t tt=1; tt<modules[ii]->systemEvents.Count(); ++tt)
        {
            if ( modules[ii]->systemEvents.Count() > 0 )
            {
                Token *funInfo = functions.Get(&modules[ii]->systemEvents[tt]);
                auto *ef = new Token(funInfo);
                ef->value->operand = tt;
                ef->value->moduleId = ii+1;
                OutputCode(ef, EFI);
            }
        }

        //Add any user events.
        for(int64_t tt=1; tt<modules[ii]->userEvents.Count(); ++tt)
        {
            if ( modules[ii]->userEvents.Count() > 0 )
            {
                Token *funInfo = functions.Get(&modules[ii]->userEvents[tt]);
                auto *ef = new Token(funInfo);
                ef->value->operand = tt;
                ef->value->moduleId = ii+1;
                OutputCode(ef, EFI);
            }
        }
    }

    //Add the component data
    for(int64_t ii=0; ii<componentsData.Count(); ++ii)
    {
        auto *component = new Token();

        component->type = COMPONENT;
        component->value->component = new ComponentData(*componentsData[ii]);
        OutputCode(component, COM);
    }

    bool result = true;

    if (errors != 0 || ( warningLevel >= 3 && warnings != 0 ))
    {
        result = false;
    }

    return result;
}

/// \desc Fixes up the locations of any function calls where the call was used before the function
///       definition was encountered.
void Parser::FixUpFunctionCalls()
{
    for(int64_t ii=0; ii<program.Count(); ++ii)
    {
        if ( program[ii]->opcode == JSR )
        {
            if ( program[ii]->location == 0 )
            {
                Token *token = functions.Get(&program[ii]->variableName);
                program[ii]->location = token->value->location;
            }
        }
    }
}

/// \desc Fixes up the locations of any jumps to the end of the program.
void Parser::FixUpJumpsToEnd()
{
    for(int64_t ii=0; ii<program.Count(); ++ii)
    {
        if ( program[ii]->opcode == JMP &&  program[ii]->location == 0 )
        {
            program[ii]->location = program.Count() - 1;
        }
    }
}

/// \desc Shunting yard expression parser, takes the lexed tokens from an expression and
///       arranges them into an output queue in the correct order to be processed into
///       an ordered list ready for code generation.
/// \param token First token in the expression.
/// \return A pointer to the output queue containing the ordered token list.
Token *Parser::ShuntingYard(Token *token, int64_t tokenLocation)
{
    Stack<Token *> ops;

    //This loop forms the core of a shunting yard expression parser.
    while (position < tokenLocation)
    {
        switch (token->type)
        {
            case RETURN: case EVENT_RETURN:
                ops.push_back(token);
                break;
            case BREAK: case SWITCH_BEGIN: case SWITCH_COND_BEGIN: case CASE_BLOCK_BEGIN: case WHILE_COND_BEGIN:
            case WHILE_BLOCK_BEGIN: case IF_COND_BEGIN: case IF_BLOCK_BEGIN: case ELSE_BLOCK_BEGIN:
            case FOR_UPDATE_BEGIN: case FOR_BLOCK_BEGIN: case FOR_INIT_BEGIN: case FOR_COND_BEGIN:
            case FUNCTION_CALL_END: case END_OF_SCRIPT:
            case DEFAULT_BLOCK_BEGIN: case DEFAULT_BLOCK_END: case FUNCTION_PARAMETER:
                output.Enqueue(token);
                break;
            case FUNCTION_DEF_BEGIN:
                {
                    output.Enqueue(token);
                    auto *t = functions.Get(token->identifier);
                    printf("%s", t->identifier->cStr());
                    break;
                }
            case FUNCTION_DEF_END:
            {
                output.Enqueue(token);
                break;
            }
            case FOR_UPDATE_END:
            case WHILE_BLOCK_END: case CASE_BLOCK_END: case SWITCH_END: case FOR_COND_END: case FOR_INIT_END:
            case FOR_BLOCK_END: case IF_BLOCK_END: case IF_COND_END: case ELSE_BLOCK_END:
            case WHILE_COND_END: case SWITCH_COND_END:
                while( ops.top() != 0 )
                {
                    output.Enqueue(ops.pop_back());
                }
                output.Enqueue(token);
                break;
            case FUNCTION_CALL_BEGIN: //Function's value used in an expression.
            {
                auto id = U8String(token->identifier);
                if ( standardFunctions.Exists(&id) )
                {
                    token->value->opcode = JBF;
                }
                else
                {
                    token->value->opcode = JSR;
                }
                //queue function call begin
                output.Enqueue(token);
                break;
            }
            case COLLECTION_ADDRESS:
                {
                    while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN &&
                           (GET_BP(token->type) < ops.peek(1)->bp() ||
                            (token->bp() == ops.peek(1)->bp() && token->is_left_assoc())))
                    {
                        output.Enqueue(ops.pop_back());
                    }
                    output.Enqueue(token);
                }
                break;
            case VARIABLE_DEF: case VARIABLE_VALUE: case PARAMETER_VALUE: case COLLECTION_VALUE:
            case VARIABLE_ADDRESS:  //variable address is higher priority than anything else and only
                output.Enqueue(token);
                break;
            case PARAM_BEGIN:
            {
                auto *tmp = new Token(token);
                tmp->type = OPEN_PAREN;
                ops.push_back(tmp);
            }
            case OPEN_PAREN:
                ops.push_back(token);
                break;
            case PARAM_END:
            case CLOSE_PAREN:
                while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN)
                {
                    output.Enqueue(ops.pop_back());
                }
                if (ops.top() != 0 && ops.peek(1)->type == OPEN_PAREN)
                {
                    ops.pop_back();
                }
                break;
            default:
                if (token->is_value())
                {
                    output.Enqueue(token);
                }
                else if (token->is_unary())
                {
                    while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN && token->bp() < ops.peek(1)->bp())
                    {
                        output.Enqueue(ops.pop_back());
                    }
                    ops.push_back(token);
                }
                else if (token->is_binary())
                {
                    while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN && (token->bp() < ops.peek(1)->bp() ||
                                                                                 (token->bp() == ops.peek(1)->bp() && token->is_left_assoc())))
                    {
                        output.Enqueue(ops.pop_back());
                    }
                    ops.push_back(token);
                }
                else if (token->is_terminal())
                {
                    while( ops.top() != 0 )
                    {
                        output.Enqueue(ops.pop_back());
                    }
                }
                break;
        }
        position++;
        if ( position < tokens.Count() )
        {
            token = tokens[position];
        }
    }

    while (ops.top() != 0)
    {
        output.Enqueue(ops.pop_back());
    }

    return token;
}

/// \desc Parses expressions up to the point where rbp right binding power or precedence
///       is greater than or equal to the left terms binding power, precedence.
Token *Parser::Expression(int64_t tokenLocation)
{
    Token *token = Peek(0);
    token = ShuntingYard(token, tokenLocation);

    if ( parserInfoLevel == 2 )
    {
        printf("\nExpression Output:\n");
        for(int64_t ii=0; ii<output.Count(); ++ii)
        {
            DisplayTokenAsText(ii, output.Peek(ii)->type);
        }
        printf("\n");
    }

    Token *lastVariable; //used for prefix operations

    int64_t lastModuleId = output.Peek()->value->moduleId;

    //Generate the run time code.
    while( !output.IsEmpty() )
    {
        Token *currentToken = output.Dequeue();
        if ( lastModuleId != currentToken->value->moduleId )
        {
            OutputCode(currentToken, CID);
        }
        switch(currentToken->type)
        {
            case WHILE_COND_BEGIN:
            {
                breakableTokens.push_back(currentToken);
                int64_t jmp = jumpLocations.pop_back();
                jumpLocations.push_back(jmp);
                program[jmp]->location = program.Count();
                if ( continueLocations.Count() > 0 )
                {
                    program[continueLocations.pop_back()]->location = program.Count();
                }
                break;
            }
            case WHILE_COND_END:
            {
                auto *tmp = new Token(token);
                tmp->value->location = jumpLocations.pop_back() + 1;
                OutputCode(tmp, JIT);
                if (breakLocations.Count() > 0)
                {
                    program[breakLocations.pop_back()]->location = program.Count();
                }
                break;
            }
            case WHILE_BLOCK_BEGIN:
            {
                //Add initial jump to condition.
                jumpLocations.push_back(program.Count());
                OutputCode(token, JMP);
            }
            case WHILE_BLOCK_END:
            {
                break;
            }
            case SWITCH_BEGIN:
            {
                currentToken->switchCaseIndex = 0;
                switches.push_back(currentToken);
                breakableTokens.push_back(currentToken);
                break;
            }
            case SWITCH_END:
            {
                auto *t = switches.pop_back();
                program[t->switchIndex]->location = program.Count();

                Token *bt = breakableTokens.pop_back();
                while( bt->breakLocations.Count() > 0 )
                {
                    program[bt->breakLocations.pop_back()]->location = program.Count();
                }
                break;
            }
            case SWITCH_COND_BEGIN:
                break;
            case FUNCTION_CALL_BEGIN:
                functionCalls.push_back(new Token(currentToken));
                break;
            case FUNCTION_CALL_END:
            {
                auto *fc = functionCalls.pop_back();
                auto id = U8String(fc->identifier);
                if ( fc->value->opcode == JSR )
                {
                    Token *funInfo = functions.Get(&id);
                    fc->value->location = funInfo->value->location;
                }

                //Add parameter count.
                OutputCount(fc->value->iValue, fc->value->moduleId);

                if ( standardFunctions.Exists(&id) )
                {
                    OutputCode(fc, JBF);
                }
                else
                {
                    OutputCode(fc, JSR);
                }
                break;
            }
            case SWITCH_COND_END:
            {
                auto *t = switches.pop_back();
                t->switchIndex = program.Count();
                OutputCode(t, JTB);
                switches.push_back(t);
                break;
            }
            case CASE_BLOCK_BEGIN:
            {
                auto *t = switches.pop_back();
                program[t->switchIndex]->cases[t->switchCaseIndex]->location = program.Count();
                switches.push_back(t);
                break;
            }
            case CASE_BLOCK_END:
            {
                auto *t = switches.pop_back();
                t->switchCaseIndex++;
                switches.push_back(t);
                break;
            }
            case PREFIX_DEC: case PREFIX_INC: case POSTFIX_DEC: case POSTFIX_INC:
            {
                auto *variable = GetVariableInfo(currentToken);
                auto *tmp = new Token(variable);
                tmp->type = currentToken->type;
                tmp->value->type = currentToken->type;
                tmp->value->opcode = currentToken->value->opcode;
                OutputCode(tmp, tmp->value->opcode);
                break;
            }
            case PARAMETER_VALUE:
            {
                OutputCode(currentToken, PSL);
                break;
            }
            case VARIABLE_DEF:
            {
                CreateVariable(currentToken);
                break;
            }
            case COLLECTION_ADDRESS:
            {
                auto *tmp = new Token(currentToken);
                auto *variable = GetVariableInfo(currentToken);
                tmp->value->operand = variable->value->operand;
                OutputCode(tmp, tmp->value->opcode);
                break;
            }
            case VARIABLE_ADDRESS:
            {
                OutputCode(currentToken, PVA);
                break;
            }
            case VARIABLE_VALUE: case COLLECTION_VALUE:
            {
                lastVariable = currentToken;
                if ( !output.IsEmpty() )
                {
                    TokenTypes type = output.Peek()->type;
                    //prefix and postfix operations directly increment the variable.
                    if ( type == PREFIX_INC || type == PREFIX_DEC || type == POSTFIX_INC || type == POSTFIX_DEC )
                    {
                        break;
                    }
                }
                PushValue(currentToken);
                break;
            }
            case ASSIGNMENT: case ADD_ASSIGNMENT: case SUBTRACT_ASSIGNMENT: case MULTIPLY_ASSIGNMENT:
            case DIVIDE_ASSIGNMENT: case MODULO_ASSIGNMENT:
            {
                Token *variable = GetVariableInfo(currentToken);
                //If token type is a compound assignment add the operation before the assignment.
                if (currentToken->type != ASSIGNMENT)
                {
                    CreateOperation(currentToken);
                }
                if (currentToken->type == ADD_ASSIGNMENT || currentToken->type == SUBTRACT_ASSIGNMENT ||
                    currentToken->type == MULTIPLY_ASSIGNMENT || currentToken->type == DIVIDE_ASSIGNMENT ||
                    currentToken->type == MODULO_ASSIGNMENT )
                {
                    break;
                }
                auto *tmp = new Token(currentToken);
                tmp->value = new DslValue(variable->value);
                if ( variable->modifier == TMLocalScope )
                {
                    tmp->value->opcode = SLV;
                }
                else
                {
                    tmp->value->opcode = SAV;
                }
                OutputCode(tmp, tmp->value->opcode);
                break;
            }
            case BREAK:
            {
                Token *t = breakableTokens.pop_back();
                t->breakLocations.push_back(program.Count());
                breakableTokens.push_back(t);
                OutputCode(currentToken, JMP);
                break;
            }
            case FUNCTION_DEF_BEGIN:
            {
                jumpLocations.push_back(program.Count());
                OutputCode(currentToken, JMP);
                Token *funInfo = functions.Get(currentToken->identifier);
                funInfo->value->location = program.Count();
                functions.Set(currentToken->identifier, funInfo);
                break;
            }
            case FUNCTION_DEF_END:
                OutputCode(currentToken, RET);
                program[jumpLocations.pop_back()]->location = program.Count();
                break;
            case IF_BLOCK_BEGIN: case IF_COND_BEGIN:
                break;
            case IF_COND_END:
                ifJumpLocations.push_back(program.Count());
                OutputCode(currentToken, JIF);
                break;
            case IF_BLOCK_END:
            {
                int64_t jmp = ifJumpLocations.pop_back();
                //update jump position to correct position after block end
                if ( !output.IsEmpty() && output.Peek(0)->type == ELSE_BLOCK_BEGIN )
                {
                    program[jmp]->location = program.Count() + 1;
                }
                else
                {
                    program[jmp]->location = program.Count();
                }
                break;
            }
            case ELSE_BLOCK_BEGIN:
                ifJumpLocations.push_back(program.Count());
                OutputCode(token, JMP);
                break;
            case ELSE_BLOCK_END:
                program[ifJumpLocations.pop_back()]->location = program.Count();
                break;
            case CONTINUE:
                continueLocations.push_back(program.Count());
                OutputCode(token, JMP);
                break;
            case FOR_COND_BEGIN:
                jumpLocations.push_back(program.Count());
                break;
            case FOR_COND_END:
                jumpLocations.push_back(program.Count());
                OutputCode(token, JIF);
                break;
            case FOR_BLOCK_BEGIN: case FOR_BLOCK_END: case PARAM_BEGIN: case PARAM_END:
            case FOR_INIT_BEGIN: case FOR_INIT_END:
            case FOR_UPDATE_BEGIN:
                break;
            case FOR_UPDATE_END:
            {
                //Update conditional jump to exit
                program[jumpLocations.pop_back()]->location = program.Count() + 1;
                if (continueLocations.Count() > 0)
                {
                    program[continueLocations.pop_back()]->location = program.Count() + 1;
                }
                if (breakLocations.Count() > 0)
                {
                    program[breakLocations.pop_back()]->location = program.Count() + 1;
                }
                //Set jump back to conditional check
                auto *tmp = new Token(token);
                tmp->value->location = jumpLocations.pop_back();
                OutputCode(tmp, JMP);
                break;
            }
            case EVENT_RETURN:
                OutputCode(token, RFE);
                break;
            case RETURN:
                OutputCode(token, RET);
                break;
            default:
                if ( currentToken->is_value() )
                {
                    PushValue(currentToken);
                    break;
                }
                else if ( currentToken->is_op() )
                {
                    //else some other operator
                    CreateOperation(currentToken);
                    break;
                }
                fatal = true;
                printf("Token in output queue but not processed: %s\n",
                       tokenNames[(int64_t)currentToken->type & 0xFF]);
                break;
        }
    }

    return token;
}
#pragma clang diagnostic pop