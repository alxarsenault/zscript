local utest = @import("utest");
local wp = @import("wp");

TEST_CASE("vector-assign", function() {
  local v1 = wp.vector(wp.f32);

  v1.assign([1, 2, 3, 4, 5]);
  // foreach(v in v1) {
  //   print(v);
  // }
  REQUIRE(v1.size() == 5);

  // print(v1.to_array());
  REQUIRE(v1[0] == 1.0);

  v1.assign(55.2);
  REQUIRE(v1[0] == 55.2);
});

TEST_CASE("vector-add", function() {
  local v1 = wp.vector(wp.f32, 3);
  local v2 = wp.vector(wp.f32, 3);

  v1[0] = 1;
  v1[1] = 2;
  v1[2] = 3;

  v2[0] = 4;
  v2[1] = 5;
  v2[2] = 6;


  local v3 = v1 + v2;
  // print(__LINE__, v1._handle, v2._handle, v3._handle, v3.to_array());

  v1 += v2;

  // print(__LINE__, v1._handle, v2._handle, v3._handle, v1.to_array());

  v1.sum(v2);
  // print(__LINE__, v1._handle, v2._handle, v3._handle, v1.to_array());

  local v4 = v1 * 2;
  // print(v4.to_array());

  v4 *= v1;
  // print(v4.to_array());



  local v5 = wp.vector(wp.i32, 3);
  v5[0] = -10;
  v5[1] = -20;
  v5[2] = -30;

  v2 += v5;
  // print(v2.to_array());

  local a0 = [1, 2, 3];
  local a1 = wp.vector(wp.f32, 3);
  a1[0] = 10;
  a1[1] = 20;
  a1[2] = 30;

  local a2 = a1 + a0;
  // print(a2.to_array());

});


TEST_CASE("wp.vector", function() {

  local v = wp.vector(wp.f32, 10, 1.1);


  REQUIRE(typeof(v) == "vector<float>");

  REQUIRE(v.size() == 10);
  REQUIRE(v[0] == 1.1);

  v.push(42.5);
  REQUIRE(v.size() == 11);
  REQUIRE(v[10] == 42.5);

  v[0] = 12.12;
  REQUIRE(v[0] == 12.12);
  REQUIRE(v[1] == 1.1);

  v.resize(15);
  REQUIRE(v.size() == 15);

  // v.bk = 23;
  v.clear();
  REQUIRE(v.size() == 0);

  v.push(1);
  v.push(2);
  v.insert(0, 10.2);

  REQUIRE(v[0] == 10.2);
  v.insert(2, 10.3);

  REQUIRE(v[0] == 10.2);
  REQUIRE(v[1] == 1);
  REQUIRE(v[2] == 10.3);
  REQUIRE(v[3] == 2);

  v.clear();

  for (int i = 0; i < 5; i++) {
    v.push(i);
  }

  REQUIRE(v.size() == 5);
  v.erase(2);

  REQUIRE(v.size() == 4);
  REQUIRE(v[0] == 0);
  REQUIRE(v[1] == 1);
  REQUIRE(v[2] == 3);
  REQUIRE(v[3] == 4);

  v.erase_n(0, 2);
  REQUIRE(v.size() == 2);
  REQUIRE(v[0] == 3);
  REQUIRE(v[1] == 4);



  // Clone.
  local v2 = wp.vector(wp.f32, 5, 3.1);

  local v3 = clone(v2);

  // print(v2, v3);

  REQUIRE(v2.size() == v3.size());
  REQUIRE(v2 != v3);

  REQUIRE(v2.data_pointer() != v3.data_pointer());

  v2[0] = 128.2;
  v3[0] = 129.2;
  REQUIRE(v2[0] == 128.2);
  REQUIRE(v3[0] == 129.2);
});


TEST_CASE("Dvector", function() {

  local v = wp.vector(wp.f64, 10, 1.1);
  REQUIRE(typeof(v) == "vector<double>");
  REQUIRE(v.size() == 10);
  REQUIRE(v[0] == 1.1);

  v.push(42.5);
  REQUIRE(v.size() == 11);
  REQUIRE(v[10] == 42.5);

  v[0] = 12.12;
  REQUIRE(v[0] == 12.12);
  REQUIRE(v[1] == 1.1);

  v.resize(15);
  REQUIRE(v.size() == 15);
});


TEST_CASE("Ivector", function() {

  local v = wp.vector(wp.i32, 10, 1);
  REQUIRE(typeof(v) == "vector<int32>");
  REQUIRE(v.size() == 10);
  REQUIRE(v[0] == 1);

  v.push(42);
  REQUIRE(v.size() == 11);
  REQUIRE(v[10] == 42);

  v[0] = 12;
  REQUIRE(v[0] == 12);
  REQUIRE(v[1] == 1);

  v.resize(15);
  REQUIRE(v.size() == 15);

  // v.bk = 23;
});

utest.run();
