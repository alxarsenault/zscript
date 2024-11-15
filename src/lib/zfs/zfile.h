#pragma once

#include "zinclude_guard.h"

namespace zs::file_library {

inline constexpr std::string_view k_file_uid = "fs.file";
inline constexpr std::string_view k_file_delegate_name = "__fs_file_delegate";

//"r"  read  Open a file for reading  read from start  return NULL and set error
//"w"  write  Create a file for writing  destroy contents  create new
//"a"  append  Append to a file  write to end  create new
//"r+"  read extended  Open a file for read/write  read from start  return NULL
// and set error "w+"  write extended  Create a file for read/write  destroy
// contents  create new "a+"  append extended  Open a file for read/write  write
// to end  create new

// https://en.cppreference.com/w/cpp/io/basic_filebuf/open
// https://en.cppreference.com/w/cpp/io/c/fopen
// https://en.cppreference.com/w/cpp/io/ios_base/openmode

// Seek to the end of stream before each write.
inline constexpr int_t open_mode_append = 1;

/// Seek to the end of stream immediately after open.
inline constexpr int_t open_mode_end = 2;

/// Open in binary mode.
inline constexpr int_t open_mode_binary = 4;

// Open for reading.
inline constexpr int_t open_mode_read = 8;

// Open for writing.
inline constexpr int_t open_mode_write = 16;

// Discard the contents of the stream when opening.
inline constexpr int_t open_mode_trunc = 32;
inline constexpr int_t open_mode_create_if_not_found = 64;

struct zfile {
  zfile(zs::engine* eng, std::string_view path, int_t openmode = open_mode_read | open_mode_write);

  zs::string path;
  std::fstream stream;
};

zfile* get_file(zs::vm_ref vm);

zs::object create_file(zs::vm_ref vm, std::string_view path, int_t openmode);

} // namespace zs::file_library.
