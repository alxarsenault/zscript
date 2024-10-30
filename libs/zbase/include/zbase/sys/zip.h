// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zbase/zbase.h>
#include <zbase/sys/error_code.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include <span>

struct mz_zip_archive_tag;
typedef mz_zip_archive_tag mz_zip_archive;

ZBASE_BEGIN_NAMESPACE

/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/

struct zip_info {
  std::string filename;

  struct {
    int year = 1980;
    int month = 0;
    int day = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
  } date_time;

  std::string comment;
  std::string extra;
  uint16_t create_system = 0;
  uint16_t create_version = 0;
  uint16_t extract_version = 0;
  uint16_t flag_bits = 0;
  size_t volume = 0;
  uint32_t internal_attr = 0;
  uint32_t external_attr = 0;
  size_t header_offset = 0;
  uint32_t crc = 0;
  size_t compress_size = 0;
  size_t file_size = 0;
};

class zip_file {
public:
  zip_file();
  zip_file(const std::string& filename);
  zip_file(std::istream& stream);
  zip_file(const std::vector<uint8_t>& bytes);

  ~zip_file();

  void load(std::istream& stream);
  void load(const std::string& filename);
  void load(std::span<const uint8_t> bytes);

  void save(const std::string& filename);
  void save(std::ostream& stream);
  void save(std::vector<uint8_t>& bytes);

  void reset();

  bool has_file(const std::string& name) const;
  bool has_file(const zip_info& name) const;

  zip_info getinfo(const std::string& name) const;

  std::vector<zip_info> infolist() const;

  std::vector<std::string> namelist() const;

  std::ostream& open(const std::string& name);
  std::ostream& open(const zip_info& name);

  void extract(const std::string& member, const std::string& path);
  void extract(const zip_info& member, const std::string& path);

  void extractall(const std::string& path);
  void extractall(const std::string& path, const std::vector<std::string>& members);
  void extractall(const std::string& path, const std::vector<zip_info>& members);

  void printdir() const;
  void printdir(std::ostream& stream) const;

  std::string read(const zip_info& info) const;
  std::vector<uint8_t> read_buffer(const zip_info& info) const;
  std::string read(const std::string& name) const;

  std::pair<bool, std::string> testzip();

  void write(const std::string& filename);
  void write(const std::string& filename, const std::string& arcname);

  void writestr(const std::string& arcname, const std::string& bytes);
  void writestr(const zip_info& info, const std::string& bytes);

  const std::string& get_filename() const { return _filename; }

  std::string comment;

private:
  void start_read() const;

  void start_write();

  void append_comment();

  void remove_comment();

  zip_info getinfo(int index) const;

  std::unique_ptr<mz_zip_archive> _archive;
  std::vector<char> _buffer;
  std::stringstream _open_stream;
  std::string _filename;
};

ZBASE_END_NAMESPACE
