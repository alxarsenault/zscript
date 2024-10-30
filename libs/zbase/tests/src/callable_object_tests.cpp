#include <ztests/ztests.h>
#include <zbase/memory/callable_object.h>
 
TEST_CASE("callable_object") {
  {
    int count = 0;
    auto cobj = zb::callable_object<void(int)>::create_unique([&](int value) {
      REQUIRE(value == 32);
      count++;
    });

    cobj->call(32);
    REQUIRE(count == 1);

    (*cobj)(32);
    REQUIRE(count == 2);
  }

  {
    using callable_type = zb::callable_object<int(int)>;
    int count = 0;
    std::shared_ptr<callable_type> cobj = callable_type::create_shared([&](int value) {
      REQUIRE(value == 32);
      count++;
      return value;
    });

    REQUIRE(cobj->call(32) == 32);
    REQUIRE(count == 1);

    REQUIRE((*cobj)(32) == 32);
    REQUIRE(count == 2);
  }

  {

    int count = 0;
    std::shared_ptr<zb::task> cobj = zb::task::create_shared([&]() { count++; });

    cobj->call();
    REQUIRE(count == 1);

    (*cobj)();
    REQUIRE(count == 2);
  }

  {

    int count = 0;
    zb::task::shared_ptr cobj = zb::task::create_shared([&]() { count++; });

    cobj->call();
    REQUIRE(count == 1);

    cobj->call();
    REQUIRE(count == 2);
  }

  {

    int count = 0;
    auto task = zb::task::create_shared([&]() { count++; });

    task->call();
    REQUIRE(count == 1);
  }

  {

    int count = 0;
    zb::task::create_unique([&]() { count++; })->call();

    REQUIRE(count == 1);
  }
}

static inline std::string my_function(const std::string& a) { return a; }

TEST_CASE("KLKLLK") {
  using callable_object = zb::callable_object<std::string(const std::string&)>;

  using stack_callable_object = callable_object::stack_callable_object<256>;

  //  REQUIRE(std::is_base_of_v<zb::callable_object<std::string(const std::string&)>,
  //  zb::callable_object<std::string(const std::string&)>>);
  stack_callable_object obj;

  obj.set(&my_function);
  {
    std::string s1 = obj("A");
    REQUIRE(s1 == "A");
  }

  obj.reset();

  {
    std::string s1 = obj.call("A");
    REQUIRE(s1 == "");
  }

  {
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "bingo");
  }

  obj.set([](const std::string& s) { return s; });

  REQUIRE(obj);

  {
    std::string s1 = obj("A");
    REQUIRE(s1 == "A");
  }

  {
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "A");
  }
}
TEST_CASE("sssss") {

  using callable_object = zb::callable_object<std::string(const std::string&)>;
  using stack_callable_object = callable_object::stack_callable_object<256>;

  stack_callable_object obj;

  {
    std::string s1 = obj.call("A");
    REQUIRE(s1 == "");
  }

  {
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "bingo");
  }

  obj.set([](const std::string& s) { return s; });

  REQUIRE(obj);

  {
    std::string s1 = obj("A");
    REQUIRE(s1 == "A");
  }

  {
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "A");
  }

  obj.reset();

  {
    std::string s1 = obj("A");
    REQUIRE(s1 == "");
  }

  {
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "bingo");
  }

  REQUIRE(!obj);

  obj.set(callable_object::create_unique([](const std::string& s) { return "B"; }));

  REQUIRE(obj);

  {
    std::string s1 = obj("A");
    REQUIRE(s1 == "B");
  }

  {
    std::function<void()> asss;
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "B");
  }

  obj.set(nullptr);

  obj.reset();

  {
    std::string s1 = obj("A");
    REQUIRE(s1 == "");
  }

  {
    std::string s1 = obj.call_or("A", "bingo");
    REQUIRE(s1 == "bingo");
  }

  REQUIRE(!obj);
}


