//Test collection with other collections as elements.
var a = { 1, 2, 3 };
var b = { 4, 5, 6 };

var c = { a, b };

print(c);


stop


0000    JMP     0002
0001    JBF     print
0002    DEF     TMScriptScope.test.a    ;var TMScriptScope.test.a
0003    PVA     &TMScriptScope.test.a
0004    DEF     TMScriptScope.test.b    ;var TMScriptScope.test.b
0005    PVA     &TMScriptScope.test.b
0006    DEF     TMScriptScope.test.c    ;var TMScriptScope.test.c
0007    PVA     &TMScriptScope.test.c

0008    PSV     TMScriptScope.test.a
0009    DCS     &TMScriptScope.test.c

0010    PSV     TMScriptScope.test.b
0011    DCS     &TMScriptScope.test.c

0012    PSV     TMScriptScope.test.c
0013    PSI     1
0014    JBF     print
0015    END




//var k = {,,,,"4":"four", "5":"five",,,, "D": "d" };
//print(k);

{        ,        ,        ,     , "4":"four", "5":"five",       ,        ,        , "D": "d" };
{ "k.0":0, "k.1":0, "k.2":0,       "4":four,   "5":five,  "k.3":0, "k.4":0, "k.5":0, "D":d }


var a = { };
var b = { 1.1, 2, 3 };
var c = { "one", "two", "three" };
var d = { 1:"one", 2:"two" };
var e = { 2:"two", 1:"one" };
var g = { "a":"a", { "1":"1", "2", "2" } };
var h = { "one":fun(), "two":9 + fun() + 10 };
var i = { "1":b, "2":c", "3":d" };
var j = { "a", "b", "c" };
var k = {,,,,"4":"four", "5":"five",,,, "D": "d" };
var l = { "4":"four" };

var fun() { return 1 + 2; }

print(a, b, c, d, e, f, g, h, i, j, k);

//////////////////
var a = { };
var b = { 1.1, 2, 3 };
var c = { "one", "two", "three" };
var d = { 1:"one", 2:"two" };
var e = { 2:"two", 1:"one" };

print("a = ", a, "\n", "b = ", b,  "\n", "c = ", c,  "\n", "d = ", d, "\n", "e = ", e, "\n");





BUGBUG
//change collection element
var c = { "one", "two", "three" };

c[1] = "xxx";

print(c[1]);