#pragma once

#include <zscript/zscript.h>
#include "lang/zparser.h"
#include <zbase/strings/string_view.h>

namespace zs {
class cpp_compiler : public engine_holder {
public:
  cpp_compiler(zs::engine* eng, zs::parser* p);

  ~cpp_compiler();

  zs::error_result generate(zb::string_view filename, zb::string_view fct_name, zb::string_view namespc = "",
      std::span<zs::parameter_info> vars = {});

  zs::error_result generate(zb::string_view filename, zb::string_view fct_name, zb::string_view namespc = "",
      std::initializer_list<zs::parameter_info> vars = {});

  zs::error_result export_code(const std::filesystem::path& directory) const;

  inline zs::ostringstream& stream() { return _stream; }
  inline zs::ostringstream& header_stream() { return _header_stream; }

  std::string get_header_content() const;
  std::string get_source_content() const;

private:
  zs::parser* _parser;
  zs::string _filename;
  zs::ostringstream _stream;
  zs::ostringstream _header_stream;
  zs::ostringstream _pre_stream;
  zs::ostringstream _post_stream;
  zb::indent_t<char> _indent = { 0, 2 };
  zs::object _line_comment;

  template <ast_node_type NodeType, class... Args>
  zs::error_result gen(const zs::object& node, Args... args);

  template <class... Args>
  inline std::ostream& write(const Args&... args) {
    return zb::stream_print(_stream, args...);
  }

  template <class... Args>
  inline std::ostream& writee(const Args&... args) {
    return zb::stream_print(_stream, args..., "\n");
  }

  template <class... Args>
  inline std::ostream& iwrite(const Args&... args) {
    return zb::stream_print(_stream, _indent, args...);
  }

  template <int Indent, class... Args>
  inline std::ostream& iwrite(const Args&... args) {
    _indent += Indent;
    return zb::stream_print(_stream, _indent, args...);
  }

  template <class... Args>
  inline std::ostream& iwritee(const Args&... args) {
    return zb::stream_print(_stream, _indent, args..., "\n");
  }

  template <int Indent, class... Args>
  inline std::ostream& iwritee(const Args&... args) {
    _indent += Indent;
    return zb::stream_print(_stream, _indent, args..., "\n");
  }

  template <class... Args>
  inline std::ostream& eiwrite(const Args&... args) {
    return zb::stream_print(_stream, "\n", _indent, args...);
  }

  template <int Indent, class... Args>
  inline std::ostream& eiwrite(const Args&... args) {
    _indent += Indent;
    return zb::stream_print(_stream, "\n", _indent, args...);
  }
  template <class... Args>
  inline std::ostream& eiwritee(const Args&... args) {
    return zb::stream_print(_stream, "\n", _indent, args..., "\n");
  }

  template <int Indent, class... Args>
  inline std::ostream& eiwritee(const Args&... args) {
    _indent += Indent;
    return zb::stream_print(_stream, "\n", _indent, args..., "\n");
  }

  inline std::ostream& endl() { return _stream << "\n"; }

  zs::error_result arith_eq_op(const zs::object& node, std::string_view op, std::string_view ops);
};
} // namespace zs.
