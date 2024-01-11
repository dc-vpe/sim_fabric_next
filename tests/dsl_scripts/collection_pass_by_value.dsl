//Confirms that collections are passed by value
var a = { 1, 2, 3 };
var b = a;

b[1] = 10;

print("a = ", a "\n");
print("b = ", b "\n");

b["a.1"] = 11;
print("a = ", a "\n");
