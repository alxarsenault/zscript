#include <ztests/ztests.h>
#include  <zbase/utility/traits.h>
  
TEST_CASE("zb::dsadasdasd") {

  enum class some_values { a, b, c, d, e = 672 };

#define TTT(type, value) zb::type_value_pair<type, value>
  using tmape = zb::type_map<TTT(double, some_values::c), TTT(int, some_values::e)>;
 
  {

    constexpr auto MyValue = some_values::e;
    using values = tmape::value_part;
    constexpr size_t Index = values::value_index<MyValue>();
    using types = tmape::types_part;
   using  Type = types::type_at_index<Index>;

    ZBASE_MAYBE_UNUSED Type v = 32;
    //    zb::print("TYPE", typeid(Type).name());
  }

  {

    constexpr auto MyValue = some_values::c;
    using values = tmape::value_part;
    constexpr size_t Index = values::value_index<MyValue>();
    using types = tmape::types_part;
    using Type = types::type_at_index<Index>;

    ZBASE_MAYBE_UNUSED Type v = 2.2;
    //    zb::print("TYPE", typeid(Type).name());
  }

  {
    using MyType = int;
    using types = tmape::types_part;
    using values = tmape::value_part;
    constexpr size_t Index = types::type_index<MyType>();

    constexpr auto v = values::value_at_index<Index>();
    REQUIRE(v == some_values::e);

    //    zb::print("TYPE VALUE ", v, typeid(v).name());
  }
}

TEST_CASE("zb::tyoecoh") {
  {
    using type = zb::type_list<int, double, bool, int, int>;

    REQUIRE(type::k_size == 5);
    REQUIRE(type::size() == 5);

    REQUIRE(type::type_index<int>() == 0);
    REQUIRE(type::type_index<double>() == 1);
    REQUIRE(type::type_index<bool>() == 2);
    REQUIRE(type::type_index<void*>() == -1);

    REQUIRE(type::k_index<int> == 0);
    REQUIRE(type::k_index<double> == 1);
    REQUIRE(type::k_index<bool> == 2);
    REQUIRE(type::k_index<void*> == -1);

    REQUIRE(type::k_count<int> == 3);
    REQUIRE(type::k_count<double> == 1);
    REQUIRE(type::k_count<bool> == 1);
    REQUIRE(type::k_count<void*> == 0);

    {
      using slist = type::sub_list<0, 1, 3>;
      REQUIRE(slist::k_size == 3);
      REQUIRE(slist::k_index<int> == 0);
      REQUIRE(slist::k_index<double> == 1);
      REQUIRE(type::k_index<void*> == -1);

      REQUIRE(slist::k_count<int> == 2);
    }
  }

  {
    using type = zb::type_list<int, double, bool>;
    using rlist = type::reversed_list;
    REQUIRE(rlist::k_size == 3);
    REQUIRE(rlist::k_index<bool> == 0);
    REQUIRE(rlist::k_index<double> == 1);
    REQUIRE(rlist::k_index<int> == 2);
    REQUIRE(rlist::k_index<void*> == -1);
  }

  {
    using type = zb::type_list<int, double, bool>;
    using slist = type::n_first_list<2>;
    REQUIRE(slist::k_size == 2);
    REQUIRE(slist::k_index<int> == 0);
    REQUIRE(slist::k_index<double> == 1);
  }

  {
    using type = zb::type_list<int, double, bool, char*>;
    using slist = type::n_last_list<2>;
    REQUIRE(slist::k_size == 2);
    REQUIRE(slist::k_index<bool> == 0);
    REQUIRE(slist::k_index<char*> == 1);
  }

  {
    using type = zb::type_list<int, double, bool, char*>;
    using slist = type::n_last_list<3>;
    REQUIRE(slist::k_size == 3);
    REQUIRE(slist::k_index<double> == 0);
    REQUIRE(slist::k_index<bool> == 1);
    REQUIRE(slist::k_index<char*> == 2);
  }
}

TEST_CASE("DSKJDSKJDSK") {
  {
    enum class some_values { a, b, c };
    enum class other_values { d, e, f };

    using type = zb::value_list<some_values::a, other_values::e>;

    //    zb::print(type::value_at_index<0>());
    //    zb::print(type::value_at_index<1>());
    REQUIRE(type::value_at_index<0>() == some_values::a);
    REQUIRE(type::value_at_index<0>() != some_values::b);

    // Doesn't compile.
    // All good.
    // REQUIRE(type::value_at_index<0>() != other_values::e);

    REQUIRE(type::value_at_index<1>() == other_values::e);

    REQUIRE(type::value_index<other_values::f>() == -1);
  }
}
