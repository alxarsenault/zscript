#include <ztests/ztests.h>
#include <zbase/container/small_vector.h>
#include <string>

TEST_CASE("zb::small_vector") {

  zb::small_vector<int, 32> vec;
  vec.push_back(21);

  REQUIRE(vec[0] == 21);

  zb::small_vector_base<int>& vb = vec;
  REQUIRE(vb[0] == 21);

  REQUIRE(vec.find(21) != vec.end());

  REQUIRE(vec.pfind(21));
  REQUIRE(*vec.pfind(21) == 21);
}

template <class T>
class my_empty_allocator {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  inline explicit constexpr my_empty_allocator() noexcept = default;

  inline constexpr my_empty_allocator(const my_empty_allocator&) noexcept = default;
  inline constexpr my_empty_allocator(my_empty_allocator&&) noexcept = default;

  template <class U>
  inline constexpr my_empty_allocator(const my_empty_allocator<U>& a) noexcept {}

  inline constexpr my_empty_allocator& operator=(const my_empty_allocator&) noexcept = default;
  inline constexpr my_empty_allocator& operator=(my_empty_allocator&&) noexcept = default;

  inline constexpr T* allocate(size_t n) {
    if (n > std::allocator_traits<my_empty_allocator>::max_size(*this)) {
      throw std::runtime_error("out of mem");
      //      zs::throw_exception(zs::error_code::out_of_memory);
    }

    return static_cast<T*>(::malloc(n * sizeof(T)));
  }

  inline constexpr void deallocate(T* ptr, size_t) noexcept { ::free(ptr); }

  inline constexpr bool operator==(const my_empty_allocator& a) const noexcept { return true; }

  template <class U>
  inline constexpr bool operator==(const my_empty_allocator<U>& a) const noexcept {
    return true;
  }

private:
  template <class U>
  friend class my_empty_allocator;
};

template <class T>
class my_allocator {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::false_type;

  inline explicit constexpr my_allocator(void* eng) noexcept
      : _engine(eng) {}

  inline constexpr my_allocator(const my_allocator&) noexcept = default;
  inline constexpr my_allocator(my_allocator&&) noexcept = default;

  template <class U>
  inline constexpr my_allocator(const my_allocator<U>& a) noexcept
      : _engine(a.get_engine()) {}

  inline constexpr my_allocator& operator=(const my_allocator&) noexcept = default;
  inline constexpr my_allocator& operator=(my_allocator&&) noexcept = default;

  inline constexpr T* allocate(size_t n) {
    if (ZBASE_UNLIKELY(n > std::allocator_traits<my_allocator>::max_size(*this))) {
      throw std::runtime_error("out of mem");
      //      zs::throw_exception(zs::error_code::out_of_memory);
    }

    return static_cast<T*>(::malloc(n * sizeof(T)));
  }

  inline constexpr void deallocate(T* ptr, size_t sz) noexcept { ::free(ptr); }

  inline constexpr bool operator==(const my_allocator& a) const noexcept { return _engine == a.get_engine(); }

  template <class U>
  inline constexpr bool operator==(const my_allocator<U>& a) const noexcept {
    return _engine == a.get_engine();
  }

  inline void* get_engine() const noexcept { return _engine; }

private:
  void* _engine;

  template <class U>
  friend class my_allocator;
};

TEST_CASE("small_vector") {
  {
    zb::small_vector<int, 32, my_empty_allocator<int>> vec;
    vec.resize(512);
  }
  {
    //    std::vector<int> vec;
    zb::small_vector<int, 32, my_allocator<int>> vec(my_allocator<int>(nullptr));
    vec.resize(512);
  }
}
