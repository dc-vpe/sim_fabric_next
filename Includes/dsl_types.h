#ifndef DSL_TYPES_H
#define DSL_TYPES_H

#include <cstddef>
#include <cstdint>

/// \desc Should debug info be added?
/// \remark Comment out to build without debug info, this is temporary, will need to modify
///         makefile to define this macro for debug builds.
#define ADD_DEBUG_INFO 1

//A byte is a single 8 bit unsigned value.
#ifdef Byte
#undef Byte
#endif
#define Byte unsigned char

//u8chr is a decoded 4 byte utf8 character.
#ifdef u8chr
#undef u8chr
#endif
#define u8chr uint32_t

/// Initial size to allocate for an identifier.
#ifdef ALLOC_BLOCK_SIZE
#undef ALLOC_BLOCK_SIZE
#endif
#define ALLOC_BLOCK_SIZE 256


#ifdef NEXT_BLOCK_SIZE
#undef NEXT_BLOCK_SIZE
#endif
#define NEXT_BLOCK_SIZE(len)  ((((len)/ALLOC_BLOCK_SIZE)+1) * ALLOC_BLOCK_SIZE)

#ifndef TO_UPPER_HEX
#undef TO_UPPER_HEX
#endif
#define TO_UPPER_HEX(a) ((a) & 0xDF)

/// \desc Max block size for statically returned character strings.
#ifdef MAX_STRING_LEN_SIZE
#undef MAX_STRING_LEN_SIZE
#endif
#define MAX_STRING_LEN_SIZE 1024

#endif