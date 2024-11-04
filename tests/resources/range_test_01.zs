

var range = import("range");
var pppp = fs::path("/");
var lll = pppp.list();
var output = {};
var r = range(10, 25);
var klkr = range(10, 678);
output.r = r;
output.r_length = r.length();
output.r_middle = r.middle();

var r2 = r.with_start(20);
output.r2 = r2;

var r3 = range(-10, 10);
var r4 = r3.with_length(20);
var r5 = range(-25.5, 0.0).with_length(51.0);
var r6 = range(r5);
r6.start = 0.0;

var r7 = range(r6).move_to(120.12);

var r8 = range(10, 5);
var r9 = range([90, 92]);
var r10 = range(range({start = 0, end = 21}))
output.contains_01 = r7.contains(124);
output.contains_02 = r7.contains(212);
output.contains_03 = r7.contains(r4);
output.r3 = r3;
output.r4 = r4;
output.r5 = r5;
output.r6 = r6;
output.r7 = r7;
output.r8 = r8;
output.r10 = r10;


output.cvalue_01 = r8.clip_value(12)
output.cvalue_02 = r8.clip_value(7)
output.cvalue_03 = r8.clip_value(3)

return output;