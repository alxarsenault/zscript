#include <zscript/objects/object_include_guard.h>

namespace zs {
class native_closure_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  enum class closure_type : uint8_t { fct, obj };

  ZS_CHECK static native_closure_object* create(zs::engine* eng, zs::native_closure* closure);

  ZS_CHECK static native_closure_object* create(zs::engine* eng, zs::function_t fct);

  virtual ~native_closure_object() override;

  int_t call(vm_ref v);

  inline void set_user_pointer(zs::raw_pointer_t ptr) noexcept { _user_pointer = ptr; }

  inline zs::raw_pointer_t get_user_pointer() const noexcept { return _user_pointer; }

  inline void set_release_hook(native_closure_release_hook_t hook) noexcept { _release_hook = hook; }

  /// @param n_param_check
  ///         Defines the parameters number check policy (0 disables the param
  ///         checking).
  ///
  ///         If `n_param_check` is greater than 0, the VM ensures that the
  ///         number of parameters is exactly the number specified in
  ///         `n_param_check` (eg. if `n_param_check` == 3 the function can only
  ///         be called with 3 parameters).
  ///
  ///         If `n_param_check` is less than 0 the VM ensures that the closure
  ///         is called with at least the absolute value of the number specified
  ///         in `n_param_check` (eg. `n_param_check` == -3 will check that the
  ///         function is called with at least 3 parameters). The hidden
  ///         parameter ‘this’ is included in this number.
  ///
  ///         If SQ_MATCHTYPEMASKSTRING is passed instead of the number of
  ///         parameters, the function will automatically infer the number of
  ///         parameters to check from the typemask (eg. if the typemask is
  ///         “.sn”, it is like passing 3).
  ///
  /// @param typemask
  ///         Defines a mask to validate the parametes types passed to the
  ///         function. If the parameter is NULL, no typechecking is applied
  ///         (default).
  ///
  /// @remarks The typemask consists string that represent the expected
  /// parameter type.
  ///          The types are expressed as follows:
  ///           ‘o’ : null
  ///           ‘i’ : integer
  ///           ‘f’ : float
  ///           ‘n’ : integer or float
  ///           ‘s’ : string
  ///           ‘t’ : table
  ///           ‘a’ : array
  ///           ‘u’ : userdata
  ///           ‘c’ : closure and nativeclosure
  ///           ‘p’ : userpointer
  ///           ‘x’ : instance(class instance)
  ///           ‘y’ : class
  ///           ‘b’ : bool
  ///           ‘.’ : any type
  ///
  ///          The symbol ‘|’ can be used as `or` to accept multiple types on
  ///          the same parameter. There isn’t any limit on the number of `or`
  ///          that can be used. Spaces are ignored so can be inserted between
  ///          types to increase readability.
  ///
  ///          For instance to check a function that expect a table as ‘this’ a
  ///          string as first parameter and a number or a userpointer as second
  ///          parameter, the string would be “tsn|p” (table, string, number or
  ///          userpointer).
  ///
  ///          If the parameters mask is contains fewer parameters than
  ///          `n_param_check`, the remaining parameters will not be
  ///          typechecked.
  zs::error_result parse_type_check(int_t n_param_check, std::string_view typemask);

  ZS_CHECK native_closure_object* clone();

  ZS_CK_INLINE closure_type get_closure_type() const noexcept { return _ctype; }

private:
  native_closure_object() = delete;
  native_closure_object(zs::engine* eng, zs::native_closure* closure) noexcept;
  native_closure_object(zs::engine* eng, zs::function_t fct) noexcept;

  union callback_type {
    zs::native_closure* closure;
    zs::function_t fct;
  };

  callback_type _callback;
  native_closure_release_hook_t _release_hook = nullptr;
  zs::raw_pointer_t _user_pointer = nullptr;
  zs::vector<uint32_t> _type_check;
  int_t _n_param_check = 0;
  closure_type _ctype;
};
} // namespace zs.
