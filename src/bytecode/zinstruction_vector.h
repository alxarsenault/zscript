#pragma once

#include <zscript/zscript.h>
#include "zinstruction.h"
#include <span>

namespace zs {
class instruction_vector;

class instruction_iterator {
public:
  inline instruction_iterator() noexcept = default;

  inline instruction_iterator(const uint8_t* ptr) noexcept
      : _data(ptr) {}

  [[nodiscard]] inline zs::opcode get_opcode() const noexcept { return (zs::opcode)*_data; }

  [[nodiscard]] inline opcode operator*() const noexcept { return get_opcode(); }

  template <opcode Op>
  [[nodiscard]] inline const instruction_t<Op>* get() const noexcept {
    zbase_assert(get_opcode() == Op, "zs::instruction_stream::get - invalid opcode '", opcode_to_string(Op),
        "' expected '", opcode_to_string(get_opcode()), "'");
    return (const instruction_t<Op>*)_data;
  }

  template <opcode Op>
  [[nodiscard]] inline const instruction_t<Op>& get_ref() const noexcept {
    zbase_assert(get_opcode() == Op, "zs::instruction_stream::get - invalid opcode '", opcode_to_string(Op),
        "' expected '", opcode_to_string(get_opcode()), "'");
    return *(const instruction_t<Op>*)_data;
  }

  template <opcode Op>
  [[nodiscard]] inline operator const instruction_t<Op>&() const noexcept {
    zbase_assert(get_opcode() == Op, "zs::instruction_stream::get - invalid opcode '", opcode_to_string(Op),
        "' expected '", opcode_to_string(get_opcode()), "'");
    return *(const instruction_t<Op>*)_data;
  }

  inline instruction_iterator& operator++() noexcept {
    _data += zs::get_instruction_size(get_opcode());
    return *this;
  }

  inline instruction_iterator operator++(int) noexcept {
    instruction_iterator tmp(*this);
    _data += zs::get_instruction_size(get_opcode());
    return tmp;
  }

  //  inline instruction_iterator operator+(size_t n) const noexcept {
  //    instruction_iterator tmp(*this);
  //    while (n--) {
  //      tmp._data += zs::get_instruction_size(tmp.get_opcode());
  //    }
  //
  //    return tmp;
  //  }

  //  inline instruction_iterator operator[](size_t n) const noexcept { return operator+(n); }

  [[nodiscard]] inline bool operator==(instruction_iterator rhs) const noexcept { return _data == rhs._data; }
  [[nodiscard]] inline bool operator!=(instruction_iterator rhs) const noexcept { return _data != rhs._data; }
  [[nodiscard]] inline bool operator<(instruction_iterator rhs) const noexcept { return _data < rhs._data; }
  [[nodiscard]] inline bool operator<=(instruction_iterator rhs) const noexcept { return _data <= rhs._data; }
  [[nodiscard]] inline bool operator>(instruction_iterator rhs) const noexcept { return _data > rhs._data; }
  [[nodiscard]] inline bool operator>=(instruction_iterator rhs) const noexcept { return _data >= rhs._data; }

  [[nodiscard]] inline const uint8_t* data() const noexcept { return _data; }
  [[nodiscard]] inline const uint8_t*& data_ptr_ref() noexcept { return _data; }

  template <class Vec>
  inline size_t get_index(const Vec& inst_vec) const noexcept {
    return inst_vec.get_iterator_index(*this);
  }

  template <class Vec>
  inline size_t get_instruction_index(const Vec& inst_vec) const noexcept {
    return inst_vec.get_iterator_instruction_index(*this);
  }

private:
  friend class instruction_vector;
  const uint8_t* _data;
};

class instruction_vector {
public:
  using iterator = instruction_iterator;

  inline instruction_vector(zs::engine* eng)
      : _data(zs::allocator<uint8_t>(eng)) {}

  template <class Inst>
  inline void push(const Inst& inst) {
    _data.insert(_data.end(), (const uint8_t*)&inst, ((const uint8_t*)&inst) + sizeof(Inst));
  }

  template <opcode Op, class... Args>
  inline void push(Args... args) {
    instruction_t<Op> inst(Op, args...);
    _data.insert(_data.end(), (const uint8_t*)&inst, ((const uint8_t*)&inst) + get_instruction_size<Op>());
  }

  [[nodiscard]] inline iterator begin() const noexcept { return iterator(_data.data()); }

  [[nodiscard]] inline iterator end() const noexcept { return iterator(_data.data() + _data.size()); }

  [[nodiscard]] inline uint8_t* data() noexcept { return _data.data(); }
  [[nodiscard]] inline const uint8_t* data() const noexcept { return _data.data(); }

  [[nodiscard]] inline uint8_t* data(size_t index) noexcept { return _data.data() + index; }
  [[nodiscard]] inline const uint8_t* data(size_t index) const noexcept { return _data.data() + index; }

  //  ZB_CHECK ZB_INLINE pointer data(size_type index) noexcept { return data() + index; }
  //  ZB_CHECK ZB_INLINE const_pointer data(size_type index) const noexcept { return data() + index; }

  //  inline iterator operator[](size_t n) const noexcept { return begin() + n;
  //  }

  /// Byte offset.
  inline iterator operator[](size_t n) const noexcept { return iterator(_data.data() + n); }

  template <opcode Op>
  [[nodiscard]] inline instruction_t<Op>* get(size_t index) noexcept {
    instruction_t<Op>* inst = (instruction_t<Op>*)(_data.data() + index);
    zbase_assert(inst->op == Op, "zs::instruction_vector::get - invalid opcode '", opcode_to_string(Op),
        "' expected '", opcode_to_string(inst->op), "'");
    return inst;
  }

  template <opcode Op>
  [[nodiscard]] inline instruction_t<Op>& get_ref(size_t index) noexcept {
    instruction_t<Op>* inst = (instruction_t<Op>*)(_data.data() + index);
    zbase_assert(inst->op == Op, "zs::instruction_vector::get_ref - invalid opcode '", opcode_to_string(Op),
        "' expected '", opcode_to_string(inst->op), "'");
    return *inst;
  }

  [[nodiscard]] inline zs::opcode get_opcode(size_t index) const noexcept {
    return (zs::opcode) * (_data.data() + index);
  }

  //  inline iterator at_index(size_t n) const noexcept { return
  //  iterator(_data.data() + n); }

  inline size_t get_iterator_index(const iterator& it) const noexcept {
    return std::distance((const uint8_t*)_data.data(), (const uint8_t*)it.data());
  }

  inline size_t get_iterator_instruction_index(const iterator& it) const noexcept {
    auto b = begin();
    size_t count = 0;
    while (b != end()) {

      if (b == it) {
        return count;
      }
      count++;
      b._data += zs::get_instruction_size(b.get_opcode());
    }

    return -1;
  }

  void serialize(std::ostream& stream = std::cout) const;

  void debug_print(std::ostream& stream = std::cout) const;

  zs::vector<uint8_t> _data;
};

class instruction_stream {
public:
  using value_type = uint8_t;
  using span_type = std::span<const value_type>;
  using iterator = instruction_iterator;
  using difference_type = typename span_type::difference_type;

  inline instruction_stream() noexcept = default;

  inline instruction_stream(span_type s) noexcept
      : _data(s)
      , _it(s.begin()) {}

  inline instruction_stream(const instruction_vector& s) noexcept
      : instruction_stream(s._data) {}

  [[nodiscard]] inline iterator begin() const noexcept { return iterator(&(*_it)); }

  [[nodiscard]] inline iterator end() const noexcept { return iterator(_data.data() + _data.size()); }

  [[nodiscard]] inline zs::opcode get_opcode() const noexcept { return (zs::opcode)*_it; }

  template <opcode Op>
  [[nodiscard]] inline const instruction_t<Op>* get() const noexcept {
    zbase_assert(get_opcode() == Op, "zs::instruction_stream::get - invalid opcode '", opcode_to_string(Op),
        "' expected '", opcode_to_string(get_opcode()), "'");
    return (const instruction_t<Op>*)&(*_it);
  }

  inline zs::opcode next() noexcept {
    _it += zs::get_instruction_size(get_opcode());
    return get_opcode();
  }

  inline instruction_stream& operator++() noexcept {
    _it += zs::get_instruction_size(get_opcode());
    return *this;
  }

  inline instruction_stream operator++(int) noexcept {
    instruction_stream s = *this;
    _it += zs::get_instruction_size(get_opcode());
    return s;
  }

  //  inline iterator operator[](size_t n) const noexcept { return begin() + n;
  //  }

  [[nodiscard]] inline bool is_valid() const noexcept { return _it != _data.end(); }

  [[nodiscard]] inline bool is_next_valid() const noexcept {
    return _it + zs::get_instruction_size(get_opcode()) < _data.end();
  }

  [[nodiscard]] inline bool is_end() const noexcept { return _it == _data.end(); }

  [[nodiscard]] inline bool is_next_end() const noexcept {
    return _it + zs::get_instruction_size(get_opcode()) >= _data.end();
  }

private:
  std::span<const value_type> _data;
  typename std::span<const value_type>::iterator _it = {};
};
} // namespace zs.
