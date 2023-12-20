var a = "There are 3 foxes and 20 cats in the garden.";

var sub;
var start;
var ex = "%D%D";
var len;

len = string.len(ex);
start = string.find(a, ex, 0);
sub = string.sub(a, start, len);

print(sub);

stop