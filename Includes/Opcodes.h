//
// Created by krw10 on 8/25/2023.
//

#ifndef DSL_CPP_OPCODES_H
#define DSL_CPP_OPCODES_H

enum OPCODES
{
    NOP = 1,//No operation  NOP
    DEF,    //Define a variable
    SAV,    //Save to variable
    EXP,    //Exponent
    MUL,    //multiply
    DIV,    //divide
    ADD,    //add
    SUB,    //subtract
    MOD,    //modulo
    XOR,    //exclusive or
    BND,    //bitwise and
    BOR,    //bitwise or
    INC,    //increment
    DEC,    //decrement
    NOT,    //invert
    NEG,    //multiply by -1
    SVL,    //shift left
    SVR,    //shift right
    CTI,    //convert to integer
    CTD,    //convert to double
    CTC,    //convert to char
    CTS,    //convert to string
    CTB,    //convert to bool
    JMP,    //Jump always
    JIF,    //Jump if bValue is false
    JIT,    //Jump if bValue is true
    JBF,    //Jump to native subroutine.
    JSR,    //Jump to script subroutine.
    RET,    //return to caller
    PSI,    //Push value
    PSV,    //push variable
    END,    //End program
    TEQ,    //test if equal
    TNE,    //test if not equal
    TGR,    //test if greater than
    TGE,    //test if greater than or equal
    TLS,    //test if less than
    TLE,    //test if less than or equal
    AND,    //logical and
    LOR,    //logical or
    JTB,    //Switch jump table, operand contains total entries.
    DFL,    //Define local variable
    PSL,    //Push local variable
    SLV,    //Save local variable
    PSP,    //Push parameter
    INL,    //Increment local variable
    DEL,    //Decrement local variable
    PCV,    //Push collection value
    PVA,    //Push variable address (works for variables and collections)
    ADA,    //Add assign.
    SUA,    //Subtract assign
    MUA,    //Multiply Assign
    DIA,    //Divide Assign
    MOA,    //Modulo Assign
    DCS,    //Save non-static result in a collection during definition
    EFI,    //Event function information
    RFE,    //Return from event.
    CID,    //Change module id.
};

#endif //DSL_CPP_OPCODES_H
