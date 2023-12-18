//
// Created by krw10 on 8/1/2023.
//


#include <cstdio>
#include "../Includes/Hashmap.h"
#include <cstdlib>

extern int64_t total_passed;
extern int64_t total_failed;
extern int64_t total_run;

void CreateMany()
{
    U8String u8Strings[2000];
    char sz[2000][33];

    total_run++;
    for(int64_t ii=0; ii<2000; ++ii)
    {
        int64_t len = 1 + (int64_t)rand() / ((RAND_MAX + 1u) / 32); //NOLINT (limited use test no need to include C11 rand lib)
        memset(sz[ii], 0, 32);
        for(int64_t tt = 0; tt < len; ++tt)
        {
            sz[ii][tt] = 'A' + (char)(1 + rand() / ((RAND_MAX + 1u) / 26)); //NOLINT (limited use test no need to include C11 rand lib)
        }
        u8Strings[ii].CopyFromCString(sz[ii]);
    }
    for(int64_t ii=0; ii<2000; ++ii)
    {
        if ( !u8Strings[ii].IsEqual(sz[ii]) )
        {
            printf("%s\n", u8Strings[ii].cStr());
            printf("%s\n", sz[ii]);
            total_failed++;
            return;
        }
    }
    total_passed++;
}

[[maybe_unused]] void RunAllStringTests()
{
    total_passed = 0;
    total_failed = 0;
    total_run = 0;

    CreateMany();

    printf("Total String Tests Run: %d, Total Passed: %d, Total Failed: %d\n", total_run, total_passed, total_failed);
}