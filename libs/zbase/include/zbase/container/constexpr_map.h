
// https://github.com/mapbox/eternal/tree/master

#pragma once

#include <zbase/zbase.h>
#include <utility>
#include <functional>
#include <cstdint>

ZBASE_BEGIN_NAMESPACE
namespace eternal {
namespace impl {

  template <typename Key>
  struct compare_key {
    const Key key;

    constexpr compare_key(const Key& key_) noexcept
        : key(key_) {}

    template <typename Element>
    constexpr bool operator<(const Element& rhs) const noexcept {
      return key < rhs->first;
    }
  };

  template <typename Key, typename Value>
  class element {
  public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<key_type, mapped_type>;
    using compare_key_type = compare_key<key_type>;

    inline constexpr element(const key_type& key, const mapped_type& value) noexcept
        : pair(key, value) {}

    ZB_CHECK inline constexpr bool operator<(const element& rhs) const noexcept {
      return pair.first < rhs.pair.first;
    }

    ZB_CHECK inline constexpr bool operator<(const compare_key_type& rhs) const noexcept {
      return pair.first < rhs.key;
    }

    ZB_CHECK inline constexpr const auto& operator*() const noexcept { return pair; }

    ZB_CHECK inline constexpr const auto* operator->() const noexcept { return &pair; }

    inline constexpr void swap(element& rhs) noexcept {
      std::swap(pair.first, rhs.pair.first);
      std::swap(pair.second, rhs.pair.second);
    }

  private:
    value_type pair;
  };

  template <class Key, class Hasher = std::hash<Key>>
  struct compare_key_hash : compare_key<Key> {
    using base_type = compare_key<Key>;
    const size_t hash;

    inline constexpr compare_key_hash(const Key& key_) noexcept
        : base_type(key_)
        , hash(Hasher()(key_)) {}

    template <typename Element>
    ZB_CHECK inline constexpr bool operator<(const Element& rhs) const noexcept {
      return hash < rhs.hash || (!(rhs.hash < hash) && base_type::operator<(rhs));
    }
  };

  template <typename Key, typename Value, typename Hasher = std::hash<Key>>
  class element_hash : public element<Key, Value> {
    using base_type = element<Key, Value>;

  public:
    using key_type = Key;
    using mapped_type = Value;
    using compare_key_type = compare_key_hash<key_type, Hasher>;

    friend compare_key_type;

    constexpr element_hash(const key_type& key, const mapped_type& value) noexcept
        : base_type(key, value)
        , hash(Hasher()(key)) {}

    template <typename T>
    constexpr bool operator<(const T& rhs) const noexcept {
      return hash < rhs.hash || (!(rhs.hash < hash) && base_type::operator<(rhs));
    }

    constexpr void swap(element_hash& rhs) noexcept {
      std::swap(hash, rhs.hash);
      base_type::swap(rhs);
    }

  private:
    size_t hash;
  };

} // namespace impl

template <typename Element>
class iterator {
public:
  inline constexpr iterator(const Element* pos_) noexcept
      : pos(pos_) {}

  ZB_CHECK inline constexpr bool operator==(const iterator& rhs) const noexcept { return pos == rhs.pos; }

  ZB_CHECK inline constexpr bool operator!=(const iterator& rhs) const noexcept { return pos != rhs.pos; }

  inline constexpr iterator& operator++() noexcept {
    ++pos;
    return *this;
  }

  inline constexpr iterator& operator+=(size_t i) noexcept {
    pos += i;
    return *this;
  }

  ZB_CHECK inline constexpr iterator operator+(size_t i) const noexcept { return pos + i; }

  inline constexpr iterator& operator--() noexcept {
    --pos;
    return *this;
  }

  inline constexpr iterator& operator-=(size_t i) noexcept {
    pos -= i;
    return *this;
  }

  ZB_CHECK inline constexpr size_t operator-(const iterator& rhs) const noexcept { return pos - rhs.pos; }

  ZB_CHECK inline constexpr const auto& operator*() const noexcept { return **pos; }

  ZB_CHECK inline constexpr const auto* operator->() const noexcept { return &**pos; }

private:
  const Element* pos;
};

namespace impl {

  template <typename Compare, typename Iterator, typename Key>
  ZB_CHECK inline constexpr auto bound(Iterator left, Iterator right, const Key& key) noexcept {
    size_t count = right - left;
    while (count > 0) {
      const size_t step = count / 2;
      right = left + step;
      if (Compare()(*right, key)) {
        left = ++right;
        count -= step + 1;
      }
      else {
        count = step;
      }
    }
    return left;
  }

  struct less {
    template <typename A, typename B>
    ZB_CHECK inline constexpr bool operator()(const A& a, const B& b) const noexcept {
      return a < b;
    }
  };

  struct greater_equal {
    template <typename A, typename B>
    ZB_CHECK inline constexpr bool operator()(const A& a, const B& b) const noexcept {
      return !(b < a);
    }
  };

  template <typename Element, size_t N>
  class map {
  private:
    static_assert(N > 0, "map is empty");

    Element data_[N];

    template <typename T, size_t... I>
    inline constexpr map(const T (&data)[N], std::index_sequence<I...>) noexcept
        : data_{ { data[I].first, data[I].second }... } {
      static_assert(sizeof...(I) == N, "index_sequence has identical length");
      // Yes, this is a bubblesort. It's usually evaluated at compile-time, it's fast for data
      // that is already sorted (like static maps), it has a small code size, and it's stable.
      for (auto left = data_, right = data_ + N - 1; data_ < right; right = left, left = data_) {
        for (auto it = data_; it < right; ++it) {
          if (it[1] < it[0]) {
            it[0].swap(it[1]);
            left = it;
          }
        }
      }
    }

    using compare_key_type = typename Element::compare_key_type;

  public:
    template <typename T>
    inline constexpr map(const T (&data)[N]) noexcept
        : map(data, std::make_index_sequence<N>()) {}

    using key_type = typename Element::key_type;
    using mapped_type = typename Element::mapped_type;
    using value_type = typename Element::value_type;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using const_reference = const value_type&;
    using const_pointer = const value_type*;
    using const_iterator = iterator<Element>;

    ZB_CHECK inline constexpr bool unique() const noexcept {
      for (auto right = data_ + N - 1, it = data_; it < right; ++it) {
        if (!(it[0] < it[1])) {
          return false;
        }
      }
      return true;
    }

    ZB_CHECK inline constexpr const mapped_type& at(const key_type& key) const noexcept {
      return find(key)->second;
    }

    ZB_CHECK inline constexpr size_t size() const noexcept { return N; }

    ZB_CHECK inline constexpr const_iterator begin() const noexcept { return data_; }

    ZB_CHECK inline constexpr const_iterator cbegin() const noexcept { return begin(); }

    ZB_CHECK inline constexpr const_iterator end() const noexcept { return data_ + N; }

    ZB_CHECK inline constexpr const_iterator cend() const noexcept { return end(); }

    ZB_CHECK inline constexpr const_iterator lower_bound(const key_type& key) const noexcept {
      return bound<less>(data_, data_ + N, compare_key_type{ key });
    }

    ZB_CHECK inline constexpr const_iterator upper_bound(const key_type& key) const noexcept {
      return bound<greater_equal>(data_, data_ + N, compare_key_type{ key });
    }

    ZB_CHECK inline constexpr std::pair<const_iterator, const_iterator> equal_range(
        const key_type& key) const noexcept {
      const compare_key_type compare_key{ key };
      auto first = bound<less>(data_, data_ + N, compare_key);
      return { first, bound<greater_equal>(first, data_ + N, compare_key) };
    }

    ZB_CHECK inline constexpr size_t count(const key_type& key) const noexcept {
      const auto range = equal_range(key);
      return range.second - range.first;
    }

    ZB_CHECK inline constexpr const_iterator find(const key_type& key) const noexcept {
      const compare_key_type compare_key{ key };
      auto it = bound<less>(data_, data_ + N, compare_key);
      if (it != data_ + N && greater_equal()(*it, compare_key)) {
        return it;
      }
      else {
        return end();
      }
    }

    ZB_CHECK inline constexpr bool contains(const key_type& key) const noexcept { return find(key) != end(); }
  };

  // Use different constants for 32 bit vs. 64 bit size_t.
  constexpr size_t hash_offset
      = std::conditional_t<sizeof(size_t) < 8, std::integral_constant<uint32_t, 0x811C9DC5>,
          std::integral_constant<uint64_t, 0xCBF29CE484222325>>::value;

  constexpr size_t hash_prime = std::conditional_t<sizeof(size_t) < 8,
      std::integral_constant<uint32_t, 0x1000193>, std::integral_constant<uint64_t, 0x100000001B3>>::value;

  // FNV-1a hash
  ZB_CHECK inline constexpr static size_t str_hash(
      std::string_view str, const size_t value = hash_offset) noexcept {
    return !str.empty() ? str_hash(str.substr(1), (value ^ static_cast<size_t>(str[0])) * hash_prime) : value;
  }

  ZB_CHECK inline constexpr bool str_less(const char* lhs, const char* rhs) noexcept {
    return *lhs && *rhs && *lhs == *rhs ? str_less(lhs + 1, rhs + 1) : *lhs < *rhs;
  }

  ZB_CHECK inline constexpr bool str_equal(const char* lhs, const char* rhs) noexcept {
    return *lhs == *rhs && (*lhs == '\0' || str_equal(lhs + 1, rhs + 1));
  }

} // namespace impl

struct string {
  inline constexpr string(const char* data) noexcept
      : _data(data) {}

  inline constexpr string(std::string_view data) noexcept
      : _data(data) {}

  inline constexpr string(const string&) noexcept = default;
  inline constexpr string(string&&) noexcept = default;
  inline constexpr string& operator=(const string&) noexcept = default;
  inline constexpr string& operator=(string&&) noexcept = default;

  ZB_CHECK inline constexpr bool operator<(const string& rhs) const noexcept { return _data < rhs._data; }

  ZB_CHECK inline constexpr bool operator==(const string& rhs) const noexcept { return _data == rhs._data; }

  std::string_view _data;
};

template <typename Key, typename Value, size_t N>
ZB_CHECK inline static constexpr auto map(const std::pair<const Key, const Value> (&items)[N]) noexcept {
  return impl::map<impl::element<Key, Value>, N>(items);
}

template <typename Key, typename Value, size_t N>
ZB_CHECK inline static constexpr auto hash_map(const std::pair<const Key, const Value> (&items)[N]) noexcept {
  return impl::map<impl::element_hash<Key, Value>, N>(items);
}

} // namespace eternal
ZBASE_END_NAMESPACE

namespace std {

template <>
struct hash<::zb::eternal::string> {
  constexpr size_t operator()(const ::zb::eternal::string& str) const {
    return ::zb::eternal::impl::str_hash(str._data);
  }
};

} // namespace std
