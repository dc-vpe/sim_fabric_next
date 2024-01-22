//This script tests the break keyword with a for statement.
var a;
for(a = 10; a>1; --a)
{
    if ( a <= 5 )
    {
        break;
    }
}

print("a should equal 5, a equals ", a);

stop