local utest = @import("utest");
local veclib = @import("wp-veclib");

TEST_CASE("fvector", function() {
  local vectype = veclib.float_vector;
  local v = vectype.vector_create(10, 2.3);

  REQUIRE(vectype.vector_size(v) == 10);
  REQUIRE(vectype.vector_get(v, 0) == 2.3);
  REQUIRE(vectype.vector_get(v, 1) == 2.3);

  // Set.
  vectype.vector_set(v, 1, 5.5);
  REQUIRE(vectype.vector_get(v, 1) == 5.5);

  vectype.vector_push(v, 4.8);
  REQUIRE(vectype.vector_size(v) == 11);
  REQUIRE(vectype.vector_get(v, 10) == 4.8);

  vectype.vector_resize(v, 15);
  REQUIRE(vectype.vector_size(v) == 15);

  vectype.vector_resize(v, 16, 3.2);
  REQUIRE(vectype.vector_size(v) == 16);

  vectype.vector_pop(v);
  REQUIRE(vectype.vector_size(v) == 15);

  vectype.vector_erase(v, 0);
  REQUIRE(vectype.vector_size(v) == 14);
  REQUIRE(vectype.vector_get(v, 0) == 5.5);


  vectype.vector_insert(v, 0, 1.1);
  REQUIRE(vectype.vector_size(v) == 15);
  REQUIRE(vectype.vector_get(v, 0) == 1.1);
  REQUIRE(vectype.vector_get(v, 1) == 5.5);

  vectype.vector_insert_n(v, 0, 2, 1.5);
  REQUIRE(vectype.vector_size(v) == 17);
  REQUIRE(vectype.vector_get(v, 0) == 1.5);
  REQUIRE(vectype.vector_get(v, 1) == 1.5);
  REQUIRE(vectype.vector_get(v, 2) == 1.1);
  REQUIRE(vectype.vector_get(v, 3) == 5.5);

  // print(vectype.vector_to_array(v));
});


TEST_CASE("dvector", function() {
  local vectype = veclib.double_vector;
  local v = vectype.vector_create(10, 2.3);

  REQUIRE(vectype.vector_size(v) == 10);
  REQUIRE(vectype.vector_get(v, 0) == 2.3);
  REQUIRE(vectype.vector_get(v, 1) == 2.3);

  // Set.
  vectype.vector_set(v, 1, 5.5);
  REQUIRE(vectype.vector_get(v, 1) == 5.5);

  vectype.vector_push(v, 4.8);
  REQUIRE(vectype.vector_size(v) == 11);
  REQUIRE(vectype.vector_get(v, 10) == 4.8);

  vectype.vector_resize(v, 15);
  REQUIRE(vectype.vector_size(v) == 15);

  vectype.vector_resize(v, 16, 3.2);
  REQUIRE(vectype.vector_size(v) == 16);

  vectype.vector_pop(v);
  REQUIRE(vectype.vector_size(v) == 15);

  vectype.vector_erase(v, 0);
  REQUIRE(vectype.vector_size(v) == 14);
  REQUIRE(vectype.vector_get(v, 0) == 5.5);


  vectype.vector_insert(v, 0, 1.1);
  REQUIRE(vectype.vector_size(v) == 15);
  REQUIRE(vectype.vector_get(v, 0) == 1.1);
  REQUIRE(vectype.vector_get(v, 1) == 5.5);

  vectype.vector_insert_n(v, 0, 2, 1.5);
  REQUIRE(vectype.vector_size(v) == 17);
  REQUIRE(vectype.vector_get(v, 0) == 1.5);
  REQUIRE(vectype.vector_get(v, 1) == 1.5);
  REQUIRE(vectype.vector_get(v, 2) == 1.1);
  REQUIRE(vectype.vector_get(v, 3) == 5.5);

  // print(veclib.dvector_to_array(v));
});


TEST_CASE("ivector", function() {
  local vectype = veclib.int32_vector;
  local v = vectype.vector_create(10, 2);


  REQUIRE(vectype.vector_size(v) == 10);
  REQUIRE(vectype.vector_get(v, 0) == 2);
  REQUIRE(vectype.vector_get(v, 1) == 2);

  // Set.
  vectype.vector_set(v, 1, 5);
  REQUIRE(vectype.vector_get(v, 1) == 5);

  vectype.vector_push(v, 4);
  REQUIRE(vectype.vector_size(v) == 11);
  REQUIRE(vectype.vector_get(v, 10) == 4);

  vectype.vector_resize(v, 15);
  REQUIRE(vectype.vector_size(v) == 15);

  vectype.vector_resize(v, 16, 3);
  REQUIRE(vectype.vector_size(v) == 16);

  vectype.vector_pop(v);
  REQUIRE(vectype.vector_size(v) == 15);

  vectype.vector_erase(v, 0);
  REQUIRE(vectype.vector_size(v) == 14);
  REQUIRE(vectype.vector_get(v, 0) == 5);


  vectype.vector_insert(v, 0, 1);
  REQUIRE(vectype.vector_size(v) == 15);
  REQUIRE(vectype.vector_get(v, 0) == 1);
  REQUIRE(vectype.vector_get(v, 1) == 5);

  vectype.vector_insert_n(v, 0, 2, 1);
  REQUIRE(vectype.vector_size(v) == 17);
  REQUIRE(vectype.vector_get(v, 0) == 1);
  REQUIRE(vectype.vector_get(v, 1) == 1);
  REQUIRE(vectype.vector_get(v, 2) == 1);
  REQUIRE(vectype.vector_get(v, 3) == 5);

  // print(veclib.ivector_to_array(v));
});



utest.run();
