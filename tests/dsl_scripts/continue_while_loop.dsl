//This script tests continue in a while loop
var a = 0;
var count = 0;

while( a < 10 )
{
    ++a;
    if ( a > 5 )
    {
        continue;
    }
    ++count;
}
print("count should equal 5, count == ", count);