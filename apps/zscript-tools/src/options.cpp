#include "options.h"
#include <zbase/utility/print.h>
#include <argparse.h>

options parse_opts(int argc, char* argv[]) {
  options args;
  argparse::ArgumentParser program("test");

  // File path.
  program
      .add_argument("input") //
      .nargs(1) //
      .help("Thing to use.");

  program
      .add_argument("args") //
      .remaining();

  // Main function name.
  program
      .add_argument("-m", "--main") //
      .default_value(std::string{ "main" })
      .nargs(1) //
      .help("increase output verbosity");

  // Main function name.
  program
      .add_argument("-I", "--include") //
      .append() //
      .help("include directory path");

  program
      .add_argument("-D") //
      .nargs(argparse::nargs_pattern::optional)
      .append() //
      .metavar("KEY=VALUE"); //

  //  parser.add_argument("--set",
  //                          metavar="KEY=VALUE",
  //                          nargs='+',
  //                          help="Set a number of key-value pairs "
  //                               "(do not put spaces before or after the = sign). "
  //                               "If a value contains spaces, you should define "
  //                               "it with double quotes: "
  //                               'foo="this is a sentence". Note that '
  //                               "values are always treated as strings.")

  //  .nargs(argparse::nargs_pattern::any);

  //  program.add_argument("-c")//
  //        .nargs(2)//
  //        .default_value(std::vector<std::string>{"A", "B"});

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  try { 
    args.filepath = std::filesystem::weakly_canonical(program.get("input"));
    
    args.main_function = program.get("--main");
    args.include_directories = program.get<std::vector<std::string>>("--include");

    if (auto main_args = program.present<std::vector<std::string>>("args")) {
      args.args = main_args.value();
    }

    if (auto defines = program.present<std::vector<std::string>>("-D")) {
      std::vector<std::string> defs = defines.value();

      for (const auto& d : defs) {
        size_t eqpos = d.find('=');
        args.defines.emplace_back(d.substr(0, eqpos), d.substr(eqpos + 1));
      }
    }

    //    zb::print(args.filepath, args.main_function, args.args, args.include_directories, args.defines);
  }

  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  //  std::cout << program << std::endl;

  return args;
}
