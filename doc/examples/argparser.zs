@module argparser

```
Flag
```
struct flag {
  string name;
  array cmd;
  string description;

  constructor(string name, var<string, array> cmd, string description = "") {
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

```
Option
```
struct option {
  string name;
  array cmd;
  string description;
  bool required;
  bool multiple;

  constructor(string name, var<string, array> cmd,
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

```
Argument parser
```
struct argparser {
  static var option = option;
  static var flag = flag;
  string name;
  string description;
  var options = [];
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
    zs::print("HELP");
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

    //opt.parser = zs::toweak(this);
    this.options.push(opt);
    return opt;
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

    //flg.parser = zs::toweak(this);
    this.flags.push(flg);
    return flg;
  }

  function parse(array args) {
    var self = this;

    var result = zs::set_metadata({}, {
      _error = null,

      has_error = function() {
        return typeof(this._error) != "null";
      }

      print_error = function() {
        zs::print(this._error);
      }

      print_help = function() {
        self.print_help();
      }
    });

    var processed_args = [];

    for(int i = 1; i < args.size(); i++) {
      for(int k = 0; k < this.options.size(); k++) {
        var opt = this.options[k];
        if(_is_option_arg(opt, args[i])) {
          processed_args.push(i);

          int last_i = i;
          i = _process_option(opt, i, args, result);

          if(i != last_i) {
            processed_args.push(i);
          }
          break;
        }
      }
    }

    args.erase_indices(processed_args);
    processed_args.clear();

    for(int i = 1; i < args.size(); i++) {
      for(int k = 0; k < this.flags.size(); k++) {
        var flg = this.flags[k];
        if(_is_flag_arg(flg, args[i])) {
          processed_args.push(i);
          int last_i = i;
          i = _process_flag(flg, i, args, result);

          if(i != last_i) {
            processed_args.push(i);
          }
          break;
        }
      }
    }

    args.erase_indices(processed_args);
    processed_args.clear();

    if(result.has_error()) {
      return result;
    }

    for(int k = 0; k < this.options.size(); k++) {
      const opt = this.options[k];
      if(opt.required && !result.contains(opt.name)) {
        result._error = "Missing required field '" + opt.name + "'.";
        return result;
      }
    }

    // print("unprocessed_args", args);

    return result;
  }

  static private function _is_option_arg(var opt, string arg) {
    for(int k = 0; k < opt.cmd.size(); k++) {
      if(arg.starts_with(opt.cmd[k])) {
        return true;
      }
    }

    return false;
  }

  static private function _is_flag_arg(var flg, string arg) {
    for(int k = 0; k < flg.cmd.size(); k++) {
      if(arg.starts_with(flg.cmd[k])) {
        return true;
      }
    }

    return false;
  }

  static private function _add_flag_to_result(flg, result) {
    result[flg.name] = true;
    return true;
  }

  static private function _add_option_to_result(opt, result, value) {
    if(!result.contains(opt.name)) {
      result[opt.name] = value;
      return true;
    }

    if(!opt.multiple) {
      result._error = "Duplicated value for '" + opt.name + "'.";
      return false;
    }

    if(typeof(result[opt.name]) == "array"){
      result[opt.name].push(value);
      return true;
    }

    result[opt.name] = [result[opt.name], value];
    return true;
  }

  static private function _process_flag(flg, idx, args, result) {
    for(int k = 0; k < flg.cmd.size(); k++) {
      const eq_cmd = flg.cmd[k];
      if(args[idx].starts_with(eq_cmd)) {
        if(!_add_flag_to_result(flg, result)) {
          return args.size();
        }

        return idx;
      }
    }

    return idx + 1;
  }

  static private function _process_option(opt, idx, args, result) {
    for(int k = 0; k < opt.cmd.size(); k++) {
      const eq_cmd = opt.cmd[k] + "=";
      if(args[idx].starts_with(eq_cmd)) {
        if(!_add_option_to_result(opt, result, args[idx].replace_first( eq_cmd, ""))) {
          return args.size();
        }

        return idx;
      }
    }

    if(idx + 1 >= args.size()) {
      result._error = "Missing value for '" + opt.name + "'.";
      return args.size();
    }

    if(_get_arg_option_command_index(args[idx + 1]) != -1) {
      result._error = "Invalid value for '" + opt.name + "'.";
      return args.size();
    }

    if(!_add_option_to_result(opt, result, args[idx + 1])) {
      return args.size();
    }

    return idx + 1;
  }

  private function _get_arg_option_command_index(arg) {
    for(int k = 0; k < this.options.size(); k++) {
      var opt = this.options[k];
      if(_is_option_arg(opt, arg)) {
        return k;
      }
    }

    return -1;
  }
};

__exports__ = argparser;