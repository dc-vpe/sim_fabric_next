//
// Created by krw10 on 6/21/2023.
//
#include "../Includes/U8String.h"
#include <malloc.h>
#include <cstdio>

bool U8String::IsEqual(U8String *u8String)
{
    if (u8String == nullptr)
    {
        return false;
    }

    if (Count() != u8String->Count() )
    {
        return false;
    }

    int64_t ii;
    for(ii=0; ii < Count(); ++ii)
    {
        if (get(ii) != u8String->get(ii))
        {
            return false;
        }
    }

    return true;
}

bool U8String::IsEqual(const char *string)
{
    if (string == nullptr)
    {
        return false;
    }

    if (Count() != strlen(string) )
    {
        return false;
    }

    for(int64_t ii=0; ii < Count(); ++ii)
    {
        if (get(ii) > 127 ) //UTF8 below 128 is just a normal char
        {
            return false;
        }
        if (get(ii) != string[ii])
        {
            return false;
        }
    }

    return true;
}

bool U8String::IsGreater(U8String *u8String)
{
    if (u8String == nullptr)
    {
        return false;
    }

    size_t l1 = Count();
    size_t l2 = u8String->Count();
    if ( l1 == l2 )
    {
        for(int64_t ii=0; ii<l1; ++ii)
        {
            if (get(ii) > u8String->get(ii))
            {
                return true;
            }
        }
        return false;
    }
    else if ( l1 > l2 )
    {
        return true;
    }

    return false;
}

bool U8String::IsLess(U8String *u8String)
{
    if (u8String == nullptr)
    {
        return false;
    }

    size_t l1 = Count();
    size_t l2 = u8String->Count();
    if ( l1 == l2 )
    {
        for(int64_t ii=0; ii<l1; ++ii)
        {
            if (get(ii) < u8String->get(ii))
            {
                return true;
            }
        }
        return false;
    }
    else if ( l1 < l2 )
    {
        return true;
    }

    return false;
}

bool U8String::push_back(u8chr ch)
{
    if ( !buffer->push_back(ch) )
    {
        return false;
    }
    if ( !buffer->Set(Count(), U8_NULL_CHR) )
    {
        return false;
    }

    if ( !ascii->push_back((char)ch) )
    {
        return false;
    }
    if ( !ascii->Set(Count(), '\0') )
    {
        return false;
    }

    return true;
}

bool U8String::push_back(U8String *u8String)
{
    for(int64_t ii=0; ii< u8String->Count(); ++ii)
    {
        if ( !push_back(u8String->get(ii)) )
        {
            return false;
        }
    }

    return true;
}

U8String::U8String(const char *cString)
{
    buffer = new List<u8chr>();
    ascii = new List<char>();
    size_t length = strlen(cString);
    for(int64_t ii=0; ii<length; ++ii)
    {
        if ( !push_back(cString[ii]) )
        {
            //error handled in push_back.
            return;
        }
    }
}

U8String::U8String(U8String *u8String)
{
    buffer = new List<u8chr>();
    ascii = new List<char>();
    auto count = (int64_t) u8String->Count();
    for(int64_t ii=0; ii<count; ++ii)
    {
        push_back(u8String->get(ii));
    }
}

bool U8String::CopyFromCString(const char *cString)
{
    Clear();

    //convert cString (char *null terminated) to a UTF8 string null terminated.
    Byte *pIn = (Byte *)cString;
    u8chr ch;
    int64_t e;
    do
    {
        pIn = utf8_decode(pIn, &ch, &e);
        if ( e )
        {
            PrintIssue(1007,
                       true,
                       false,
                       "Warning invalid UTF8 character %04x, ignoring",
                       ch);
        }
        else
        {
            if ( ch != U8_NULL_CHR)
            {
                if ( !push_back(ch) )
                {
                    return false;
                }
            }
        }
    } while( ch != U8_NULL_CHR );

    return true;
}

bool U8String::CopyFromInt(int64_t i)
{
    char szTmp[256] = { "0" };

    lltoa(i, szTmp, 10);

    return CopyFromCString(szTmp);
}

bool U8String::CopyFromDouble(double d)
{
    char szTmp[256] = { "0" };

    gcvt(d, 32, szTmp);

    return CopyFromCString(szTmp);
}

u8chr U8String::get(size_t index)
{
    if (index < Count())
    {
        return buffer->get((int64_t) index);
    }

    return U8_NULL_CHR;
}

bool U8String::set(size_t index, u8chr ch)
{
    if ( !buffer->Set(index, ch) )
    {
        return false;
    }
    if ( !buffer->Set(index+1, U8_NULL_CHR) )
    {
        return false;
    }
    if ( !ascii->Set(index, (char)ch) )
    {
        return false;
    }

    if ( !ascii->Set(index+1, '\0') )
    {
        return false;
    }
    return true;
}

int64_t U8String::IndexOf(u8chr ch)
{
   for(int64_t ii=0; ii < Count(); ++ii)
   {
       if (buffer->get(ii) == ch )
       {
           return ii;
       }
   }

   return -1;
}

bool U8String::Append(U8String *u8String)
{
    for(int ii=0; ii< u8String->Count(); ++ii)
    {
        if ( !push_back(u8String->get(ii)) )
        {
            return false;
        }
    }
    return true;
}

bool U8String::Append(const char *cStr)
{
    char *ptr = (char *)cStr;
    while( *ptr != '\0' )
    {
        if (!push_back(*ptr) )
        {
            return false;
        }
        ++ptr;
    }

    return true;
}

bool U8String::Append(int64_t i)
{
    U8String tmp;
    tmp.CopyFromInt(i);
    return push_back(&tmp);
}

bool U8String::printf(char *format, ...)
{
    va_list length_args;
    va_start(length_args, format);
    va_list result_args;
    va_copy(result_args, length_args);
    const auto length = vsnprintf(nullptr, 0U, format, length_args);
    ascii->Resize(length+1);
    vsprintf((char *)ascii->Array(), format, result_args);
    va_end(result_args);
    va_end(length_args);

    Byte *pIn = (Byte *)ascii->Array();

    for(int ii=0; ii<length; ++ii)
    {
        u8chr ch;
        int64_t e;
        pIn = utf8_decode(pIn, &ch, &e);
        if ( e )
        {
            PrintIssue(1007,
                       true,
                       false,
                       "Warning invalid UTF8 character %04x, ignoring",
                       ch);
        }
        buffer->Set(ii, ch);
    }
    buffer->SetCount(length);
    ascii->SetCount(length);

    return true;
}

