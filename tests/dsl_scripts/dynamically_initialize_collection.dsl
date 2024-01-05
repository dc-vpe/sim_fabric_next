//This script tests the ability of a collection to be dynamically assigned during creation.
var v = { "hello":fun(), "goodby":111 };
print(v);


var fun() { return 3+3; };


stop