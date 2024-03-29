//
// Created by krw10 on 6/21/2023.
//
// This class stores and processes operations for variables and values.
//

#ifndef DSL_VALUE_H
#define DSL_VALUE_H

#include "../Includes/dsl_types.h"
#include "../Includes/ErrorProcessing.h"
#include "U8String.h"
#include "TokenTypes.h"
#include "Opcodes.h"
#include "Collection.h"
#include "ComponentData.h"
#include <cstdio>
#include <cstddef>
#include <malloc.h>

/// \desc Contains information about a single value in a code.
class DslValue
{
public:
    /// \desc Creates an empty DSL value.
    DslValue();

    /// \desc Creates a dsl value containing an integer.
    explicit DslValue(int64_t i);

    /// \desc Creates a dsl value containing a string.
    explicit DslValue(U8String *u8String);

    /// \desc Creates a DslValue containing an opcode and one or more operands.
    /// \param op Opcode that defines the action that should be taken.
    /// \param operand Additional information needed to process the instruction.
    /// \param location for jump instructions contains the position to jump to if the jump is taken.
    explicit DslValue(OPCODES op, int64_t operand = 0, int64_t location = 0);

    /// \desc Creates a DSL value from the one pointed.
    /// \param dslValue Pointer to the dsl value to be copied.
    explicit DslValue(DslValue *dslValue);

    /// \desc Frees up the sources used by the DslValue.
    ~DslValue()
        = default;

    /// \desc If an integer Type this field contains the value.
    int64_t iValue;

    /// \desc If a double Type this field contains the value.
    double dValue;

    /// \desc If a UTF8 character Type this field contains the value.
    u8chr cValue;

    /// \desc If a boolean value this field contains the value.
    bool bValue;

    /// \desc If a string Type this field contains the value.
    U8String sValue;

    /// \desc The Type of value stored in the struct. See ValueTypes.
    TokenTypes type;

    /// \desc If this is a variable then this field contains its _n, used for debugging.
    /// \remark This field is no longer needed once the code is generated.
    U8String variableName;

    /// \desc Variable name used in the script. This field is always present even when
    ///       used in the CPU runtime. This allows errors to be displayed with the
    ///       exact name entered in the script.
    U8String variableScriptName;

    /// \desc If this is an instruction this is its opcode.
    OPCODES opcode;

    /// \desc Additional info needed by the instruction.
    int64_t operand;

    /// \desc Contains the position to jump to if the opcode is a jump.
    int64_t location;

    /// \desc Indexes into the collection for collection elements.
    Collection indexes;

    /// \desc Points at the memory address of this variable, collection, or collection element. This is set
    ///       by push variable address and create variable. It is used when saving the variables value.
    DslValue *elementAddress;

    /// \desc Contains the assigned memory address of the variable or collection. Set when the variable is
    ///       created and is not changed.
    DslValue *address;

    /// \desc key name when parsing a json file.
    U8String jsonKey;

    /// \desc id of the module this instruction is part of.
    int64_t moduleId;

    /// \desc If this is a JTB instruction its cases are placed here. The default case if present is
    ///       placed at the end.
    List<DslValue *> cases;

    /// \desc Component definition if any.
    ComponentData *component;

    /// \desc Copies the right dslValue to this one.
    /// \param right DslValue containing the data to copy.
    void SAV(DslValue *right);

    /// \desc Performs a lite copy only copying the type
    ///       and the iValue, dValue, cValue, bValue, or sValue fields data.
    void LiteCopy(DslValue *right);

    /// \desc Converts this DslValue to the specified type.
    void Convert(TokenTypes valueType);

    /// \desc Performs the specified binary operation.
    void BinaryOperation(OPCODES op, DslValue *right);

    /// \desc Prints a key in a format compatible with how its used in a script.
    static void PrintKey(U8String *key);

    /// \desc Prints the value to the console.
    /// \param showEscapes True if escape codes should display as their \ code, else false.
    void Print(bool showEscapes);

    /// \desc Displays the contents of this dsl value to the console.
    /// \param showEscapes If false escape codes are shown as they would be if used in printf,
    ///                    If true the escape codes are shown as they appear in the code. For
    ///                    example, true would cause a line feed, while false would display
    ///                    the \n characters.
    void printItem(bool showEscapes);

    /// \desc Formats a single character into a json compatible format.
    /// \param ch character to format.
    /// \param out Pointer to U8String to output the formatted text to.
    static void FormatJsonCharacter(u8chr ch, U8String *out);

    /// \desc Formats a string into the correct json format for writing to a file.
    /// \param out Pointer to U8String to output the formatted text to.
    /// \param in Pointer to U8String containing the input string text.
    static void FormatJsonString(U8String *out, U8String *in);

    /// \desc Formats and appends the dsl value to the buffer.
    /// \param buffer Buffer to append the json formatted text to.
    void JsonAppendItemText(U8String *buffer);

    /// \desc Outputs the key when writing json format text.
    /// \param key Pointer to a U8String containing the key to write.
    /// \param buffer Pointer to a U8String to append the key text to.
    static void JsonAppendKeyName(U8String *key, U8String *buffer);

    /// \desc Returns the dsl value as a UTF8 string in the provided buffer.
    /// \param Pointer to the U8String buffer than contains the produced json text
    ///                representation of the dsl value.
    bool AppendAsJsonText(U8String *buffer);

    /// \desc Raises the value to the power of the right value.
    /// \param the right side term.
    void EXP(DslValue *right);

    /// \desc Convert the right value to this and then multiply this by right and update this.
    /// \param right The right side term.
    void MUL(DslValue *right);

    /// \desc Convert the right value to this and then divide this by right and update this.
    /// \param right The right side term.
    void DIV(DslValue *right);

    /// \desc Convert the right value to this and then modulo this by right and update this.
    /// \param right The right side term.
    void MOD(DslValue *right);

    /// \desc Convert the right value to this and then add this by right and update this.
    /// \param right The right side term.
    void ADD(DslValue *right);

    /// \desc Convert the right value to this and then subtract this by right and update this.
    /// \param right The right side term.
    void SUB(DslValue *right);

    /// \desc Convert the right value to this and then exclusive or this by right and update this.
    /// \param right The right side term.
    void XOR(DslValue *right);

    /// \desc Bitwise and the bits left the number of places in the right term.
    /// \param right The right side term.
    void BND(DslValue *right);

    /// \desc Bitwise or the bits right the number of places in the right term.
    /// \param right The right side term.
    void BOR(DslValue *right);

    /// \desc Convert the right value to this and then shift right assign this by right and update this.
    /// \param right The right side term.
    void SVR(DslValue *right);

    /// \desc Convert the right value to this and then shift left assign this by right and update this.
    /// \param right The right side term.
    void SVL(DslValue *right);

    /// \desc Convert the right value to this and then increment this by right and update this.
    void INC();

    /// \desc Convert the right value to this and then decrement this by right and update this.
    void DEC();

    /// \desc Convert the right value to this and then logical not this by right and update this.
    void NOT();

    /// \desc Checks if right is equal to this index.
    /// \param right The right side term.
    void TEQ(DslValue *right);

    /// \desc Checks if this index is equal to this index.
    /// \param right side term.
    /// \return True if the values are equal, else false.
    /// \remark The right side term and left side terms are not changed by this call.
    bool IsEqual(DslValue *right);

    /// \desc Checks if this dslValue is equal to 0.
    /// \return True if equal to 0 else, false.
    bool IsZero();

    /// \desc Checks if right is not equal to this index.
    /// \param right The right side term.
    void TNE(DslValue *right);

    /// \desc Checks if right is greater to this index.
    /// \param right The right side term.
    void TGR(DslValue *right);

    /// \desc Checks if right is greater to this index.
    /// \param right The right side term.
    void TLS(DslValue *right);

    /// \desc Checks if right is greater or equal to this index.
    /// \param right The right side term.
    void TGE(DslValue *right);

    /// \desc Checks if right is less or equal to this index.
    /// \param right The right side term.
    /// \return True if this is less or equal to right.
    void TLE(DslValue *right);

    /// \desc Convert the right value to this and then logically AND this by right and update this.
    /// \param right The right side term of the modulo.
    void AND(DslValue *right);

    /// \desc Convert the right value to this and then logically OR this by right and update this.
    /// \param right The right side term of the modulo.
    void LOR(DslValue *right);

    /// \desc Invert this
    void NEG();

    /// \desc Gets the dsl values store in the provided collection.
    /// \param dslValue DslValue to get the collections dslValues.
    /// \param keyData Pointer to a caller supplied list that is filled in with the key data information
    ///                for the collection. The list remains unchanged if the dslValue does not contain
    ///                a collection.
    static void GetKeyData(DslValue *dslValue, List<KeyData *> *keyData);

    /// \desc Writes the contents of the dsl value to the supplied u8String buffer.
    /// \param out Reference to an output buffer of type u8String that receives the formatted value.
    /// \param addComma if true a trailing comma is added after the value.
    /// \returns A pointer to a const char * buffer in the U8String containing the text. This allows
    ///          the return parameter to be used in other calls directly as long as it is read and
    ///          not changed.
    const char *GetValueAsString(U8String *out, bool addComma);

private:

    /// \desc Prints the character to the console.
    /// \param showEscapes True if escape codes should display as their \ code, else false.
    static void DisplayCharacter(u8chr ch, bool showEscapes);

    /// \desc Copies the source collection to this one.
    /// \param source Pointer to the collection type dslValue to copy to this dsl value.
    void CopyCollection(DslValue *right);

    /// \desc Converts this type to an integer type.
    void ToInteger();

    /// \desc Converts this type to a double type.
    void ToDouble();

    /// \desc Converts this type to a UTF8 char type.
    void ToChar();

    /// \desc Converts this type of a U8String type.
    void ToString();

    /// \desc Converts this type to a boolean type.
    void ToBool();

    /// \desc Checks if the floating point double values are close to equality.
    static bool Approximately(double x, double y)
    {
        const double epsilon = 1e-5; //Max allowable difference any value <= to this value is considered equal.
        return std::abs(x - y) <= epsilon * std::abs(x);
    }
};

#endif //DSL_VALUE_H
