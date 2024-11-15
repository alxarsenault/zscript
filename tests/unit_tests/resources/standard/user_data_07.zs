const math = import("math");
var a = math.normal_dist(1.0, 2.0);
a.mean = 3.0;
return a.mean + a.get_mean();