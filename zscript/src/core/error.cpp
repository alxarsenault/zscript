#include <zscript/core/zcore.h>
#include <zbase/strings/stack_string.h>

namespace zs {

struct error_code_exception : zs::exception {
  inline error_code_exception(error_code error) noexcept
      : _error{ error } {}

  virtual ~error_code_exception() override = default;

  virtual const char* what() const noexcept override { return error_result(_error).message(); }

  ZB_CHECK ZB_INLINE constexpr error_code error() const noexcept { return _error; }

private:
  error_code _error;
};

struct logic_exception : zs::exception {
  inline logic_exception(std::string_view msg) noexcept
      : _msg(msg.size() <= _msg.maximum_size ? msg : msg.substr(0, _msg.maximum_size)) {}

  virtual ~logic_exception() override = default;

  ZB_CHECK virtual const char* what() const noexcept override { return _msg.c_str(); }

private:
  zb::stack_string<512> _msg;
};

struct object_exception : zs::logic_exception {
  using logic_exception::logic_exception;
  virtual ~object_exception() override = default;
};

struct lexer_exception : zs::logic_exception {
  using logic_exception::logic_exception;
  virtual ~lexer_exception() override = default;
};

struct compiler_exception : zs::logic_exception {
  using logic_exception::logic_exception;
  virtual ~compiler_exception() override = default;
};

void throw_exception(zs::error_code ec) { throw zs::error_code_exception(ec); }

void throw_exception(zs::error_code ec, std::string_view msg) { throw zs::error_code_exception(ec); }

void throw_exception(zs::error_type e, std::string_view msg) {
  switch (e) {
  case zs::error_type::logic:
    throw zs::logic_exception(msg);
  case zs::error_type::object:
    throw zs::object_exception(msg);
  case zs::error_type::lexer:
    throw zs::lexer_exception(msg);
  case zs::error_type::compiler:
    throw zs::compiler_exception(msg);
  default:
    throw zs::logic_exception(msg);
  }
}
} // namespace zs.
