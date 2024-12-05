#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct options {
  std::filesystem::path filepath;
  std::string main_function;
  std::vector<std::string> args;
  std::vector<std::string> include_directories;
  std::vector<std::pair<std::string, std::string>> defines;
};

options parse_opts(int argc, char* argv[]);
