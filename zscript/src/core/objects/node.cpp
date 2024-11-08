#include <zscript/zscript.h>

namespace zs {
node_object::node_object(zs::engine* eng) noexcept
    : delegate_object(eng, zs::object_type::k_node)
    , _children(zs::allocator<object>(eng))
    , _attributes(zs::allocator<attribute>(eng)) {}

node_object* node_object::create(zs::engine* eng, const object& name) noexcept {
  node_object* node = internal::zs_new<memory_tag::nt_node, node_object>(eng, eng);
  node->_name = name;

  return node;
}

node_object* node_object::create(zs::engine* eng, const object& name, const object& value) noexcept {
  node_object* node = internal::zs_new<memory_tag::nt_node, node_object>(eng, eng);
  node->_name = name;
  node->_value = value;

  return node;
}

zs::error_result node_object::get(int_t idx, object& dst) const noexcept {

  const int_t sz = _children.size();
  zbase_assert(sz, "call node::get in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  dst = _children[idx];
  return {};
}

zs::error_result node_object::set(int_t idx, const object& obj) noexcept {

  const int_t sz = _children.size();
  zbase_assert(sz, "call node::set in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  _children[idx] = obj;
  return {};
}

zs::error_result node_object::set(int_t idx, object&& obj) noexcept {

  const int_t sz = _children.size();
  zbase_assert(sz, "call node::set in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  _children[idx] = std::move(obj);
  return {};
}

zs::error_result node_object::add_child(const object& obj) noexcept {
  _children.push_back(obj);
  return {};
}

zs::error_result node_object::add_child(object&& obj) noexcept {
  _children.push_back(std::move(obj));
  return {};
}

node_object* node_object::clone() const noexcept {
  node_object* node = node_object::create(reference_counted_object::_engine, _name, _value);
  node->_children = _children;
  node->_attributes = _attributes;
  return node;
}

std::ostream& node_object::stream_internal(std::ostream& stream, int indent) const {
  stream << zb::indent_t(indent) << "<" << name().get_string_unchecked();

  if (const size_t sz = _attributes.size()) {
    for (size_t i = 0; i < sz; i++) {
      stream << " " << _attributes[i].name.get_string_unchecked() << "=" << _attributes[i].value;
    }
  }

  stream << ">";

  if (!_value.is_null()) {
    stream << "\n";
    stream << zb::indent_t(indent + 1) << _value;
  }

  if (const size_t sz = _children.size()) {
    stream << "\n";

    for (size_t i = 0; i < sz; i++) {
      if (_children[i].is_node()) {

        _children[i].as_node().stream_internal(stream, indent + 1);
        stream << "\n";
      }
      else if (_children[i].is_string()) {
        stream << zb::indent_t(indent + 1) << _children[i].get_string_unchecked() << "\n";
      }
      else {
        stream << zb::indent_t(indent + 1) << _children[i] << "\n";
      }
    }

    stream << zb::indent_t(indent) << "</" << name().get_string_unchecked() << ">";
  }
  else {
    stream << "\n" << zb::indent_t(indent) << "</" << name().get_string_unchecked() << ">";
  }

  return stream;
}

std::ostream& node_object::stream(std::ostream& stream) const { return stream_internal(stream); }

std::ostream& operator<<(std::ostream& stream, const node_object& node) { return node.stream(stream); }
} // namespace zs.
