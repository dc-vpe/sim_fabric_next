//Complex expression auto casing with recursive function call
var v = "hello " + fun1(((8*5)/5 - 10 - 8) + (3-1) - 2);

print("v should equal hello -50, v equals ", v);


var fun1(x) { return f1(x); }
var f1(y) { return y * 5; }
stop
