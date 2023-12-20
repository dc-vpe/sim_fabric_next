//
// Created by krw10 on 8/1/2023.
//

#include <cstdio>
#include "../../Includes/Hashmap.h"
#include "../../Includes/ParseData.h"

bool SetKey()
{
    Token token;

    printf("Set key test.\n");
    Hashmap hashmap;

    auto *s = new U8String("Test");

    total_run++;
    hashmap.Set(s, &token);
    if ( hashmap.Exists(s) )
    {
        total_passed++;
        return true;
    }
    total_failed++;
    return false;
}

bool SetManyKeys()
{
    total_run++;
    printf("Set many bucketList test.\n");
    Hashmap hashmap;

    int64_t total = 10000;
    auto *keys = new U8String[total];

    for(int64_t ii=0; ii<total; ++ii)
    {
        char szTmp[128];
        sprintf(szTmp, "This is string number %d", ii);
        keys[ii].CopyFromCString(szTmp);
    }

    Token token;
    for(int64_t ii=0; ii<total; ++ii)
    {
        if ( !hashmap.Set(&keys[ii], &token) )
        {
            total_failed++;
            return false;
        }
    }
    if (!fatal)
    {
        for(int64_t ii=0; ii<total; ++ii)
        {
            if ( !hashmap.Exists(&keys[ii]))
            {
                total_failed++;
                return false;
            }
        }
    }

    total_passed++;

    return true;
}

bool ReplaceKeys(int64_t total)
{
    total_run++;

    Hashmap hashmap;

    Token token;
    token.type = TokenTypes::VAR;
    auto *keys = new U8String[total];

    for(int64_t ii=0; ii<total; ++ii)
    {
        char szTmp[128];
        sprintf(szTmp, "This is string number %d", ii);
        keys[ii].CopyFromCString(szTmp);
    }

    for(int64_t ii=0; ii<total; ++ii)
    {
        if ( !hashmap.Set(&keys[ii], &token) )
        {
            total_failed++;
            return false;
        }
    }

    Token token1;
    token1.type = TokenTypes::VARIABLE_DEF;
    for(int64_t ii=0; ii<total; ++ii)
    {
        if ( !hashmap.Set(&keys[ii], &token1) )
        {
            total_failed++;
            return false;
        }
    }

    for(int64_t ii=0; ii<total; ++ii)
    {
        Token *result = hashmap.Get(&keys[ii]);
        if ( result == nullptr )
        {
            total_failed++;
            return false;
        }
        if ( result->type != TokenTypes::VARIABLE_DEF )
        {
            total_failed++;
            return false;
        }
    }

    total_passed++;
    return true;
}

bool ReplaceKey()
{
    printf("Replace Key\n");
    return ReplaceKeys(1);
}

bool ReplaceManyKeys()
{
    printf("Replace many bucketList\n");
    return ReplaceKeys(10000);
}

bool RemoveKeys(int64_t total)
{
    total_run++;

    Hashmap hashmap;

    Token token;
    token.type = TokenTypes::VAR;
    auto *keys = new U8String[total];

    for(int64_t ii=0; ii<total; ++ii)
    {
        char szTmp[128];
        sprintf(szTmp, "This is string number %d", ii);
        keys[ii].CopyFromCString(szTmp);
    }

    for(int64_t ii=0; ii<total; ++ii)
    {
        if ( !hashmap.Set(&keys[ii], &token) )
        {
            total_failed++;
            return false;
        }
    }

    for(int64_t ii=0; ii<total; ++ii)
    {
        if ( !hashmap.Remove(&keys[ii]))
        {
            total_failed++;
            return false;
        }
    }

    for(int64_t ii=0; ii<total; ++ii)
    {
        if ( hashmap.Exists(&keys[ii]))
        {
            total_failed++;
            return false;
        }
    }

    total_passed++;
    return true;
}

bool RemoveKey()
{
    printf("Remove key\n");
    return RemoveKeys(1);
}

bool RemoveManyKeys()
{
    printf("Remove key\n");
    return RemoveKeys(10000);
}

bool RunAllHashmapTests()
{
    total_passed = 0;
    total_failed = 0;
    total_run = 0;

    SetKey();
    SetManyKeys();
    ReplaceKey();
    ReplaceManyKeys();
    RemoveKey();
    RemoveManyKeys();

    printf("Total Hashmap Tests Run: %d, Total Passed: %d, Total Failed: %d\n", total_run, total_passed, total_failed);

   return true;
}