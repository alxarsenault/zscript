// @module argparser

/// Flag.
struct flag {
  string name;
  array cmd;
  string description;

  constructor(string name, var<string | array> cmd, string description = "") {
    this.name = name;
    this.description = description;

    if(zs::is_array(cmd)) {
      this.cmd = cmd;
    }
    else {
      this.cmd = [cmd];
    }
  }

  function set_description(string description) {
    this.description = description;
    return this;
  }
};

/// Option.
struct option {
  string name;
  array cmd;
  string description;
  bool required;
  bool multiple;

  constructor(string name, var<string | array> cmd,
      string description = "",
      bool required = false,
      bool multiple = false) {

    this.name = name;
    this.description = description;
    this.required = required;
    this.multiple = multiple;

    if(zs::is_array(cmd)) {
      this.cmd = cmd;
    }
    else {
      this.cmd = [cmd];
    }
  }

  function set_description(string description) {
    this.description = description;
    return this;
  }

  function set_required(bool required) {
    this.required = required;
    return this;
  }

  function set_multiple(bool multiple) {
    this.multiple = multiple;
    return this;
  }

  function add_cmd(string cmd) {
    this.cmd.push(cmd);
    return this;
  }
};

/// Argument.
struct argument {
  string name;
  string description;
  bool required;
  int count;

  constructor(string name, string description = "",
      bool required = false,
      int count = 1) {

    this.name = name;
    this.description = description;
    this.required = required;
    this.count = count;
  }

  function set_description(string description) {
    this.description = description;
    return this;
  }

  function set_required(bool required) {
    this.required = required;
    return this;
  }

  function set_count(int count) {
    this.count = count;
    return this;
  }
};

/// Argument parser.
struct argparser {
  static var argument = argument;
  static var option = option;
  static var flag = flag;
  string name;
  string description;
  var options = [];
  var arguments = [];
  var flags = [];

  constructor(string name, string description = "") {
    this.name = name;
    this.description = description;

    add_flag(flag("help", "--help", "Print help"));
  }

  function set_description(description) {
    this.description = description;
    return this;
  }

  function print_help() {
    io::print("HELP", this.name, this.description);

    for(int i = 0; i < this.arguments.size(); i++) {
      const arg = this.arguments[i];
      io::write(arg.name, " : ", arg.description, "\n");
    }

    for(int i = 0; i < this.options.size(); i++) {
      const opt = this.options[i];
      io::write(opt.name, " : ", opt.cmd, " ", opt.description, "\n");
    }

    for(int i = 0; i < this.flags.size(); i++) {
      const flg = this.flags[i];
      io::write(flg.name, " : ", flg.cmd, " ", flg.description, "\n");
    }
  }

  // Add an option.
  function add_option(var option_obj_or_name, var cmd = null,
      string description = "",
      bool required = false,
      bool multiple = false) {

    var opt;

    if(typeof(option_obj_or_name) == "string") {
      opt = option(option_obj_or_name, cmd, description, required, multiple);
    }
    else {
      opt = option_obj_or_name;
    }

    this.options.push(opt);
    return opt;
  }

  // Add an argument.
  function add_argument(var argument_obj_or_name,
      string description = "",
      bool required = false,
      int count = 1) {

    var arg;

    if(typeof(argument_obj_or_name) == "string") {
      arg = argument(argument_obj_or_name, description, required, count);
    }
    else {
      arg = argument_obj_or_name;
    }

    for(int i = 0; i < this.arguments.size(); i++) {
      if(!this.arguments[i].count ) {
        io::print("Can't have multiple arguments with multiple");
        return null;
      }
    }

    this.arguments.push(arg);
    return arg;
  }

  // Add a flag.
  function add_flag(var flag_obj_or_name, var cmd = null,
      string description = "") {

    var flg;

    if(typeof(flag_obj_or_name) == "string") {
      flg = flag(flag_obj_or_name, cmd, description);
    }
    else {
      flg = flag_obj_or_name;
    }

    this.flags.push(flg);
    return flg;
  }

  function parse(array args) {
    var self = this;
    var first_arg = args.erase_get(0);

    var result = zs::create_object({
      _error = null,

      has_error = function() {
        return typeof(this._error) != "null";
      }

      print_error = function() {
        io::print(this._error);
      }

      print_help = function() {
        self.print_help();
      }
    }, true);

    if(!_process_flags(args, result)) {
      return result;
    }

    if(!_process_options(args, result)) {
      return result;
    }

    if(!_process_arguments(args, result)) {
      return result;
    }

    // io::print("unprocessed_args", args);

    return result;
  }

  private function _process_options(args, result) {
    const opts = this.options;
    const opts_sz = opts.size();

    for(int k = 0; k < opts_sz; k++) {
      const opt = opts[k];

      for(int i = 0; i < args.size();) {
        if(_is_option_or_flag_arg(opt, args[i])) {
          int n = _process_option(opt, i, args, result);

          if(n == 0) {
            return false;
          }

          if(n == 1) {
            args.erase(i);
          }
          else if(n == 2) {
            args.erase_indices(i, i + 1);
          }
        }
        else {
          ++i;
        }
      }

      // Validate.
      if(opt.required && !result.contains(opt.name)) {
        result._error = "Missing required field '" + opt.name + "'.";
        return false;
      }
    }

    return true;
  }

  private function _process_flags(args, result) {
    const flgs = this.flags;
    const flgs_sz = flgs.size();

    for(int k = 0; k < flgs_sz; k++) {
      const flg = flgs[k];

      for(int i = 0; i < args.size();) {
        if(_is_option_or_flag_arg(flg, args[i])) {
          result[flg.name] = true;
          args.erase(i);
        }
        else {
          ++i;
        }
      }
    }

    return true;
  }

  private function _process_arguments(args, result) {
    const p_args = this.arguments;
    const int p_args_sz = p_args.size();

    for(int k = 0; k < p_args_sz; k++) {
      const arg = p_args[k];

      int count = 0;
      for(int i = 0; i < args.size();) {
        if(!_add_value_to_array(arg.name, args.erase_get(i), result, arg.count)) {
          return false;
        }

        if(arg.count == ++count) {
          break;
        }
      }

      if(result.contains(arg.name)) {
        if(!arg.count) {
          continue;
        }

        const arr = result[arg.name];
        int arr_sz = 1;

        if(zs::is_array(arr)) {
          arr_sz = arr.size();
        }

        if(arg.count != arr_sz) {
          result._error = "Invalid number of parameter (" + zs::to_string(arr_sz) + ") for required field '" + arg.name + "' ("+ zs::to_string(arg.count)+").";
          return false;
        }
      }
      else if(arg.required) {
        result._error = "Missing required field '" + arg.name + "'.";
        return false;
      }
    }

    return true;
  }

  static private function _is_option_or_flag_arg(var obj, string arg) {
    for(int k = 0; k < obj.cmd.size(); k++) {
      if(arg.starts_with(obj.cmd[k])) {
        return true;
      }
    }

    return false;
  }

  static private function _add_option_to_result(opt, result, value) {
    int count = 1;

    if(opt.multiple) {
      count = 0;
    }

    return _add_value_to_array(opt.name, value, result, count);
  }

  static private function _add_value_to_array(name, value, result, count) {
    if(!zs::contains(result, name)) {
      result[name] = value;
      return true;
    }

    var res = result[name];
    if(zs::is_array(res)){
      res.push(value);
    }
    else {
      result[name] = [res, value];
    }

    if(count and result[name].size() > count) {
      result._error = "Duplicated value for '" + name + "'.";
      return false;
    }

    return true;
  }

  static private function _process_option(opt, idx, args, result) {
    for(int k = 0; k < opt.cmd.size(); k++) {
      const eq_cmd = opt.cmd[k] + "=";
      if(args[idx].starts_with(eq_cmd)) {
        if(!_add_option_to_result(opt, result, args[idx].replace_first( eq_cmd, ""))) {
          return 0;
        }

        return 1;
      }
    }

    if(idx + 1 >= args.size()) {
      result._error = "Missing value for '" + opt.name + "'.";
      return 0;
    }

    if(!_add_option_to_result(opt, result, args[idx + 1])) {
      return 0;
    }

    return 2;
  }
};

return argparser;