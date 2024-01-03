//This script tests variable assignment and functions
//calling other functions internally using parameters

var a;

a = fun();

print(a);

var fun() 
{ 
	return bar(1, 3);
}

var bar(a, b)
{
    return a + b;
}

stop
