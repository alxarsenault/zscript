#include "lang/preprocessor/zmacro.h"

namespace zs {
bool macro::has_default_values() const noexcept {
  for (const object& obj : default_values.as_array()) {
    if (!obj.is_none()) {
      return true;
    }
  }

  return false;
}

size_t macro::first_default_value_index() const noexcept {
  size_t i = 0;
  for (const object& obj : default_values.as_array()) {
    if (!obj.is_none()) {
      return i;
    }
    ++i;
  }

  return -1;
}

std::ostream& operator<<(std::ostream& stream, const macro& m) {
  return stream << "name: " << m.name << " params: " << m.params << " content: " << m.content
                << " default_values: " << m.default_values;
}
} // namespace zs.
