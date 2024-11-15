#include <zscript/objects/object_include_guard.h>

namespace zs {
class native_array_object_interface : public delegate_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  virtual ~native_array_object_interface() override = default;

  virtual zs::error_result get(int_t idx, object& dst) const = 0;
  virtual zs::error_result set(int_t idx, const object& obj) = 0;

  virtual zs::error_result push(const object& obj) = 0;

  virtual int_t size() const noexcept = 0;

  virtual native_array_object_interface* clone() const noexcept = 0;

protected:
  inline native_array_object_interface(zs::engine* eng) noexcept
      : delegate_object(eng, zs::object_type::k_native_array) {}
};

/// native_array_object.
template <class T>
class native_array_object final : public native_array_object_interface, public zs::vector<T> {
public:
  ZS_OBJECT_CLASS_COMMON;

  using value_type = T;
  using vector_type = zs::vector<value_type>;
  using vector_type::operator[];
  using vector_type::begin;
  using vector_type::end;

  static constexpr native_array_type native_type = to_native_array_type<value_type>();

  static_assert(native_type != native_array_type::n_invalid, "invalid type");

  ZS_CHECK inline static native_array_object* create(zs::engine* eng, int_t sz) {
    native_array_object* arr = internal::zs_new<native_array_object>(eng, eng);

    if (sz) {
      arr->resize(sz);
    }

    return arr;
  }

  virtual ~native_array_object() override = default;

  ZS_INLINE void resize(size_t sz) { vector_type::resize(sz); }

  ZS_CHECK virtual int_t size() const noexcept override { return (int_t)vector_type::size(); }
  ZS_CHECK bool empty() const noexcept { return vector_type::empty(); }

  ZS_INLINE zs::error_result get(int_t idx, value_type& dst) const {
    if (idx >= (int_t)vector_type::size()) {
      return zs::error_code::out_of_bounds;
    }

    dst = vector_type::operator[](idx);
    return {};
  }

  ZS_INLINE zs::error_result set(int_t idx, const value_type& obj) {
    if (idx >= (int_t)vector_type::size()) {
      return zs::error_code::out_of_bounds;
    }

    vector_type::operator[](idx) = obj;
    return {};
  }

  ZS_INLINE zs::error_result push(const value_type& obj) {
    vector_type::push_back(obj);
    return {};
  }

  ZS_INLINE zs::error_result push(value_type&& obj) {
    vector_type::push_back(std::move(obj));
    return {};
  }

  virtual native_array_object* clone() const noexcept override {
    native_array_object* arr = native_array_object::create(_engine, 0);
    ((vector_type&)*arr) = *this;
    return arr;
  }

  ZS_CK_INLINE vector_type& get_vector() noexcept { return *this; }
  ZS_CK_INLINE const vector_type& get_vector() const noexcept { return *this; }

  //
  // Interface.
  //

  virtual zs::error_result get(int_t idx, object& dst) const override {
    const int_t sz = vector_type::size();
    // TODO: Not sure about this.
    idx = (idx + sz) % sz;
    dst = vector_type::operator[](idx);
    return {};
  }

  virtual zs::error_result set(int_t idx, const object& obj) override {
    if (idx >= (int_t)vector_type::size()) {
      return zs::error_code::out_of_bounds;
    }

    value_type val = 0;
    if (auto err = obj.get_value<value_type>(val)) {
      return err;
    }

    vector_type::operator[](idx) = val;
    return {};
  }

  virtual zs::error_result push(const object& obj) override {
    value_type val = 0;
    if (auto err = obj.get_value<value_type>(val)) {
      return err;
    }

    vector_type::push_back(val);
    return {};
  }

private:
  ZS_INLINE native_array_object(zs::engine* eng) noexcept
      : native_array_object_interface(eng)
      , vector_type((zs::allocator<T>(eng))) {}
};
} // namespace zs.
