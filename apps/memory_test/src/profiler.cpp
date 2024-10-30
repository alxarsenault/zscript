
#include "profiler.h"

#include <chrono>
#include <format>
#include <bit>

namespace {

template <class T>
class profiler_allocator {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  inline constexpr profiler_allocator() noexcept = default;

  inline constexpr profiler_allocator(const profiler_allocator&) noexcept = default;
  inline constexpr profiler_allocator(profiler_allocator&&) noexcept = default;

  template <class U>
  inline constexpr profiler_allocator(const profiler_allocator<U>& a) noexcept {}

  inline constexpr profiler_allocator& operator=(profiler_allocator&) noexcept = default;
  inline constexpr profiler_allocator& operator=(profiler_allocator&&) noexcept = default;

  inline constexpr T* allocate(size_t n) {
    if (ZBASE_UNLIKELY(n > std::allocator_traits<profiler_allocator>::max_size(*this))) {
      zs::throw_exception(zs::error_code::out_of_memory);
    }

    return static_cast<T*>(::malloc(n * sizeof(T)));
  }

  inline constexpr void deallocate(T* ptr, size_t) noexcept { ::free(ptr); }

  inline constexpr bool operator==(const profiler_allocator& a) const noexcept { return true; }

  template <class U>
  inline constexpr bool operator==(const profiler_allocator<U>& a) const noexcept {
    return true;
  }

  template <class U>
  struct rebind {
    using other = profiler_allocator<U>;
  };

  template <class U>
  friend class profiler_allocator;
};

using memory_map = std::unordered_map<void*, size_t, std::hash<void*>, std::equal_to<void*>,
    profiler_allocator<std::pair<void* const, size_t>>>;

using memory_tag_map = std::unordered_map<void*, zs::alloc_info_t, std::hash<void*>, std::equal_to<void*>,
    profiler_allocator<std::pair<void* const, zs::alloc_info_t>>>;

static memory_map s_addr_map;
static memory_tag_map s_tag_map;

static bool s_verbose = true;

static size_t s_malloc_count = 0;
static size_t s_free_count = 0;
static size_t s_realloc_count = 0;
static int64_t s_mem_size = 0;
static size_t s_max_mem_size = 0;
static size_t s_max_alloc_size = 0;

static void profiler_print(
    std::string_view name, const void* ptr, size_t size, zs::memory_tag info) {
  if (s_verbose) {

    intptr_t p = (intptr_t)ptr;

    uint64_t alignment = (1 << std::countr_zero((uint64_t)p));
    bool is_p2_aligned = std::bit_ceil(alignment) == std::bit_floor(alignment);
    //    std::cout << (1<<std::countr_zero((uint64_t)p)) << " " <<zb::is_aligned(ptr, 32) << " "<<
    //    std::bitset<64>(p) << " " << (p & 0b1111) << " --- " << std::countr_zero((uint64_t)p) << "\n";
    std::cout << "│ " << std::format("{:%T}", std::chrono::system_clock::now()) << //
        " │ " << std::format("{: <6s}", name) << //
        " │ " << std::format("{: <18s}", zb::enum_name(info)) << //
        " │" << std::format("{:6d}", size) << //
        " │" << std::format("{:6d}", s_mem_size) << //
        " │ " << std::format("{:#016x}", p) << //
        " │" << std::format("{:5d} |", zb::minimum(alignment, 2048)) << "\n"; //

    //    zb::print( //
    //        std::format("| {:%T}", std::chrono::system_clock::now()), //
    //        std::format("| {: <6s}", name), //
    //              std::format("| {: <18s}", zb::enum_name(info)), //
    //        std::format("|{:6d}", size), //
    //        std::format("|{:6d}", s_mem_size), //
    //        std::format("| {:#016x} |", (intptr_t)ptr) //
    //    );
  }
}

static zs::allocate_t s_engine_allocate = [](zs::engine* eng, zs::raw_pointer_t user_ptr, void* ptr,
                                              size_t size, size_t old_size, zs::alloc_info_t info) -> void* {
  if (!size) {
    zbase_assert(ptr, "invalid pointer");
    size_t ptr_sz = s_addr_map[ptr];

    zs::alloc_info_t sinfo = s_tag_map[ptr];
    s_tag_map.erase(ptr);

    s_mem_size -= ptr_sz;
    size_t n = s_addr_map.erase(ptr);
    zbase_warning(n, "could not find pointer");

    ::free(ptr);

    s_free_count++;
    profiler_print("free", ptr, ptr_sz, (zs::memory_tag)(info ? info : sinfo));

    return nullptr;
  }

  if (ptr) {
    s_tag_map.erase(ptr);

    s_mem_size -= s_addr_map[ptr];
    s_addr_map.erase(ptr);

    ptr = ::realloc(ptr, size);
    s_addr_map[ptr] = size;
    s_tag_map[ptr] = info;

    s_mem_size += size;
    s_max_mem_size = zb::maximum(s_max_mem_size, s_mem_size);
    s_max_alloc_size = zb::maximum(s_max_alloc_size, size);

    s_realloc_count++;
    profiler_print("ralloc", ptr, size, (zs::memory_tag)info);

    return ptr;
  }

  zbase_assert(!ptr, "pointer should be nullptr");
  ptr = ::malloc(size);
  s_addr_map[ptr] = size;
  s_tag_map[ptr] = info;

  s_mem_size += size;
  s_max_mem_size = zb::maximum(s_max_mem_size, s_mem_size);
  s_max_alloc_size = zb::maximum(s_max_alloc_size, size);

  s_malloc_count++;
  profiler_print("malloc", ptr, size, (zs::memory_tag)info);

  return ptr;
};
} // namespace

void profiler::init() {
  s_malloc_count = 0;
  s_realloc_count = 0;
  s_free_count = 0;
  s_mem_size = 0;
  s_max_mem_size = 0;
  s_max_alloc_size = 0;

  std::cout << "┌─────────────────┬────────┬────────────────────┬───────┬───────┬──────────────────┬──────┐"
            << std::endl;
  std::cout << "│      Time       │ Action │        Type        │ Size  │ Total │   Pointer Addr   │ Algn │"
            << std::endl;
  std::cout << "├─────────────────┼────────┼────────────────────┼───────┼───────┼──────────────────┼──────┤"
            << std::endl;
}

void profiler::set_verbose(bool verbose) { s_verbose = verbose; }

void profiler::terminate() {

  std::cout << "└─────────────────┴────────┴────────────────────┴───────┴───────┴──────────────────┴──────┘\n"
            << std::endl;

  zb::print("malloc count :", s_malloc_count);
  zb::print("realloc count:", s_realloc_count);
  zb::print("free count   :", s_free_count);
  zb::print("max mem (kB) :", double(s_max_mem_size) / 1024);
  zb::print("max alloc size (kB) :", double(s_max_alloc_size) / 1024);

  zb::print("memory", s_addr_map);
}

zs::allocate_t profiler::get_engine_allocator() { return s_engine_allocate; }

void* operator new(size_t n) {

  void* ptr = ::malloc(n);

  zb::print("NEW", ptr);
}

void operator delete(void* p) noexcept {
  zb::print("DELETE", p);
  ::free(p);
}
