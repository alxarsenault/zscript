#include <zscript/utility/string_template.h>
#include "zvirtual_machine.h"
#include "lang/jit/zjit_compiler.h"

namespace zs {

static zs::object get_content(zs::vm_ref vm, const zs::object& tbl, std::string_view content) {

  zs::object closure = [](zs::vm_ref vm, std::string_view content) -> zs::object {
    zs::jit_compiler compiler(vm.get_engine());
    zs::object fct_state;

    zs::token_type tok = zs::token_type::tok_return;
    if (auto err = compiler.compile(content, "stemplate", fct_state, vm.get_virtual_machine(), &tok, false)) {
      return err;
    }

    return zs::_c(vm.get_engine(), std::move(fct_state), vm->get_root());
  }(vm, content);

  if (!closure.is_closure()) {
    return nullptr;
  }

  zs::object value;
  if (auto err = vm->call(closure, tbl, value)) {
    return err;
  }

  return value;
}

static inline bool is_null_none_or_error(const zs::object& obj) {
  return obj.is_type(zs::object_type::k_null, zs::object_type::k_none, zs::object_type::k_error);
}

zs::string render_template_string(zs::vm_ref vm, const zs::object& tbl, std::string_view content,
    std::string_view l_quote, std::string_view r_quote) {
  zs::engine* eng = vm.get_engine();

  zs::string output_str(eng);

  const char* it = content.data();
  const char* end = content.data() + content.size();
  const char* l_begin = it;

  const size_t lq_sz = l_quote.size();
  const size_t rq_sz = r_quote.size();

  size_t count_index = 0;

  enum class side : uint8_t { left, right };
  side side = side::left;

  while (it < end) {
    char c = *it++;

    if (side == side::left) {
      if (c == l_quote[count_index++]) {
        if (count_index == lq_sz) {
          side = side::right;

          if (zs::int_t sz = it - l_begin - lq_sz; sz > 0) {
            output_str.append(std::string_view(l_begin, sz));
          }

          l_begin = it;
          count_index = 0;
        }
      }
      else {
        count_index = 0;
      }
    }
    else {
      if (c == r_quote[count_index++]) {

        if (count_index == rq_sz) {
          std::string_view sstr = zb::strip_all(std::string_view(l_begin, it - rq_sz - l_begin));

          if (!sstr.empty()) {
            if (auto obj = get_content(vm, tbl, sstr); !is_null_none_or_error(obj)) {

              output_str.append(
                  (zs::create_string_stream(eng) << zs::streamer<zs::serializer_type::plain>(obj)).view());
              //              output_str.append((zs::create_string_stream(eng) << obj).view());
            }
            else {
              zb::print("ERRORRRRR");
            }
          }

          count_index = 0;
          side = side::left;
          l_begin = it;
        }
      }
      else {
        count_index = 0;
      }
    }
  }

  if (zs::int_t sz = end - l_begin; sz > 0) {
    output_str.append(std::string_view(l_begin, sz));
  }

  return output_str;
}

} // namespace zs.
