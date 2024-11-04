var output = {};

var a = [1, 2, 3];

output.size_01 = a.size();
output.length_01 = a.length();
output.capacity_01 = a.capacity();
output.is_empty_01 = a.is_empty();

a.resize(10);

output.size_02 = a.size();
output.length_02 = a.length();
output.capacity_02 = a.capacity();
output.is_empty_02 = a.is_empty();

a.resize(3);

output.get0_00 = a.get(0);
output.get1_01 = a.get(1);
output.get2_02 = a.get(2);

a.erase(1);

output.size_03 = a.size();

a.push(10);
a.push(11);
a.push(12);

output.size_04 = a.size();
output.get0_02 = a.get(0);
output.get1_02 = a.get(1);
output.get2_02 = a.get(2);
output.get3_02 = a.get(3);
output.get4_02 = a.get(4);


output.is_number_array = a.is_number_array();
return output;