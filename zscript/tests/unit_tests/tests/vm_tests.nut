local utest = @import("utest");

print(__FILE__);

// Lamda function.
TEST_CASE("Lamda function", function() {
  local f = $(a, b) {
    return a + b;
  };

  REQUIRE(f(1, 2) == 3);
});

TEST_CASE("Lamda function2", function() {
  local k = 50;
  local f = $(a, b) {
    return k + a + b;
  };

  REQUIRE(f(1, 2) == 53);
});


TEST_CASE("Lamda function3", function() {
  local k = {
    a = 51
  };

  //
  REQUIRE($(t) { return t.a; }(k) == 51);

  // Table access.
  REQUIRE(k[$() { return "a" }()] == 51);
});

TEST_CASE("Basic", function() {
  local a = 10;
  REQUIRE(a == 10);
});

TEST_CASE("VM", function() {
  local a = @("/Users");
  // local b = "/Users";
  // local c = @(b);

  // local π = "banana";
  // print(a.root());
  // REQUIRE(a.root() == "/");
  // REQUIRE(type(a) == "path");
  // REQUIRE(type(b) == "string");
  // REQUIRE(type(c) == "path");
});

TEST_CASE("VM2", function() {

});

TEST_CASE("VM3", function() {

  local my_table = {
    // data = {},
    _data = {},

    function doIt(p) {
      return p;
    }
  };

  local meta = {
    function _get(key) {
      // ::print("_GET", key);

      if (key in _data) {
        return _data[key];
      }

      throw null;
    }

    function _newslot(key, value) {
      // ::print("_newslot", key, value);
      // _data[key] <- value;

      rawset(key, value);
    }

    function _set(key, value) {
      // print("_SET", key, value);

      if (key in _data) {
        _data[key] = value;
        return;
      }

      _data[key] <- value;
    }
  };

  local meta2 = {
    function _get(key) {
      // ::print("_GET", key);

      if (key in _data) {
        return _data[key];
      }

      throw null;
    }

    function _newslot(key, value) {
      // ::print("_newslot", key, value);
      _data[key] <- value;

      // this.rawset(key, value);
    }

    function _set(key, value) {
      // print("_SET", key, value);

      if (key in _data) {
        _data[key] = value;
        return;
      }

      _data[key] <- value;
    }
  };

  my_table.setdelegate(meta);


  REQUIRE(my_table.doIt(10) == 10);

  my_table.a = 10;
  REQUIRE(my_table.a == 10);
  REQUIRE(my_table._data.a == 10);

  my_table.b <- 12;

  REQUIRE(my_table.b == 12);
  REQUIRE(!my_table._data.contains("b"));


  my_table.setdelegate(meta2);


  my_table.c <- 15;

  REQUIRE(my_table.c == 15);
  REQUIRE(my_table._data.contains("c"));

  //
  REQUIRE(!my_table.get("bacon"));
  REQUIRE(rawtype(my_table.get("bacon")) == "null");

  //
  REQUIRE(my_table.get("a") == 10);

  REQUIRE(my_table.contains("a"));
});

utest.run();
