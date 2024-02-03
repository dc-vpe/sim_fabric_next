//This tests the on tick event which is called every .1 second.
var a = 0;

while( a < 15 * 10 )
{
}

var OnTick()
{
    ++a;
    print("Count = ", a, "\n");
    return 0;
}

stop