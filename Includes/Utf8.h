#ifndef DSL_UTF8_H
#define DSL_UTF8_H

#include "dsl_types.h"
#include <malloc.h>

#ifdef INVALID_UTF_CHAR
#undef INVALID_UTF_CHAR
#endif
///Internally defined invalid character used to indicate an error return when reading a character.
#define INVALID_UTF_CHAR ((u8chr)0xFFFFFFFF)

#ifdef END_OF_FILE_CHAR
#undef END_OF_FILE_CHAR
#endif
#define END_OF_FILE_CHAR ((u8chr)0x00000000)

#ifdef U8_NULL_CHR
#undef U8_NULL_CHR
#endif
#define U8_NULL_CHR ((u8chr)0x00000000)

#ifdef U8_ERROR_CHR
#undef U8_ERROR_CHR
#endif

/// \desc
/// Decodes the next character, from the buffer and reports any errors in e.
/// \param buf Pointer to the input characters to be processed.
/// \param c Pointer to the output character in utf8 compatible format.
/// \param e Pointer to the error return variable.
/// \return  Returns a pointer to the next character in buffer to be read. e will be non-zero if an error occurs.
/// \remark Branch-less decoder, so a dword 4 bytes will be read from the
/// buffer regardless of the actual length of the next character.
static Byte *utf8_decode(void *buf, u8chr *c, int64_t *e)
{
    static const char lengths[] =
    {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0,
        2, 2, 2, 2, 3, 3, 4, 0
    };

    static const uint32_t masks[]         = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    static const uint32_t minimumValues[] = {4194304, 0, 128, 2048, 65536};
    static const uint32_t shiftConsts[]   = {0, 18, 12, 6, 0};
    static const uint32_t shiftExtras[]   = {0, 6, 4, 2, 0};

    Byte *s = (Byte *) buf;
    auto len = (int64_t) (unsigned char)lengths[((int64_t) s[0]) >> 3];

    /* Compute the pointer to the next character early so that the next
     * iteration can start working on the next character. Neither Clang
     * nor GCC figure out this reordering on their own.
     */
    Byte *next = s + len + !len;

    /* Assume a four-byte character and load four bytes. Unused bits are
     * shifted out.
     */
    *c = (uint32_t) (s[0] & masks[len]) << 18;
    *c |= (uint32_t) (s[1] & 0x3f) << 12;
    *c |= (uint32_t) (s[2] & 0x3f) << 6;
    *c |= (uint32_t) (s[3] & 0x3f) << 0;
    *c >>= shiftConsts[len];

    /* Accumulate the various error conditions. */
    *e = (*c < minimumValues[len]) << 6; // non-canonical encoding
    *e |= ((*c >> 11) == 0x1b) << 7;  // surrogate half?
    *e |= (*c > 0x10FFFF) << 8;  // out of range?
    *e |= (s[1] & 0xc0) >> 2;
    *e |= (s[2] & 0xc0) >> 4;
    *e |= (s[3]) >> 6;
    *e ^= 0x2a; // stack_top two bits of each tail byte correct?
    *e >>= shiftExtras[len];

    return next;
}

/// Compares u8chr utf8 string against u8chr standard char * string.
/// \param u8str character string for the comparison.
/// \param string u8chr string for the comparison.
/// \param len Number of characters to compare.
/// \return true if the two strings are equal, else false.
[[maybe_unused]] static bool U8IsStringEqual(const u8chr *u8str, const char *string, size_t len)
{
    for (int64_t ii = 0; ii < len; ++ii)
    {
        char c1 = (char) u8str[ii];
        char c2 = string[ii];
        if (c1 != c2)
        {
            return false;
        }
    }
    return true;
}

/// \param u8str1 Compares u8chr utf8 string against u8chr standard char * string.
/// \param u8str1Len character string for the comparison.
/// \param u8str2 u8chr string for the comparison.
/// \param u8str2Len Number of characters to compare.
/// \return true if the two strings are equal, else false.
[[maybe_unused]] static bool U8IsU8Equal(const u8chr *u8str1, size_t u8str1Len, const u8chr *u8str2, size_t u8str2Len)
{
    if (u8str1Len != u8str2Len)
    {
        return false;
    }

    if (u8str1Len > 0)
    {
        if (u8str1[0] != u8str2[0])
        {
            return false;
        }
        for (int64_t ii = 0; ii < u8str1Len; ++ii)
        {
            if (u8str1[ii] != u8str2[ii])
            {
                return false;
            }
        }
    }
    return true;
}

#endif