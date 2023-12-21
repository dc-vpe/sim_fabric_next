var v = { "a":100, "b":200 };
var s;

s = string.fromCollection(v);

print(s);


stop

//call to inner function with more parms.
var a = "    XXYYZZ    ";
print(string.trimStart(a, "%S"));

//JSON strings are not preserving the ""
//{ "a": 1.5, "b":2, "c": "CCC" }

//JSON with semi colon at end causes signal fault.
{ "a": 1.5, "b":2, "c": "CCC" };


//Collection processing needs to be able to write out arrays with []
