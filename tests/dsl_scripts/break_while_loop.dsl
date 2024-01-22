//This script tests the break keyword in a while loop.
var a = 10;

while( a > 1 )
{
    if ( a <= 5 )
    {
        break;
    }
    a = a - 2;
}
print("a should equal 4, a equals ", a);