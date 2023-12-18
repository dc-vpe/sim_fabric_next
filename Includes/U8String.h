//
// Created by krw10 on 6/21/2023.
//

#ifndef DSL_UTF8STRING_H
#define DSL_UTF8STRING_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include "dsl_types.h"
#include "Utf8.h"
#include "LocationInfo.h"
#include "List.h"

/// \desc Implements a string Type that works with UTF8 Characters.
/// \remark if the DEBUG macro is specified then all U8Strings will copy
///         a cText version to an internal character buffer for ease of
///         reading and display in the debugger.
class U8String
{
public:
    /// \desc Gets the length of the string in characters.
    /// \return
    size_t Count()  { return buffer->Count(); }

    /// \desc Creates a blank UTF8 string.
    U8String()
    {
        buffer = new List<u8chr>();
        ascii = new List<char>();
    }

    /// \desc Creates a new UTF8 string and initializes it with the provided cString.
    explicit U8String(const char *cString);

    /// \desc Creates a new U8String and initializes it with the cText in the passed in U8String.
    /// \param u8String String to use to initialize this U8String.
    explicit U8String(U8String *u8String);

    /// \desc Frees the resources used by the U8String.
    ~U8String()
    {
        delete buffer;
        delete ascii;
    }

    /// \desc Appends a single character to the end of the null terminated string in the buffer.
    bool push_back(uint32_t ch);

    /// \desc Appends the u8String to the end of this string.
    /// \param u8String String to append to this string.
    /// \return True if successful, or false if out of memory.
    bool push_back(U8String *u8String);

    /// \desc Checks if the UTF8String is equal to the passed in UTF8 string.
    /// \param u8String String to check against this string.
    /// \return True if this string is less than the passed in string.
    bool IsEqual(U8String *u8String);

    /// \desc Checks if the character string is equal to this UTF8 string.
    bool IsEqual(const char *string);

    /// \desc Checks if this UTF8 string is greater than the passed in UTF8 string.
    bool IsGreater(U8String *string);

    /// \desc Checks if this UTF8 string is less than the passed in UTF8 string.
    bool IsLess(U8String *string);

    /// \desc Gets an integer representation of the numbers in the buffer.
    int64_t GetInt()
    {
        const char *str = cStr();
        char *tmp;
        return strtol(str, &tmp, 10);
    }

    /// \desc Gets a double representation of the numbers in the buffer.
    double GetDouble()
    {
        const char *str = cStr();
        char *tmp;
        return strtod(str, &tmp);
    }

    /// \desc Sets the length of the UTF8String to 0.
    /// \param this Pointer to the string structure.
    inline void Clear()
    {
        buffer->Clear();
        ascii->Clear();
    }

    void CopyFrom(U8String *u8String)
    {
        if ( this == u8String )
        {
            return;
        }
        buffer->Clear();
        ascii->Clear();

        for(int64_t ii=0; ii< u8String->Count(); ++ii)
        {
            push_back(u8String->get(ii));
        }
    }

    U8String &operator=(U8String other)
    {
        if ( this == &other )
        {
            return *this;
        }
        Clear();
        size_t count = other.Count();
        for(int64_t ii=0; ii<count; ++ii)
        {
            set(ii, other.get(ii));
        }
        return *this;
    }

    /// \desc Copies the characters in the cString (char string null terminated) to this U8String converting
    ///       it to UTF8 as needed.
    /// \return True if successful, or false if out of memory.
    bool CopyFromCString(const char *cString);

    /// \desc Converts the integer value to a string and copies the characters to this u8String.
    /// \param i Integer value to convert to a string and store in this string.
    bool CopyFromInt(int64_t i);

    /// \desc Converts the double value to a string and copies the characters to this u8String.
    /// \param d Integer value to convert to a string and store in this string.
    bool CopyFromDouble(double d);

    /// \desc Gets a character string representation of what is stored in the
    ///       the U8String. Max length is MAX_STRING_LEN_SIZE
    const char *cStr() { return ascii->Array(); }

    /// \desc Gets a character at index in the UTF8 string.
    /// \param index Zero based index in the string.
    /// \return The character or U8_NULL_CHR if the index is outside the bounds of the buffer.
    u8chr get(size_t index);

    /// \desc Sets a single u8chr to the string at index.
    /// \param index Value of the location to set the character to.
    /// \param ch character to set.
    /// \return True if the operation succeeds, else false if the index is out of range.
    bool set(size_t index, u8chr ch);

    /// \desc Gets the index of the character in the string.
    /// \param ch character to check.
    /// \return The index of the character in the string if it exists or -1 if the character is
    ///         not contained in the string.
    int64_t IndexOf(u8chr ch);

    /// \desc Appends the U8String to the end of this u8String.
    bool Append(U8String *u8String);

    /// \desc Appends the c string to the end of this u8String.
    bool Append(const char *cStr);

    /// \desc Converts the value to a string and appends it to this string.
    bool Append(int64_t i);

    /// \desc Allocates and copies the strings contents to a u8chr array.
    /// \param data Pointer to the buffer to receive the UTF8 characters.
    /// \return True if successful else false.
    /// \remark The caller is responsible for freeing the returned array by calling free.
    bool GetBuffer(u8chr *data)
    {
        for(int64_t ii=0; ii<buffer->Count(); ++ii)
        {
            *data++ = buffer->get(ii);
        }

        return true;
    }

    /// \desc Provides printf style string creation for the U8String.
    /// \return True if successful else false.
    bool printf(char *format, ...);

private:
    List<u8chr> *buffer;
    List<char> *ascii;
};

#endif //DSL_UTF8STRING_H
