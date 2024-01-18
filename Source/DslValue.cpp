//
// Created by krw10 on 6/21/2023.
//

#include "../Includes/DslValue.h"
#include "../Includes/ParseData.h"
#include <cmath>

DslValue::DslValue()
{
    iValue = 0;
    dValue = 0;
    cValue = 0;
    bValue = false;
    sValue.Clear();
    variableName.Clear();
    variableScriptName.Clear();
    type   = INTEGER_VALUE;
    opcode   = NOP;
    operand  = 0;
    location = 0;
    indexes        = {};
    elementAddress = nullptr;
    address = nullptr;
    jsonKey        = {};
    moduleId = -1;
}

DslValue::DslValue(int64_t i)
{
    iValue = i;
    dValue = 0;
    cValue = 0;
    bValue = false;
    sValue.Clear();
    variableName.Clear();
    variableScriptName.Clear();
    type   = INTEGER_VALUE;
    opcode   = NOP;
    operand  = 0;
    location = 0;
    indexes        = {};
    elementAddress = nullptr;
    address = nullptr;
    jsonKey        = {};
    moduleId = -1;
}

DslValue::DslValue(OPCODES op, int64_t operand, int64_t location)
{
    iValue = 0;
    dValue = 0;
    cValue = 0;
    bValue = false;
    sValue.Clear();
    variableName.Clear();
    variableScriptName.Clear();
    type   = INTEGER_VALUE;
    opcode = op;
    this->operand  = operand;
    this->location = location;
    indexes        = {};
    elementAddress = nullptr;
    address = nullptr;
    jsonKey        = {};
    moduleId = -1;
}

DslValue::DslValue(DslValue *dslValue)
{
    type = dslValue->type;
    variableName.CopyFrom(&dslValue->variableName);
    variableScriptName.CopyFrom(&dslValue->variableScriptName);
    iValue = dslValue->iValue;
    dValue = dslValue->dValue;
    cValue = dslValue->cValue;
    sValue.CopyFrom(&dslValue->sValue);
    bValue = dslValue->bValue;
    opcode   = dslValue->opcode;
    operand  = dslValue->operand;
    location       = dslValue->location;
    elementAddress = dslValue->elementAddress;
    address = dslValue->address;
    indexes.CopyFrom(&dslValue->indexes);
    jsonKey.CopyFrom(&dslValue->jsonKey);
    moduleId = dslValue->moduleId;
}

DslValue::DslValue(U8String *u8String)
{
    type = STRING_VALUE;
    iValue = 0;
    dValue = 0.0;
    cValue = U8_NULL_CHR;
    sValue.CopyFrom(u8String);
    bValue = false;
    opcode   = NOP;
    operand  = 0;
    location = 0;
    indexes        = {};
    elementAddress = nullptr;
    address = nullptr;
    jsonKey        = {};
    moduleId = -1;
}

/// \desc Copies the right collection to this one.
/// \param right The copped to be copied to this one.
/// \remark The address is not updated by this call as this variable is
///         a different instance than the right one.
void DslValue::CopyCollection(DslValue *right)
{
    type = right->type;
    iValue = right->iValue;
    dValue = right->dValue;
    cValue = right->cValue;
    sValue.CopyFrom(&right->sValue);
    bValue = right->bValue;
    elementAddress = right->elementAddress;
    jsonKey.CopyFrom(&right->jsonKey);
    indexes.Clear();
    moduleId = right->moduleId;
    List<KeyData *> list;
    DslValue::GetKeyData(right, &list);
    for(int ii=0; ii<list.Count(); ++ii)
    {
        auto *tmpKey = (U8String *)list.get(ii)->Key();
        auto *tmpData = (DslValue *)list.get(ii)->Data();
        auto *value = new DslValue();
        value->CopyCollection(tmpData);
        indexes.Set(new U8String(tmpKey), value);
    }
}

void DslValue::ToInteger()
{
    switch( type )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->ToInteger();
            }
            break;
        }
        default:
        case INTEGER_VALUE:
            break;
        case DOUBLE_VALUE:
            iValue = (int64_t)dValue;
            break;
        case CHAR_VALUE:
            iValue = (int64_t)cValue;
            break;
        case STRING_VALUE:
            iValue = sValue.GetInt();
            break;
        case BOOL_VALUE:
            iValue = (int64_t)bValue;
            break;
    }
    type = INTEGER_VALUE;
}

void DslValue::ToDouble()
{
    switch( type )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->ToDouble();
            }
            break;
        }
        default:
        case DOUBLE_VALUE:
            break;
        case INTEGER_VALUE:
            dValue = (double)iValue;
            break;
        case CHAR_VALUE:
            dValue = (double)cValue;
            break;
        case STRING_VALUE:
            dValue = sValue.GetDouble();
            break;
        case BOOL_VALUE:
            dValue = (bValue == true) ? 1.0 : 0.0;
            break;
    }
    type = DOUBLE_VALUE;
}

void DslValue::ToChar()
{
    switch( type )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->ToChar();
            }
            break;
        }
        default:
        case CHAR_VALUE:
            break;
        case INTEGER_VALUE:
            cValue = (u8chr)iValue;
            break;
        case DOUBLE_VALUE:
            cValue = (u8chr)(int64_t)dValue;
            break;
        case STRING_VALUE:
            cValue = sValue.get(0);
            break;
        case BOOL_VALUE:
            cValue = (bValue == true) ? (u8chr)'T' : (u8chr)'F';
            break;
    }
    type = CHAR_VALUE;
}

void DslValue::ToString()
{
    switch( type )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->ToString();
            }
            break;
        }
        default:
        case STRING_VALUE:
            break;
        case INTEGER_VALUE:
            sValue.CopyFromInt(iValue);
            break;
        case DOUBLE_VALUE:
            sValue.CopyFromDouble(dValue);
            break;
        case CHAR_VALUE:
            sValue.set(0, cValue);
            break;
        case BOOL_VALUE:
            cValue = (bValue == true) ? (u8chr)'T' : (u8chr)'F';
            break;
    }
    type = STRING_VALUE;
}

void DslValue::ToBool()
{
    switch( type )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->ToBool();
            }
            break;
        }
        default:
        case BOOL_VALUE:
            break;
        case INTEGER_VALUE:
            bValue = (iValue != 0) ? "true" : "false";
            break;
        case DOUBLE_VALUE:
            bValue = (dValue != 0.0) ? "true" : "false";
            break;
        case CHAR_VALUE:
            bValue = (cValue != 'T') ? "true" : "false";
            break;
        case STRING_VALUE:
            bValue = sValue.IsEqual("true") ? true : false;
            break;
    }
    type = BOOL_VALUE;
}

void DslValue::Convert(TokenTypes valueType)
{
    if ( type == valueType )
    {
        return;
    }
    switch( valueType )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->Convert(valueType);
            }
            break;
        }
        default:
        case INTEGER_VALUE:
            ToInteger();
            break;
        case DOUBLE_VALUE:
            ToDouble();
            break;
        case CHAR_VALUE:
            ToChar();
            break;
        case STRING_VALUE:
            ToString();
            break;
        case BOOL_VALUE:
            ToBool();
            break;
    }
}

void DslValue::BinaryOperation(OPCODES op, DslValue *right)
{
    if ( type == COLLECTION )
    {
        if ( right->type != COLLECTION )
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->BinaryOperation(op, right);
            }
            return;
        }
        else
        {
            List<KeyData *> keyDataLeft = indexes.GetKeyData();
            List<KeyData *> keyDataRight = right->indexes.GetKeyData();
            if ( keyDataLeft.Count() != keyDataRight.Count() )
            {
                PrintIssue(4001,
                           true, false, "Collection %s is not the same size as collection %s.",
                           variableScriptName.cStr(), right->variableScriptName.cStr());
                return;
            }
            for(int ii=0; ii<keyDataLeft.Count(); ++ii)
            {
                auto *subKeyDataLeft  = (DslValue *)keyDataLeft[ii]->Data();
                auto *subKeyDataRight = (DslValue *)keyDataRight[ii]->Data();
                if (subKeyDataLeft->type != subKeyDataRight->type )
                {
                    PrintIssue(4002,
                               true, false,
                               "Collection %s contains different information types than collection %s.",
                               subKeyDataLeft->variableScriptName.cStr(), subKeyDataRight->variableScriptName.cStr());
                    return;
                }
            }
            for(int ii=0; ii<keyDataLeft.Count(); ++ii)
            {
                auto *leftDslValue  = (DslValue *) keyDataLeft[ii]->Data();
                auto *rightDslValue = (DslValue *) keyDataRight[ii]->Data();
                leftDslValue->BinaryOperation(op, rightDslValue);
            }
        }
    }
    else
    {
        switch( op )
        {
            default:
                break;
            case ::EXP: EXP(right); return;
            case ::MUL: MUL(right); return;
            case ::DIV: DIV(right); return;
            case ::ADD: ADD(right); return;
            case ::SUB: SUB(right); return;
            case ::MOD: MOD(right); return;
            case ::XOR: XOR(right); return;
            case ::BND: BND(right); return;
            case ::BOR: BOR(right); return;
            case ::SVL: SVL(right); return;
            case ::SVR: SVR(right); return;
            case ::TEQ: TEQ(right); return;
            case ::TNE: TNE(right); return;
            case ::TGR: TGR(right); return;
            case ::TGE: TGE(right); return;
            case ::TLS: TLS(right); return;
            case ::TLE: TLE(right); return;
            case ::AND: AND(right); return;
            case ::LOR: LOR(right); return;
        }
    }
}

void DslValue::SAV(DslValue *right)
{
    if ( right->type == COLLECTION )
    {
        CopyCollection(right);
    }
    else
    {
        LiteCopy(right);
    }
}

void DslValue::EXP(DslValue *right)
{
    switch( type )
    {
        case COLLECTION:
            BinaryOperation(::EXP, right);
            break;
        default:
        case BOOL_VALUE:
        case STRING_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue = (int64_t)pow((double)iValue, (double)right->iValue);
            break;
        case DOUBLE_VALUE:
            right->Convert(type);
            dValue = pow((double)dValue, (double)right->dValue);
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue = (u8chr)pow((double)cValue, (double)right->cValue);
            break;
    }
}

void DslValue::MUL(DslValue *right)
{
    switch( type )
    {
        case COLLECTION:
            BinaryOperation(::MUL, right);
            break;
        default:
        case STRING_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue *= right->iValue;
            break;
        case DOUBLE_VALUE:
            right->Convert(type);
            dValue *= right->dValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue *= right->cValue;
            break;
    }
}

void DslValue::DIV(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::DIV, right);
            break;
        default:
        case STRING_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            if ( !right->IsZero() )
            {
                iValue /= right->iValue;
            }
            else
            {
                PrintIssue(4003, "Divide by zero, result is undefined.", true, true);
            }
            break;
        case DOUBLE_VALUE:
            right->Convert(type);
            if ( !right->IsZero() )
            {
                dValue /= right->dValue;
            }
            else
            {
                PrintIssue(4003, "Divide by zero, result is undefined.", true, true);
            }
            break;
        case CHAR_VALUE:
            right->Convert(type);
            if ( !right->IsZero() )
            {
                right->cValue /= right->cValue;
            }
            else
            {
                PrintIssue(4003, "Divide by zero, result is undefined.", true, true);
            }
            break;
    }
}

void DslValue::MOD(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::MOD, right);
            break;
        default:
        case STRING_VALUE:
        case BOOL_VALUE:
        case DOUBLE_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            if ( !right->IsZero() )
            {
                iValue %= right->iValue;
            }
            else
            {
                PrintIssue(4004, "Mod by zero, result is undefined.", true, true);
            }
            break;
        case CHAR_VALUE:
            right->Convert(type);
            if ( !right->IsZero() )
            {
                cValue %= right->cValue;
            }
            else
            {
                PrintIssue(4004, "Mod by zero, result is undefined.", true, true);
            }
            break;
    }
}

void DslValue::ADD(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::ADD, right);
            break;
        default:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue += right->iValue;
            break;
        case DOUBLE_VALUE:
            right->Convert(type);
            dValue += right->dValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue += right->cValue;
            break;
        case STRING_VALUE:
            right->Convert(type);
            sValue.push_back(&right->sValue);
            break;
    }
}

void DslValue::SUB(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::SUB, right);
            break;
        default:
        case STRING_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue -= right->iValue;
            break;
        case DOUBLE_VALUE:
            right->Convert(type);
            dValue -= right->dValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue -= right->cValue;
            break;
    }
}

void DslValue::XOR(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::XOR, right);
            break;
        default:
        case STRING_VALUE:
        case DOUBLE_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue ^= right->iValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue ^= right->cValue;
            break;
    }
}

void DslValue::BND(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::BND, right);
            break;
        default:
        case STRING_VALUE:
        case DOUBLE_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue &= right->iValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue &= right->cValue;
            break;
    }
}

void DslValue::BOR(DslValue *right)
{
    switch(type)
    {
        case COLLECTION:
            BinaryOperation(::BOR, right);
            break;
        default:
        case STRING_VALUE:
        case DOUBLE_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue |= right->iValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue |= right->cValue;
            break;
    }
}

void DslValue::SVR(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::SVR, right);
            break;
        default:
        case DOUBLE_VALUE:
        case STRING_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue >>= right->iValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue >>= right->cValue;
            break;
    }
}

void DslValue::SVL(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::SVL, right);
            break;
        default:
        case DOUBLE_VALUE:
        case STRING_VALUE:
        case BOOL_VALUE:
            ToInteger();
        case INTEGER_VALUE:
            right->Convert(type);
            iValue <<= right->iValue;
            break;
        case CHAR_VALUE:
            right->Convert(type);
            cValue <<= right->cValue;
            break;
    }
}

void DslValue::INC()
{
    switch(type)
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->INC();
            }
        }
        case STRING_VALUE:
        case BOOL_VALUE:
        default:
            break;
        case INTEGER_VALUE:
            ++iValue;
            break;
        case DOUBLE_VALUE:
            dValue += 1.0;
            break;
        case CHAR_VALUE:
            cValue++;
            break;
    }
}

void DslValue::DEC()
{
    switch(type)
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->DEC();
            }
        }
        case STRING_VALUE:
        case BOOL_VALUE:
        default:
            break;
        case INTEGER_VALUE:
            iValue--;
            break;
        case DOUBLE_VALUE:
            dValue -= 1.0;
            break;
        case CHAR_VALUE:
            cValue--;
            break;
    }
}

void DslValue::NOT()
{
    switch(type)
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->NOT();
            }
        }
        case DOUBLE_VALUE:
        case STRING_VALUE:
        default:
            break;
        case INTEGER_VALUE:
            iValue = !iValue;
            break;
        case CHAR_VALUE:
            cValue = !cValue;
            break;
        case BOOL_VALUE:
            bValue = !bValue;
            break;
    }
}

void DslValue::NEG()
{
    switch( type )
    {
        case COLLECTION:
        {
            List<KeyData *> keyData = indexes.GetKeyData();
            for (int ii = 0; ii < keyData.Count(); ++ii)
            {
                ((DslValue *) keyData[ii]->Data())->NEG();
            }
        }
        default:
        case INTEGER_VALUE:
            iValue *= -1;
            break;
        case DOUBLE_VALUE:
            dValue *= -1.0;
            break;
        case CHAR_VALUE:
            cValue *= -1;
            break;
        case STRING_VALUE:
        case BOOL_VALUE:
            printf("error can't negate strings or boolean values.");
            break;
    }
}

void DslValue::TEQ(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TEQ, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue == right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = dValue == right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue == right->cValue;
            break;
        case STRING_VALUE:
            bValue = sValue.IsEqual(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue == right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::TNE(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TNE, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue != right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = dValue != right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue != right->cValue;
            break;
        case STRING_VALUE:
            bValue = !sValue.IsEqual(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue != right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::TGR(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TGR, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue > right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = dValue > right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue > right->cValue;
            break;
        case STRING_VALUE:
            bValue = sValue.IsGreater(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue > right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::TGE(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TGE, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue >= right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = dValue >= right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue >= right->cValue;
            break;
        case STRING_VALUE:
            bValue = !sValue.IsLess(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue >= right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::TLE(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TLE, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue <= right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = dValue <= right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue <= right->cValue;
            break;
        case STRING_VALUE:
            bValue = !sValue.IsGreater(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue <= right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::TLS(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TLS, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue < right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = dValue < right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue < right->cValue;
            break;
        case STRING_VALUE:
            bValue = sValue.IsLess(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue < right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::AND(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::TEQ, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue && right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = (int64_t)dValue && (int64_t)right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue && right->cValue;
            break;
        case STRING_VALUE:
            bValue = sValue.IsEqual(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = bValue == right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

void DslValue::LOR(DslValue *right)
{
    switch (type)
    {
        case COLLECTION:
            BinaryOperation(::LOR, right);
            break;
        default:
            ToInteger();
        case INTEGER_VALUE:
            bValue = iValue || right->iValue;
            break;
        case DOUBLE_VALUE:
            bValue = (int64_t)dValue || (int64_t)right->dValue;
            break;
        case CHAR_VALUE:
            bValue = cValue || right->cValue;
            break;
        case STRING_VALUE:
            bValue = sValue.Count() > 0 || right->sValue.Count() > 0;
            break;
        case BOOL_VALUE:
            bValue = bValue || right->bValue;
            break;
    }

    type = BOOL_VALUE;
}

bool DslValue::IsEqual(DslValue *right)
{
    DslValue tmp;

    int value = ((int)(type == COLLECTION)) << 1 | ((int)(right->type == COLLECTION));

    switch( value )
    {
        default:
        case 0: //type != COLLECTION && right->type != COLLECTION
            if ( type == right->type )
            {
                switch(type)
                {
                    default:
                    case INTEGER_VALUE:
                        return iValue == right->iValue;
                    case DOUBLE_VALUE:
                        return dValue == right->dValue;
                    case CHAR_VALUE:
                        return cValue == right->cValue;
                    case STRING_VALUE:
                        return sValue.IsEqual(&right->sValue);
                    case BOOL_VALUE:
                        return bValue == right->bValue;
                }
            }
            else
            {
                tmp.LiteCopy(right);
                tmp.Convert(type);
                switch (type)
                {
                    default:
                    case INTEGER_VALUE:
                        return iValue == tmp.iValue;
                    case DOUBLE_VALUE:
                        return Approximately(dValue, tmp.dValue);
                    case CHAR_VALUE:
                        return cValue == tmp.cValue;
                    case STRING_VALUE:
                        return sValue.IsEqual(&tmp.sValue);
                    case BOOL_VALUE:
                        return bValue == tmp.bValue;
                }
            }
        case 1: //type != COLLECTION && right->type == COLLECTION
        case 2: //type == COLLECTION && right->type != COLLECTION
            return false;
        case 3: //type == COLLECTION && right->type == COLLECTION
            List<KeyData *> keyDataLeft = indexes.GetKeyData();
            List<KeyData *> keyDataRight = right->indexes.GetKeyData();
            if ( keyDataLeft.Count() != keyDataRight.Count() )
            {
                return false;
            }
            for(int ii=0; ii<keyDataLeft.Count(); ++ii)
            {
                if ( !((DslValue *)keyDataLeft[ii]->Data())->IsEqual((DslValue *)keyDataRight[ii]->Data()) )
                {
                    return false;
                }
            }
            return true;
    }
}

bool DslValue::IsZero()
{
    switch( type )
    {
        default:
            return false;
        case INTEGER_VALUE:
            return iValue == 0;
        case DOUBLE_VALUE:
            return Approximately(dValue, 0.0);
        case CHAR_VALUE:
            return cValue == U8_NULL_CHR;
        case STRING_VALUE:
            return sValue.Count() == 0;
        case BOOL_VALUE:
            return !bValue;
    }
}

void DslValue::LiteCopy(DslValue *right)
{
    type = right->type;
    switch(right->type)
    {
        default:
        case COLLECTION:
            CopyCollection(right);
            break;
        case INTEGER_VALUE:
            iValue = right->iValue;
            break;
        case DOUBLE_VALUE:
            dValue = right->dValue;
            break;
        case CHAR_VALUE:
            cValue = right->cValue;
            break;
        case STRING_VALUE:
            sValue.CopyFrom(&right->sValue);
            break;
        case BOOL_VALUE:
            bValue = right->bValue;
            break;
    }
}

void DslValue::DisplayCharacter(u8chr ch, bool showEscapes)
{
    if (showEscapes )
    {
        switch( ch )
        {
            default:
                putchar((int)ch);
                break;
            case '\n':
                putchar ('\\');
                putchar ('n');
                break;
            case '\r':
                putchar ('\\');
                putchar ('r');
                break;
            case '\t':
                putchar ('\\');
                putchar ('t');
                break;
            case '\b':
                putchar ('\\');
                putchar ('b');
                break;
            case '\f':
                putchar ('\\');
                putchar ('f');
                break;
        }
    }
    else
    {
        putchar((char)ch);
    }
}

void DslValue::PrintKey(U8String *key)
{
    if ( key->Count() >= 13 )
    {
        const char *ptr = key->cStr();
        switch( ptr[2] )
        {
            default:
                break;
            case 'G':
                ptr += 14;
                break;
            case 'S':
                ptr += 14;
                ptr = strchr(ptr, '.') + 1;
                break;
            case 'L':
                ptr += 13;
                break;
        }
        printf("\"%s\":", ptr);
        return;
    }

    printf("\"%s\":", key->cStr());

}

void DslValue::Print(bool showEscapes)
{
    if ( type == COLLECTION )
    {
        printf("{ ");
        List<KeyData *> list;
        DslValue::GetKeyData(this, &list);
        for(int ii=0; ii<list.Count(); ++ii)
        {
            auto *tmp = (DslValue *)list.get(ii)->Data();
            if ( tmp->type == COLLECTION )
            {
                printf("\"%s\":", list.get(ii)->Key()->cStr());
                tmp->Print(showEscapes);
            }
            else
            {
                PrintKey(list.get(ii)->Key());
                tmp->printItem(showEscapes);
            }
            if ( ii + 1 < list.Count() )
            {
                printf(", ");
            }
        }
        printf(" }");
    }
    else
    {
        printItem(showEscapes);
    }
}

bool DslValue::IsValue(int64_t value)
{
    switch( type )
    {
        default:
        case INTEGER_VALUE:
            return iValue = value;
        case DOUBLE_VALUE:
            return (int64_t)dValue == value;
        case STRING_VALUE:
            return sValue.GetInt() == value;
        case CHAR_VALUE:
            return cValue == value;
        case BOOL_VALUE:
            return bValue == value;
    }
}

void DslValue::printItem(bool showEscapes)
{
    switch( type )
    {
        default:
            printf("\n");
            return;
        case INTEGER_VALUE:
            printf("%ld", iValue);
            break;
        case DOUBLE_VALUE:
            printf("%f", dValue);
            break;
        case CHAR_VALUE:
            putchar('\'');
            DisplayCharacter(cValue, showEscapes);
            putchar('\'');
            break;
        case STRING_VALUE:
        {
            if ( !showEscapes )
            {
                printf("%s", sValue.cStr());
                break;
            }
            printf("\"");
            for(int64_t ii=0; ii< sValue.Count(); ++ii)
            {
                DisplayCharacter(sValue.get(ii), showEscapes);
            }
            printf("\"");
            break;
        }
        case BOOL_VALUE:
            if ( bValue )
            {
                printf("true");
            }
            else
            {
                printf("false");
            }
            break;
    }
}

void DslValue::GetKeyData(DslValue *dslValue, List<KeyData *> *keyData)
{
    if ( dslValue->type != COLLECTION )
    {
        return;
    }
    for(int ii=0; ii<dslValue->indexes.keys.Count(); ++ii)
    {
        keyData->push_back(dslValue->indexes.Get(dslValue->indexes.keys[ii]));
    }
}

void DslValue::FormatJsonCharacter(u8chr ch, U8String *out)
{
    if ( ch == '\"' || ch == '\\' || ch == '/' )
    {
        out->Append('\\');
        out->push_back(ch);
    }
    else if ( ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' )
    {
        out->push_back('\\');
        out->push_back(ch);
    }
    else if (ch < L' ' || ch > 126)
    {
        out->Append("\\u");
        for (int tt = 0; tt < 4; ++tt)
        {
            u8chr value = (ch >> 12) & 0xf;
            if (value <= 9)
            {
                out->push_back('0' + value);
            }
            else if (value <= 15)
            {
                out->push_back('A' + (value - 10));
            }
            ch <<= 4;
        }
    }
    else
    {
        out->push_back(ch);
    }
}

void DslValue::FormatJsonString(U8String *out, U8String *in)
{
    out->Append("\"");

    for(int ii=0; ii<in->Count(); ++ii)
    {
        u8chr ch = in->get(ii);
        FormatJsonCharacter(ch, out);
    }

    out->push_back('\"');
}

void DslValue::JsonAppendItemText(U8String *buffer)
{
    U8String tmp;

    switch( type )
    {
        default:
            tmp.printf((char *)"\n");
            buffer->Append(&tmp);
            return;
        case INTEGER_VALUE:
            tmp.printf((char *)"%ld", iValue);
            buffer->Append(&tmp);
            break;
        case DOUBLE_VALUE:
            tmp.printf((char *)"%f", dValue);
            buffer->Append(&tmp);
            break;
        case CHAR_VALUE:
            FormatJsonCharacter(cValue, buffer);
            break;
        case STRING_VALUE:
            FormatJsonString(buffer, &sValue);
            break;
        case BOOL_VALUE:
            if ( bValue )
            {
                tmp.CopyFromCString("true");
                buffer->Append(&tmp);
            }
            else
            {
                tmp.CopyFromCString("false");
                buffer->Append(&tmp);
            }
            break;
    }
}

void DslValue::JsonAppendKeyName(U8String *key, U8String *buffer)
{
    U8String tmp;

    if ( key->Count() >= 13 )
    {
        const char *ptr = key->cStr();
        switch( ptr[2] )
        {
            default:
                break;
            case 'G':
                ptr += 14;
                break;
            case 'S':
                ptr += 14;
                ptr = strchr(ptr, '.') + 1;
                break;
            case 'L':
                ptr += 13;
                break;
        }
        tmp.printf((char *)"\"%s\":", ptr);
        buffer->Append(&tmp);
        return;
    }

    tmp.printf((char *)"\"%s\":", key->cStr());
    buffer->Append(&tmp);
}

/// \desc Appends the dsl value to the end of the buffer as json formatted text.
bool DslValue::AppendAsJsonText(U8String *buffer)
{
    U8String tmp;

    if ( type == COLLECTION )
    {
        tmp.printf((char *)"{ ");
        buffer->Append(&tmp);
        List<KeyData *> list;
        DslValue::GetKeyData(this, &list);
        for(int ii=0; ii<list.Count(); ++ii)
        {
            auto *t = (DslValue *)list.get(ii)->Data();
            if ( t->type == COLLECTION )
            {
                tmp.printf((char *)"\"%s\":", list.get(ii)->Key()->cStr());
                t->AppendAsJsonText(buffer);
            }
            else
            {
                JsonAppendKeyName(list.get(ii)->Key(), buffer);
                t->JsonAppendItemText(buffer);
            }
            if ( ii + 1 < list.Count() )
            {
                tmp.printf((char *)", ");
                buffer->Append(&tmp);
            }
        }
        tmp.printf((char *)" }");
        buffer->Append(&tmp);
    }
    else
    {
        JsonAppendItemText(buffer);
    }

    return true;
}