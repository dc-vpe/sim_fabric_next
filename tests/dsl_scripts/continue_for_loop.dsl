//This script tests continue in a for loop.
var a = 0;
var count = 0;

for(a=0; a<10; a++)
{
    if ( a > 5 )
    {
        continue;
    }
    ++count;
}

print("count should equal 6, count == ", count);