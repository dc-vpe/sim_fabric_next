//
// Created by krw10 on 8/4/2023.
//

#include "../../Includes/ParseData.h"
#include <cstdlib>

void CreateIntListElements(int64_t elements)
{
    total_run++;
    List<int64_t> list = List<int64_t>();

    int64_t *array[elements];

    for (int64_t ii=0; ii<elements; ++ii)
    {
        array[ii] = &ii;
    }

    for(int64_t ii=0; ii<elements; ++ii)
    {
        if ( !list.push_back(*array[ii]))
        {
            total_failed++;
            return;
        }
    }

    for (int64_t ii=0; ii<elements; ++ii)
    {
        if( list[ii] != *array[ii] )
        {
            total_failed++;
            return;
        }
    }

    total_passed++;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
void CreateStringListElements(int64_t elements)
{
    total_run++;
    List<U8String *> list = List<U8String *>();

    U8String *array[elements];

    char sz[33];
    for (int64_t ii=0; ii<elements; ++ii)
    {
        size_t len = 1 + rand() / ((RAND_MAX + 1u) / 32);
        for(int64_t tt = 0; tt < len; ++tt)
        {
            sz[tt] = 'A' + (char)(1 + rand() / ((RAND_MAX + 1u) / 26));
        }
        sz[len] = '\0';
        array[ii] = new U8String(sz);
    }

    for(int64_t ii=0; ii<elements; ++ii)
    {
        if ( !list.push_back(array[ii]) )
        {
            total_failed++;
            return;
        }
    }

    for (int64_t ii=0; ii<elements; ++ii)
    {
        if ( !list[ii]->IsEqual(array[ii]))
        {
            total_failed++;
            return;
        }
    }

    total_passed++;
}
#pragma clang diagnostic pop

[[maybe_unused]] void TestIntOne()
{
    CreateIntListElements(1);
}

[[maybe_unused]] void TestIntLots()
{
    CreateIntListElements(10000);
}

[[maybe_unused]] void TestStringOne()
{
    CreateStringListElements(1);
}

[[maybe_unused]] void TestStringLots()
{
    CreateStringListElements(5000);
}

[[maybe_unused]] bool RunAllListTests()
{
    TestIntOne();
    TestIntLots();
    TestStringOne();
    TestStringLots();

    printf("Total List Tests Run: %d, Total Passed: %d, Total Failed: %d\n", total_run, total_passed, total_failed);

    return false;
}