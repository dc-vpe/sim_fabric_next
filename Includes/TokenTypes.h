//
// Created by krw10 on 8/26/2023.
//
#include "../Includes/dsl_types.h"

#ifndef DSL_CPP_TOKEN_TYPES_H
#define DSL_CPP_TOKEN_TYPES_H
/// \desc Total modifiers defined in the TokenModifiers enum.
#define TOTAL_SCOPE_LEVELS 6

/// \desc Defines the level of access for variables and functions. The enum list
///       is arranged in the most restrictive to least restrictive order.
/// \remark defined here because Token stores the modifier.
enum TokenModifiers
{
    /// \desc Local scope restricts the variable access to the function in which it is defined.
    TMLocalScope = 0
    , /// \desc _s scope is the default for variables and functions. It allows access anywhere within the tok
    TMScriptScope = 1
    , /// \desc Global scope access for variables and functions from anywhere.
    TMGlobalScope = 2
};

/// \desc Defines the valid operational types for a language value.
enum DSL_TOKEN_OPERATIONS
{
    /// \desc represents no value in this bit field.
    NONE = (uint32_t) 0,

    //0 to 7 id
    //8 to 15 bp

    // Token Info

    /// \desc The value is a binary operator and takes 2 values in producing a single value out.
    BINARY          = (uint32_t) (0x00010000), // binary
    UNARY           = (uint32_t) (0x00020000), // unary
    //                                0x000F0000   // binary unary mask
    LEFT_TO_RIGHT   = (uint32_t) (0x00100000), // left associative
    RIGHT_TO_LEFT   = (uint32_t) (0x00200000), // right associative
    //                                0x00F00000   // associative mask
    TOKEN_OPERATOR  = (uint32_t) (0x01000000), // operator
    TOKEN_RES_WORD  = (uint32_t) (0x02000000), // reserved word
    TOKEN_STATEMENT = (uint32_t) (0x03000000), // statement
    TOKEN_VALUE     = (uint32_t) (0x04000000), // value
    TOKEN_FUNCTION  = (uint32_t) (0x05000000), // function call
    TOKEN_PARSER    = (uint32_t) (0x06000000), // token used by parser
    TOKEN_LEXER     = (uint32_t) (0x07000000), // token used by lexer
    //                            0x0F000000   // specific type mask
};

#define BINARY_UNARY_MASK (0x000F0000)
#define ASSOCIATIVITY_MASK (0x00F00000)
#define SPECIFIC_USE_MASK (0x0F000000)
#define BP_MASK           (0x0000FF00)
#define ID_MASK           (0x000000FF)

/// \desc Sets the binding power or precedence of an operator.
#define SET_BINDING_POWER(a) ((uint32_t)((a) << 8))

/// \desc Sets the id which uniquely identifies the Type of value.
#define SET_TOKEN_ID(a) ((a) & ID_MASK)

/// \desc Gets the binding power, i.e. precedence of the value.
#define GET_BP(token_type_bp)  (((token_type_bp)&BP_MASK)>>8)

/// \desc Checks if the token is an operator type.
#define IS_OPERATOR(token_op_type)      (((token_op_type)&SPECIFIC_USE_MASK) == TOKEN_OPERATOR)

/// \desc Checks if the value is a unary operator.
#define IS_UNARY_OPERATOR(token_kind)   (((token_kind)&BINARY_UNARY_MASK) == UNARY)

/// \desc Checks if the value is a binary operator.
#define IS_BINARY_OPERATOR(token_kind)   (((token_kind)&BINARY_UNARY_MASK) == BINARY)

/// \desc Gets a value indicating if the value is left associative.
#define IS_LEFT_ASSOC(token_assoc_type) (((token_assoc_type)&ASSOCIATIVITY_MASK) == LEFT_TO_RIGHT)

/// \desc Checks if the token type is a value type (int, double, char, string, bool)
#define IS_VALUE_TYPE(token_value_type)  (((token_value_type)&SPECIFIC_USE_MASK) == TOKEN_VALUE)

/// \desc array of the value names arranged in id order. There is one _n for each defined value.
extern const char *tokenNames[];

#define IS_ASSIGNMENT_TOKEN(token_type) ( (token_type) == ASSIGNMENT ||\
                                          (token_type) == ADD_ASSIGNMENT ||\
                                          (token_type) == SUBTRACT_ASSIGNMENT ||\
                                          (token_type) == MULTIPLY_ASSIGNMENT ||\
                                          (token_type) == DIVIDE_ASSIGNMENT ||\
                                          (token_type) == MODULO_ASSIGNMENT )


/// \desc Token types are a set of bit flags packed uint32_t value that defines all of the
/// attributes the parser needs to parse the value into an AST tree.
enum TokenTypes
{
      INVALID_TOKEN        = NONE            | NONE          | NONE   | SET_BINDING_POWER(0)   | SET_TOKEN_ID(0)
    , OPEN_BLOCK           = TOKEN_OPERATOR  | NONE          | NONE   | SET_BINDING_POWER(16)  | SET_TOKEN_ID(1)
    , CLOSE_BLOCK          = TOKEN_OPERATOR  | NONE          | NONE   | SET_BINDING_POWER(16)  | SET_TOKEN_ID(2)
    , OPEN_PAREN           = TOKEN_OPERATOR  | LEFT_TO_RIGHT | NONE   | SET_BINDING_POWER(15)  | SET_TOKEN_ID(3)
    , CLOSE_PAREN          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | NONE   | SET_BINDING_POWER(15)  | SET_TOKEN_ID(4)
    , OPEN_BRACE           = TOKEN_OPERATOR  | LEFT_TO_RIGHT | NONE   | SET_BINDING_POWER(15)  | SET_TOKEN_ID(5)
    , CLOSE_BRACE          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | NONE   | SET_BINDING_POWER(15)  | SET_TOKEN_ID(6)
    , EXPONENT             = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(14)  | SET_TOKEN_ID(7)
    , PREFIX_INC           = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(8)
    , PREFIX_DEC           = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(9)
    , UNARY_POSITIVE       = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(10)
    , UNARY_NEGATIVE       = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(11)
    , UNARY_NOT            = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(12)
    , CAST_TO_INT          = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(13)
    , CAST_TO_DBL          = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(14)
    , CAST_TO_CHR          = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(15)
    , CAST_TO_STR          = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(16)
    , CAST_TO_BOOL         = TOKEN_OPERATOR  | RIGHT_TO_LEFT | UNARY  | SET_BINDING_POWER(13)  | SET_TOKEN_ID(17)
    , MULTIPLY             = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(12)  | SET_TOKEN_ID(18)
    , DIVIDE               = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(12)  | SET_TOKEN_ID(19)
    , MODULO               = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(12)  | SET_TOKEN_ID(20)
    , ADDITION             = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(11)  | SET_TOKEN_ID(21)
    , SUBTRACTION          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(11)  | SET_TOKEN_ID(22)
    , BITWISE_SHIFT_LEFT   = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(10)  | SET_TOKEN_ID(23)
    , BITWISE_SHIFT_RIGHT  = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(10)  | SET_TOKEN_ID(24)
    , LESS_THAN            = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(9)   | SET_TOKEN_ID(25)
    , LESS_OR_EQUAL        = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(9)   | SET_TOKEN_ID(26)
    , GREATER_THAN         = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(9)   | SET_TOKEN_ID(27)
    , GREATER_OR_EQUAL     = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(9)   | SET_TOKEN_ID(28)
    , EQUAL_TO             = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(8)   | SET_TOKEN_ID(29)
    , NOT_EQUAL_TO         = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(8)   | SET_TOKEN_ID(30)
    , BITWISE_AND          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(7)   | SET_TOKEN_ID(31)
    , BITWISE_XOR          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(6)   | SET_TOKEN_ID(32)
    , BITWISE_OR           = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(5)   | SET_TOKEN_ID(33)
    , LOGICAL_AND          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(4)   | SET_TOKEN_ID(34)
    , LOGICAL_OR           = TOKEN_OPERATOR  | LEFT_TO_RIGHT | BINARY | SET_BINDING_POWER(4)   | SET_TOKEN_ID(35)
    , ASSIGNMENT           = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(36)
    , MULTIPLY_ASSIGNMENT  = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(37)
    , DIVIDE_ASSIGNMENT    = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(38)
    , MODULO_ASSIGNMENT    = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(39)
    , ADD_ASSIGNMENT       = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(40)
    , SUBTRACT_ASSIGNMENT  = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(41)
    , POSTFIX_INC          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | UNARY  | SET_BINDING_POWER(2)   | SET_TOKEN_ID(42)
    , POSTFIX_DEC          = TOKEN_OPERATOR  | LEFT_TO_RIGHT | UNARY  | SET_BINDING_POWER(2)   | SET_TOKEN_ID(43)
    , COMMA                = TOKEN_OPERATOR  | LEFT_TO_RIGHT | NONE   | SET_BINDING_POWER(1)   | SET_TOKEN_ID(44)
    , SEMICOLON            = TOKEN_OPERATOR  | LEFT_TO_RIGHT | NONE   | SET_BINDING_POWER(1)   | SET_TOKEN_ID(45)
    , END_OF_SCRIPT        = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(46)
    , ERROR_TOKEN          = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(47)
    , VAR                  = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(48)
    , CONST                = TOKEN_RES_WORD  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(49)
    , GLOBAL               = TOKEN_RES_WORD  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(50)
    , SCRIPT               = TOKEN_RES_WORD  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(51)
    , LOCAL                = TOKEN_RES_WORD  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(52)
    , BLOCK                = TOKEN_RES_WORD  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(53)
    , IF                   = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(54)
    , ELSE                 = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(55)
    , SWITCH               = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(56)
    , CASE                 = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(57)
    , DEFAULT              = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(58)
    , WHILE                = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(59)
    , FOR                  = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(60)
    , BREAK                = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(61)
    , CONTINUE             = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(62)
    , RETURN               = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(63)
    , INTEGER_VALUE        = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(64)
    , DOUBLE_VALUE         = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(65)
    , CHAR_VALUE           = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(66)
    , BOOL_VALUE           = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(67)
    , STRING_VALUE         = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(68)
    , MULTI_LINE_COMMENT   = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(69)
    , SINGLE_LINE_COMMENT  = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(70)
    , VARIABLE_DEF         = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(71)
    , VARIABLE_VALUE       = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(72)
    , BRK                  = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(73)
    , FALSE                = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(74)
    , TRUE                 = TOKEN_VALUE     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(75)
    , LEND                 = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(76)
    , IF_COND_BEGIN        = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(77)
    , IF_COND_END          = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(78)
    , IF_BLOCK_BEGIN       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(79)
    , IF_BLOCK_END         = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(80)
    , ELSE_BLOCK_BEGIN     = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(81)
    , ELSE_BLOCK_END       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(82)
    , WHILE_COND_BEGIN     = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(83)
    , WHILE_COND_END       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(84)
    , WHILE_BLOCK_BEGIN    = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(85)
    , WHILE_BLOCK_END      = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(86)
    , FOR_INIT_BEGIN       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(87)
    , FOR_INIT_END         = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(88)
    , FOR_COND_BEGIN       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(89)
    , FOR_COND_END         = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(90)
    , FOR_UPDATE_BEGIN     = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(91)
    , FOR_UPDATE_END       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(92)
    , FOR_BLOCK_BEGIN      = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(93)
    , FOR_BLOCK_END        = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(94)
    , SWITCH_COND_BEGIN    = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(95)
    , SWITCH_COND_END      = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(96)
    , SWITCH_BEGIN         = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(97)
    , SWITCH_END           = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(98)
    , CASE_COND_BEGIN      = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(99)
    , CASE_COND_END        = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(100)
    , CASE_BLOCK_BEGIN     = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(101)
    , CASE_BLOCK_END       = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(102)
    , DEFAULT_BLOCK_BEGIN  = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(103)
    , DEFAULT_BLOCK_END    = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(104)
    , FUNCTION_CALL        = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(105)
    , FUNCTION_DEF_BEGIN   = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(106)
    , FUNCTION_DEF_END     = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(107)
    , FUNCTION_PARAMETER   = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(108)
    , PARAMETER_VALUE      = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(109)
    , FUNCTION_CALL_BEGIN  = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(110)
    , PARAM_BEGIN          = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(111)
    , PARAM_END            = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(112)
    , FUNCTION_CALL_END    = TOKEN_FUNCTION  | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(113)
    , COLLECTION           = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(114)
    , REFERENCE            = TOKEN_OPERATOR  | RIGHT_TO_LEFT | BINARY | SET_BINDING_POWER(3)   | SET_TOKEN_ID(115)
    , COLLECTION_BEGIN     = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(116)
    , COLLECTION_END       = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(117)
    , COLLECTION_VALUE     = TOKEN_STATEMENT | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(118)
    , KEY_BEGIN            = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(119)
    , KEY_END              = TOKEN_PARSER    | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(120)
    , STOP                 = TOKEN_LEXER     | NONE          | NONE   | SET_BINDING_POWER(100) | SET_TOKEN_ID(120)
};

#endif //DSL_CPP_TOKEN_TYPES_H
