//This script tests simple error handling. When the divide by zero
//happens the error handler is called and the program continues
//because the on error handler returns 0.
var a;

a /= 0;

print("here");

var OnError()
{
    print("error\n");
    return 0;
}

var fn() { }

stop