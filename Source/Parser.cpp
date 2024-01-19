#include "../Includes/Opcodes.h"
#include "../Includes/parser.h"

extern void DisplayTokenAsText(int64_t index, TokenTypes type);

enum Operations
{
    OperationCode,
    Value,
    Variable,
    Coll,

};

/// \desc Advances the tok position and returns the token at the new position.
Token *Parser::Advance(int64_t offset)
{
    int64_t pos = position + offset;
    if ( !IsPositionInRange(pos) )
    {
        return &end;
    }
    position = pos;

    return tokens[position];
}

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

Token *Parser::GetVariableInfo(Token *token)
{
    Token *variable = variables.Get(token->identifier);
    if ( variable == nullptr )
    {
        PrintIssue(3000,
                   "Parser error, variable does not exist, this indicates an error in the lexer "
                   "as it did not catch the undefined variable.",
                   true, true);
        return nullptr;
    }

    return variable;
}

DslValue *Parser::OutputCount(int64_t count, int64_t moduleId)
{
    auto *tokenCount = new Token(new DslValue((int64_t)count));
    tokenCount->value->moduleId = moduleId;
    return OutputCode(tokenCount, PSI);
}

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
        case PSL: case PSV: case PCV: case PVA:
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
            value = new DslValue(JTB, token->totalCases, token->totalCases);
            value->moduleId = token->value->moduleId;
            if ( !program.push_back(value) )
            {
                return nullptr;
            }
            break;
            case NOP: case RET: case JMP: case JIF: case JIT: case END: case EFI:
            case EXP: case NEG: case NOT: case MUL: case DIV: case MOD: case ADA: case SUA:
            case MUA: case DIA: case MOA: case ADD: case SUB: case SVL: case SVR: case TLS:
            case TLE: case TGR: case TGE: case TEQ: case TNE: case BND: case XOR: case BOR:
            case AND: case LOR: case CTI: case CTD: case CTC: case CTS: case CTB:
            value = new DslValue(token->value);
            value->opcode = opcode;
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
    DslValue *value;

    //If is a numeric value
    if ( token->type != VARIABLE_VALUE &&
         token->type != VARIABLE_ADDRESS &&
         token->type != COLLECTION_VALUE &&
         token->type != COLLECTION_ADDRESS )
    {
        OutputCode(token, PSI);
        return;
    }

    //This is a variable or collection.
    //Note: Variable has the location of the defined variable in the generated program.
    //This is true for collections as well. Event in the non-static initialization case
    //the collection element will have a default value added as a placeholder.
    OutputCode(token, token->value->opcode);
}

/// \desc Creates a variable definition within the output IL and sets the tokens operand 1 to the location
///       of the variable within the programs IL.
void Parser::CreateVariable(Token *token)
{
    auto *var = new DslValue(token->value);
    if ( token->modifier == TMLocalScope )
    {
        OutputCode(token, DFL);
    }
    OutputCode(token, DEF);
}

/// \desc Parses a function call.
/// \param token The current token, which is a FUNCTION_CALL_BEGIN
/// \return The token after the FUNCTION_CALL_END
/// \remark FUNCTION_CALL_BEGIN PARAM_BEGIN  PARAM_END ... FUNCTION_CALL_END
Token *Parser::ProcessFunctionCall(Token *token)
{
    auto id = U8String(token->identifier);

    //count is number of parameters passed to function call.
    auto totalParams = (int64_t)token->value->iValue;

    Token *next = Advance(); //skip FUNCTION_CALL_BEGIN

    for(int64_t ii=0; ii<totalParams; ++ii)
    {
        //Note: Expression always leaves the solution on the top of the params stack
        next = Expression(EXIT_PARAM_END);
        if ( next->type == PARAM_END )
        {
            next = Advance();
        }
    }

    //Add parameter count.
    if ( OutputCount(totalParams, token->value->moduleId) == nullptr )
    {
        return next;
    }

    if ( standardFunctions.Exists(&id) )
    {
        if ( !OutputCode(token, JBF) )
        {
            return next;
        }
    }
    else
    {
        if ( !OutputCode(token, JSR) )
        {
            return next;
        }
    }

    return next;
}

Token *Parser::ProcessSwitchStatement(Token *token)
{
    Expression(EXIT_SWITCH_COND_END);
    int64_t switchCasesStartIndex = position;
    auto *jumpTableInstruction = OutputCode(token, JTB);

    int64_t checkValuesStartIndex = program.Count();

    //Tracks the jumps to each case and default case.
    List<int64_t> jumpToExit;

    //Create the check values and jump locations.
    for(int64_t ii=0; ii<token->totalCases; ++ii)
    {
        while (Advance()->type != CASE_COND_BEGIN)
        {
        }

        OutputCode(Advance(), NOP);
    }

    //Add the openBlocks for each case
    position = switchCasesStartIndex;
    for(int64_t ii=0; ii<token->totalCases; ++ii)
    {
        while (Advance()->type != CASE_BLOCK_BEGIN)
        {
        }
        program[checkValuesStartIndex + ii]->location = program.Count();
        Token *tt = Expression(EXIT_CASE_BLOCK_END);
        jumpToExit.push_back(program.Count());
        OutputCode(tt, NOP);
    }

    //If there is a default case
    if ( token->defaultIndex >= 0 )
    {
        jumpTableInstruction->location = program.Count();
        position = token->defaultIndex;
        Expression(EXIT_DEFAULT_BLOCK_END);
    }
    else
    {
        jumpTableInstruction->location = program.Count();
    }

    //update the case conditional jump locations to the start of each case block.
    //note: default location is contained in the JTB instructions location field.
    for(int64_t ii=0; ii<jumpToExit.Count(); ++ii)
    {
        program[jumpToExit[ii]]->opcode = JMP;
        program[jumpToExit[ii]]->location = program.Count();
    }

    while (Advance()->type != SWITCH_END)
    {
    }

    return Advance();
}

Token *Parser::CreateOperation(Token *token)
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
    return token;
}

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

    //conditional Jump location
    List<int64_t> jumpLocations;
    List<int64_t> continueLocations;

    program.Clear();

    position = 0;

    auto *token = new Token();
    token->type = INVALID_TOKEN;
    token->value->moduleId = 1;
    Token *tmp;

    if ( tokens.Count() == 0 )
    {
        OutputCode(token, NOP);
        OutputCode(token, END);
    }
    else
    {
        OutputCode(token, NOP);

        while( token->type != END_OF_SCRIPT )
        {
            TokenTypes type = token->type;
            switch( type )
            {
                case VARIABLE_DEF:
                    CreateVariable(token);
                    token = Advance();
                    break;
                case FUNCTION_DEF_BEGIN:
                {
                    jumpLocations.push_back(program.Count());
                    OutputCode(token, JMP);
                    Token *funInfo = functions.Get(token->identifier);
                    funInfo->value->location = program.Count();
                    functions.Set(token->identifier, funInfo);
                    token = Advance();
                    break;
                }
                case FUNCTION_DEF_END:
                    OutputCode(token, RET);
                    program[jumpLocations.pop_back()]->location = program.Count();
                    token = Advance();
                    break;
                case FUNCTION_CALL_BEGIN:
                    token = ProcessFunctionCall(token);
                    break;
                case IF_COND_BEGIN:
                    token = Expression(EXIT_IF_COND_END);
                    break;
                case IF_COND_END:
                    jumpLocations.push_back(program.Count());
                    OutputCode(token, JIF);
                    token = Advance();
                    break;
                case IF_BLOCK_BEGIN:
                    token = Advance();
                    break;
                case IF_BLOCK_END:
                    //update jump location to correct location after block end
                    if ( Peek(1)->type == ELSE_BLOCK_BEGIN )
                    {
                        program[jumpLocations.pop_back()]->location = program.Count() + 1;
                    }
                    else
                    {
                        program[jumpLocations.pop_back()]->location = program.Count();
                    }
                    token = Advance();
                    break;
                case ELSE_BLOCK_BEGIN:
                    jumpLocations.push_back(program.Count());
                    OutputCode(token, JMP);
                    token = Advance();
                    break;
                case ELSE_BLOCK_END:
                    program[jumpLocations.pop_back()]->location = program.Count();
                    token = Advance();
                    break;
                case WHILE_COND_BEGIN:
                    program[jumpLocations.pop_back()]->location = program.Count();
                    if ( continueLocations.Count() > 0 )
                    {
                        program[continueLocations.pop_back()]->location = program.Count();
                    }
                    token = Expression(EXIT_WHILE_COND_END);
                    break;
                case WHILE_COND_END:
                    tmp = new Token(token);
                    tmp->value->location = jumpLocations.pop_back() + 1;
                    OutputCode(tmp, JIT);
                    token = Advance();
                    break;
                case WHILE_BLOCK_BEGIN:
                    //Add initial jump to condition.
                    jumpLocations.push_back(program.Count());
                    jumpLocations.push_back(program.Count());
                    OutputCode(token, JMP);
                    token = Advance();
                    break;
                case WHILE_BLOCK_END:
                    token = Advance();
                    break;
                case CONTINUE:
                    continueLocations.push_back(program.Count());
                    OutputCode(token, JMP);
                    token = Advance();
                    break;
                case FOR_INIT_BEGIN:
                    token = Expression(EXIT_FOR_INIT_END);
                    break;
                case FOR_INIT_END:
                    token = Advance();
                    break;
                case FOR_COND_BEGIN:
                    jumpLocations.push_back(program.Count());
                    token = Expression(EXIT_FOR_COND_END);
                    break;
                case FOR_COND_END:
                    jumpLocations.push_back(program.Count());
                    OutputCode(token, JIF);
                    token = Advance();
                    break;
                case FOR_BLOCK_BEGIN:
                case FOR_BLOCK_END:
                    token = Advance();
                    break;
                case FOR_UPDATE_BEGIN:
                    token = Expression(EXIT_FOR_UPDATE_END);
                    break;
                case FOR_UPDATE_END:
                    //Update conditional jump to exit
                    program[jumpLocations.pop_back()]->location = program.Count() + 1;
                    if ( continueLocations.Count() > 0 )
                    {
                        program[continueLocations.pop_back()]->location = program.Count() + 1;
                    }
                    //Set jump back to conditional check
                    tmp = new Token(token);
                    tmp->value->location = jumpLocations.pop_back();
                    OutputCode(tmp, JMP);
                    token = Advance();
                    break;
                case SWITCH_BEGIN:
                    token = ProcessSwitchStatement(token);
                    break;
                case FUNCTION_CALL_END:
                    token = Advance();
                    break;
                case RETURN:
                    token = Expression(EXIT_SEMICOLON_COMMA);
                    OutputCode(token, RET);
                    break;
                default:
                    token = Expression(EXIT_SEMICOLON_COMMA);
                    break;
            }
            if ( fatal )
            {
                return false;
            }
            if ( token->is_semicolon() || token->is_comma() )
            {
                token = Advance();
            }
        }
    }

    OutputCode(token, END);

    FixUpFunctionCalls();
    FixUpJumpsToEnd();

    //Add in default event handlers
    //EFI
    for(int ii=0; ii<modules.Count(); ++ii)
    {
        token = new Token();
        token->value->moduleId = ii+1;
        OutputCode(token, EFI);
    }

//
//
//    value = new DslValue(JBF, standardFunctions.Get(token->identifier)->value->operand);
//    value->moduleId = token->value->moduleId;
//    if ( !program.push_back(value) )
//    {
//        return nullptr;
//    }


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

/// \desc Checks if the shunting yard expression parser main loop should be exited.
/// \param parseParam True if expression is parsing a function call parameter, else false.
/// \param exitExpressionOn The tokens to look for to exit expression parsing.
/// \param type Current token type being parsed.
/// \return True if the shunting yard expression parse loop should be exited, else false.
bool Parser::ExitExpression(ExitExpressionOn exitExpressionOn, TokenTypes type)
{
    if ( type == END_OF_SCRIPT )
    {
        return true;
    }

    switch( exitExpressionOn )
    {
        case EXIT_FUNCTION_CALL_END:
            return type == FUNCTION_CALL_END;
        case EXIT_SEMICOLON_COMMA:
            return type == SEMICOLON || type == COMMA;
        case EXIT_PARAM_END:
            return type == PARAM_END;
        case EXIT_CLOSE_PAREN:
            return type == CLOSE_PAREN;
        case EXIT_IF_COND_END:
            return type == IF_COND_END;
        case EXIT_IF_BLOCK_END:
            return type == IF_BLOCK_END;
        case EXIT_ELSE_BLOCK_END:
            return type == ELSE_BLOCK_END;
        case EXIT_WHILE_COND_END:
            return type == WHILE_COND_END;
        case EXIT_WHILE_BLOCK_END:
            return type == WHILE_BLOCK_END;
        case EXIT_FOR_INIT_END:
            return type == FOR_INIT_END;
        case EXIT_FOR_COND_END:
            return type == FOR_COND_END;
        case EXIT_FOR_UPDATE_END:
            return type == FOR_UPDATE_END;
        case EXIT_FOR_BLOCK_END:
            return type == FOR_BLOCK_END;
        case EXIT_SWITCH_COND_END:
            return type == SWITCH_COND_END;
        case EXIT_SWITCH_BLOCK_END:
            return type == SWITCH_END;
        case EXIT_CASE_COND_END:
            return type == CASE_COND_END;
        case EXIT_CASE_BLOCK_END:
            return type == CASE_BLOCK_END;
        case EXIT_DEFAULT_BLOCK_END:
            return type == DEFAULT_BLOCK_END;
        case EXIT_FUNCTION_DEF_END:
            return type == FUNCTION_DEF_END;
    }

    return true;
}

/// \desc Shunting yard expression parser, takes the lexed tokens from an expression and
///       arranges them into an output queue in the correct order to be processed into
///       an ordered list ready for code generation.
/// \param exitExpressionOn Token types to look for to determine if expression parsing should exit.
/// \param token First token in the expression.
/// \param output Pointer to the output being filled in.
/// \return A pointer to the output queue containing the ordered token list.
Token *Parser::ShuntingYard(ExitExpressionOn exitExpressionOn, Token *token, Queue<Token *> *output)
{
    Stack<Token *> ops;

    //This loop forms the core of a shunting yard expression parser.
    while (!ExitExpression(exitExpressionOn, token->type))
    {
        switch (token->type)
        {
            case COLLECTION_ADDRESS:
                {
                    while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN &&
                           (GET_BP(token->type) < ops.peek(1)->bp() ||
                            (token->bp() == ops.peek(1)->bp() && token->is_left_assoc())))
                    {
                        output->Enqueue(ops.pop_back());
                    }
                    output->Enqueue(token);
                }
                break;
            case VARIABLE_ADDRESS:  //variable address is higher priority than anything else and only
                PushValue(token);   //appears on the left side of an assignment expression.
                break;
            default:
                if (token->is_value())
                {
                    output->Enqueue(token);
                }
                else if (token->is_unary())
                {
                    while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN && token->bp() < ops.peek(1)->bp())
                    {
                        output->Enqueue(ops.pop_back());
                    }
                    ops.push_back(token);
                }
                else if (token->is_binary())
                {
                    while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN && (token->bp() < ops.peek(1)->bp() ||
                           (token->bp() == ops.peek(1)->bp() && token->is_left_assoc())))
                    {
                        output->Enqueue(ops.pop_back());
                    }
                    ops.push_back(token);
                }
                break;
            case VARIABLE_DEF:
            case VARIABLE_VALUE:
            case PARAMETER_VALUE:
            case COLLECTION_VALUE:
                output->Enqueue(token);
                break;
            case FUNCTION_CALL_BEGIN:
                token = ProcessFunctionCall(token);
                break;
            case OPEN_PAREN:
                ops.push_back(token);
                break;
            case CLOSE_PAREN:
                while (ops.top() != 0 && ops.peek(1)->type != OPEN_PAREN)
                {
                    output->Enqueue(ops.pop_back());
                }
                if (ops.top() != 0 && ops.peek(1)->type == OPEN_PAREN)
                {
                    ops.pop_back();
                }
                break;
        }
        token = Advance();
    }

    while (ops.top() != 0)
    {
        output->Enqueue(ops.pop_back());
    }

    return token;
}

void Parser::IncrementOptimization()
{
    int64_t start = program.Count() - 4;
    //var = var + 1; //can be replaced with INC
    if ( start >= 0 )
    {
        if (program[start]->opcode == PSV &&
            program[start + 1]->opcode == PSV &&
            program[start + 2]->opcode == PSI &&
            program[start + 3]->opcode == ADD &&
            program[start]->operand == program[start + 1]->operand &&
            program[start + 2]->IsValue(1) )
        {
            program[start]->opcode = INC;
            program.Resize(start + 1);
        }
    }
}

/// \desc Parses expressions up to the point where rbp right binding power or precedence
///       is greater than or equal to the left terms binding power, precedence.
Token *Parser::Expression(ExitExpressionOn exitExpressionOn)
{
    Queue<Token *>    output;
    Stack<DslValue *> values;

    Token *token = Peek(0);
    token = ShuntingYard(exitExpressionOn, token, &output);

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

    //Generate the run time code.
    while( !output.IsEmpty() )
    {
        Token *t = output.Dequeue();
        if ( t->is_value() )
        {
            PushValue(t);
            continue;
        }
        else if ( t->type == FUNCTION_CALL_END )
        {
            continue;
        }
        else if ( t->type == FUNCTION_CALL_BEGIN )
        {
            //count is number of parameters passed to function call.
            auto totalParams = (int64_t)t->value->iValue;

            OutputCount(totalParams, token->value->moduleId);

            auto id = U8String(t->identifier);

            if ( standardFunctions.Exists(&id) )
            {
                program.push_back(new DslValue(JBF, standardFunctions.Get(&id)->value->operand));
                program[program.Count()-1]->moduleId = token->value->moduleId;
            }
            else
            {
                Token *funInfo = functions.Get(t->identifier);
                auto *dslValue = new DslValue(JSR);
                dslValue->variableName.CopyFrom(t->identifier);

                //if location is 0 will be fixed up post parse.
                dslValue->location = funInfo->value->location;
                program.push_back(dslValue);
                program[program.Count()-1]->moduleId = token->value->moduleId;
            }
            continue;
        }
        else if ( t->type == PREFIX_DEC || t->type == PREFIX_INC || t->type== POSTFIX_DEC || t->type == POSTFIX_INC )
        {
            Token *variable = GetVariableInfo(t);
            if ( variable == nullptr )
            {
                return nullptr;
            }
            auto *dslValue = new DslValue();
            dslValue->variableName.CopyFrom(variable->identifier);
            dslValue->operand = variable->value->operand;
            if ( t->type == PREFIX_DEC || t->type == POSTFIX_DEC)
            {
                if ( variable->modifier == TMLocalScope )
                {
                    dslValue->opcode = DEL;
                }
                else
                {
                    dslValue->opcode = DEC;
                }
            }
            else
            {
                if ( variable->modifier == TMLocalScope )
                {
                    dslValue->opcode = INL;
                }
                else
                {
                    dslValue->opcode = INC;
                }
            }
            program.push_back(dslValue);
            program[program.Count()-1]->moduleId = token->value->moduleId;
            if ( t->type == PREFIX_DEC || t->type == PREFIX_INC )
            {
                //If no more operations after dec or inc then the dec or inc is terminal for the expression.
                if (!output.IsEmpty())
                {
                    PushValue(lastVariable);
                }
            }
            continue;
        }
        else if ( t->type == PARAMETER_VALUE )
        {
            auto *dslValue = new DslValue(PSL, t->value->operand);
            dslValue->variableName.CopyFrom(t->identifier);
            program.push_back(dslValue);
            program[program.Count()-1]->moduleId = token->value->moduleId;
            continue;
        }
        else if ( t->type == VARIABLE_DEF )
        {
            CreateVariable(t);
            continue;
        }
        else if ( t->type == COLLECTION_ADDRESS )
        {
            Token *variable = GetVariableInfo(t);
            auto *ca = new DslValue(t->value);
            ca->operand = variable->value->operand;
            program.push_back(ca);
            continue;
        }
        else if ( t->type == VARIABLE_ADDRESS )
        {
            auto *d = new DslValue(t->value);
            d->opcode = PVA;
            program.push_back(d);
            continue;
        }
        else if ( t->type == VARIABLE_VALUE || t->type == COLLECTION_VALUE )
        {
            lastVariable = t;
            if ( !output.IsEmpty() )
            {
                TokenTypes type = output.Peek()->type;
                //prefix and postfix operations directly increment the variable.
                if ( type == PREFIX_INC || type == PREFIX_DEC || type == POSTFIX_INC || type == POSTFIX_DEC )
                {
                    continue;
                }
            }
            PushValue(t);
            continue;
        }
        else if (IS_ASSIGNMENT_TOKEN(t->type))
        {
            Token *variable = GetVariableInfo(t);
            //If token type is a compound assignment add the operation before the assignment.
            if ( t->type != ASSIGNMENT)
            {
                CreateOperation(t);
            }
            if ( t->type == ADD_ASSIGNMENT || t->type == SUBTRACT_ASSIGNMENT || t->type == MULTIPLY_ASSIGNMENT ||
                 t->type == DIVIDE_ASSIGNMENT || t->type == MODULO_ASSIGNMENT )
            {
                continue;
            }
            auto *dslValue = new DslValue(variable->value);
            if ( variable->modifier == TMLocalScope )
            {
                dslValue->opcode = SLV;
            }
            else
            {
                dslValue->opcode = SAV;
            }
            program.push_back(dslValue);
            program[program.Count()-1]->moduleId = token->value->moduleId;
            continue;
        }
        else if ( t->is_op() )
        {
            //else some other operator
            CreateOperation(t);
            continue;
        }
        fatal = true;
        printf("Lexer missed case should have caught the syntax issue\n");
        DisplayTokenAsText(-1, t->type);
    }

    IncrementOptimization();

    return token;
}