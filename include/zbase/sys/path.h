//
// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <zbase/zbase.h>
#include <zbase/sys/error_code.h>
#include <zbase/strings/string_view.h>
#include <filesystem>
#include <string>

ZBASE_BEGIN_SUB_NAMESPACE(sys)

enum class file_type : int8_t {
  not_found = -1,
  none = 0,
  regular = 1,
  directory = 2,
  symlink = 3,
  block = 4,
  character = 5,
  fifo = 6,
  socket = 7,
  unknown = 8
};

template <class _Allocator = std::allocator<char>>
class path;

class path_ref;

struct path_detail {
  template <class>
  friend class path;
  friend class path_ref;

  static bool is_root(__zb::string_view path);
  static bool is_directory(const char* path);
  static bool is_file(const char* path);
  static bool is_symlink(const char* path);
  static bool is_absolute(const char* path);
  static bool exists(const char* path);
  static bool has_filename(__zb::string_view path);
  static bool has_stem(__zb::string_view path);
  static bool has_extension(__zb::string_view path);

  static __zb::string_view get_stem(__zb::string_view path);
  static __zb::string_view get_filename(__zb::string_view path);
  static __zb::string_view get_extension(__zb::string_view path);
  static __zb::string_view get_root(__zb::string_view path);
  static __zb::string_view get_dirname(__zb::string_view path);

  static __zb::error_result touch(const char* path);
  static __zb::error_result mkdir(const char* path);
  static __zb::error_result mktree(const char* path);
  static __zb::error_result unlink(const char* path);
  static __zb::error_result rmdir(const char* path);

  static __zb::error_result rename(const char* path, const char* new_path);

  static size_t file_size(const char* path);
  static int64_t path_conf(const char* path);

  static const char* getcwd(char* buf, size_t size);
};

template <class _Allocator>
class path {
public:
  using value_type = char;
  using allocator_type = _Allocator;
  using string_type = std::basic_string<char, std::char_traits<char>, _Allocator>;

  inline path() noexcept(std::is_nothrow_default_constructible<allocator_type>::value) = default;

  inline path(const allocator_type& a) noexcept(std::is_nothrow_copy_constructible<allocator_type>::value)
      : _path((a)) {}

  inline path(const path&) = default;
  inline path(path&&) = default;

  inline path(const path& p, const allocator_type& a)
      : _path(p, a) {}

  inline path(path&& p, const allocator_type& a)
      : _path(std::move(p), a) {}

  inline path(const char* p)
      : _path(p) {}

  inline path(const char* p, const allocator_type& a)
      : _path(p, a) {}

  inline path(std::string_view p)
      : _path(p) {}

  inline path(std::string_view p, const allocator_type& a)
      : _path(p, a) {}

  inline path(const string_type& p)
      : _path(p, p.get_allocator()) {}

  inline path(string_type&& p)
      : _path(std::move(p)) {}

  inline path(const string_type& p, const allocator_type& a)
      : _path(p, a) {}

  inline path& operator=(const path&) = default;
  inline path& operator=(path&&) = default;

  static path pwd(const allocator_type& a) {
    string_type p((a));

    uint64_t sz = path_detail::path_conf(".");
    p.resize(sz, 0);
    const char* pp = path_detail::getcwd(p.data(), sz + 1);

    if (pp) {
      p.resize(std::string_view(pp).size());
    }
    else {
      p.clear();
    }
    return path(std::move(p));
  }

  static path pwd() {
    string_type p;

    uint64_t sz = path_detail::path_conf(".");
    p.resize(sz, 0);
    const char* pp = path_detail::getcwd(p.data(), sz + 1);
    if (!pp) {
      return path();
    }
    return path(std::move(p));
  }

  /// @note This method acceses the filesystem.
  ///
  /// Checks if this is a directory.
  /// Returns false if the referenced path does not exist rather than erroring out.
  inline bool is_directory() const;

  /// Checks if this path is a filesystem root. On UNIX, this
  /// is the case if the path consists solely of one slash, on
  /// Windows this is the case if the path looks like this:
  /// "<letter>:/".
  inline bool is_root() const;

  /// @note This method accesses the filesystem.
  ///
  /// Checks if this is a file. Returns false
  /// if the referenced path does not exist rather
  /// than erroring out.
  inline bool is_file() const;

  /// @note This method acceses the filesystem.
  ///
  /// Checks if this file is a symbolic link; also
  /// works with NTFS symlinks on Windows. Returns false
  /// rather than erroring out if the referenced path does
  /// not exist.
  inline bool is_symlink() const;

  /// Checks if this is an absolute path, i.e. one that
  /// starts with a / on all systems or with X:/
  /// only on Windows, where `X` is a drive letter.
  ///
  /// Note that / on Windows is the root of the current drive
  /// and hence also an absolute path.
  inline bool is_absolute() const;

  /// @note This method acceses the filesystem.
  ///
  /// Checks if the file exists. Note that if you don’t have
  /// sufficient rights for the check on the given path, this
  /// method will throw an exception.
  inline bool exists() const;

  inline explicit operator bool() const { return exists(); }

  inline bool has_filename() const;
  inline bool has_stem() const;
  inline bool has_extension() const;

  /// Returns the path’s filename, i.e. the last component
  /// of the path, including the file excention.
  ///
  /// For example, "/foo/bar.txt" has a basename of "bar.txt",
  /// and "/foo/bar" has a filename of "bar".
  ///
  /// @returns a new Path instance with only the filename.
  ///
  /// @see dirname()
  inline __zb::string_view filename() const;

  inline __zb::string_view stem() const;

  /// This method returns the file extension of the path,
  /// if possible; otherwise it returns an empty string.
  /// Filenames that consist entirely of a "file extension",
  /// i.e. ".txt" or "/foo/.txt" will return an empty string.
  inline __zb::string_view extension() const;

  inline path& replace_extension(std::string_view ext);

  inline path with_extension(std::string_view ext) const;
  /// Returns the path’s dirname, i.e. all components of the
  /// path except for the basename component (see basename()).
  ///
  /// For example, "/foo/bar/baz.txt" has a dirname of "/foo/bar",
  /// and "/foo/bar/baz" has a dirname of "/foo/bar".
  ///
  /// @returns a new Path instance with only the dirname.
  ///
  /// @see basename() parent()
  inline __zb::string_view dirname() const;

  /// Returns the filesystem root for this path. On UNIX,
  /// this will always return /, but on Windows it will
  /// return X:/ if the referenced path is an absolute path
  /// with drive letter, and / if the referenced path is
  /// a relative path or an absolute path on the current
  /// drive.
  inline __zb::string_view root() const;

  /// @note This method writes to the filesystem.
  ///
  /// Sets the file’s modification and access times to the
  /// current time. If the file does not yet exist, it is created.
  ///
  /// This is akin to the UNIX `touch` command.
  inline __zb::error_result touch() const;

  /**
   * \note This method writes to the filesystem.
   *
   * Creates the referenced directory non-recursively,
   * i.e. parent directories are not created. Trying
   * to create a directory below a nonexistant directory
   * will result in an ErrnoError exception.
   *
   * \remark UNIX note: The directory is created with RWX permissions
   * for everyone, but filtered by your current `umask` before applied
   * to disk.
   *
   * \see mktree()
   */
  inline __zb::error_result mkdir() const;

  /**
   * \note This method writes to the file system.
   *
   * This method provides a functionality akin to the UNIX `mkdir -p`
   * command, i.e. it creates the referenced directory, and if necessary,
   * also creates all parent directories. Note this method does not
   * throw an ErrnoError if the referenced directory already exists;
   * it just does nothing.
   *
   * \see mkdir()
   */
  inline __zb::error_result mktree() const;

  /**
   * \note This method writes to the filesystem.
   *
   * Deletes the referenced file. This cannot be used to
   * delete a directory rather than a file.
   *
   * \see remove() rmdir()
   */
  inline __zb::error_result unlink() const;

  /**
   * \note This method writes to the filesystem.
   *
   * Deletes the referenced directory, which is required
   * to be empty, if not, an ErrnoError will be thrown.
   *
   * This cannot be used to delete a file rather than a
   * directory.
   *
   * \see remove() unlink()
   */
  inline __zb::error_result rmdir() const;

  /**
   * \note This method writes to the file system.
   *
   * Renames a file to another name without involving file streams.
   *
   * \param[in] newname The new name of the file.
   */
  inline __zb::error_result rename(const char* new_path) const;

  /**
   * \note This method accesses the file system.
   *
   * Returns the file size.
   */
  inline size_t file_size() const;

  inline path concat(std::string_view p) const { return path(_path + p, _path.get_allocator()); }

  inline path join(const path& p) const { return path(*this).joined(p); }

  inline path& joined(const path& p) {
    _path.append("/");
    _path.append(p.string());
    return *this;
  }

  inline path operator+(std::string_view s) const {
    path p(*this);
    p._path += s;
    return p;
  }

  inline const string_type& string() const noexcept { return _path; }

private:
  string_type _path;
};

template <class _Allocator>
bool path<_Allocator>::is_directory() const {
  return path_detail::is_directory(_path.c_str());
}

template <class _Allocator>
bool path<_Allocator>::is_root() const {
  return path_detail::is_root(_path);
}

template <class _Allocator>
bool path<_Allocator>::is_file() const {
  return path_detail::is_file(_path.c_str());
}

template <class _Allocator>
bool path<_Allocator>::is_symlink() const {
  return path_detail::is_symlink(_path.c_str());
}

template <class _Allocator>
bool path<_Allocator>::is_absolute() const {
  return path_detail::is_absolute(_path.c_str());
}

template <class _Allocator>
bool path<_Allocator>::exists() const {
  return path_detail::exists(_path.c_str());
}

template <class _Allocator>
bool path<_Allocator>::has_filename() const {
  return path_detail::has_filename(_path);
}

template <class _Allocator>
bool path<_Allocator>::has_stem() const {
  return path_detail::has_stem(_path);
}

template <class _Allocator>
bool path<_Allocator>::has_extension() const {
  return path_detail::has_extension(_path);
}

template <class _Allocator>
__zb::string_view path<_Allocator>::filename() const {
  return path_detail::get_filename(_path);
}

template <class _Allocator>
__zb::string_view path<_Allocator>::stem() const {
  return path_detail::get_stem(_path);
}

template <class _Allocator>
__zb::string_view path<_Allocator>::extension() const {
  return path_detail::get_extension(_path);
}

template <class _Allocator>
inline path<_Allocator>& path<_Allocator>::replace_extension(std::string_view new_ext) {
  __zb::string_view ext = extension();
  if (!ext.empty()) {
    _path.erase(_path.end() - ext.size(), _path.end());
  }

  _path.append(new_ext);
  return *this;
}

template <class _Allocator>
inline path<_Allocator> path<_Allocator>::with_extension(std::string_view ext) const {
  path p(*this);
  return p.replace_extension(ext);
}

template <class _Allocator>
__zb::string_view path<_Allocator>::dirname() const {
  return path_detail::get_dirname(_path);
}

template <class _Allocator>
__zb::string_view path<_Allocator>::root() const {
  return path_detail::get_root(_path);
}

template <class _Allocator>
__zb::error_result path<_Allocator>::touch() const {
  return path_detail::touch(_path.c_str());
}

template <class _Allocator>
__zb::error_result path<_Allocator>::mkdir() const {
  return path_detail::mkdir(_path.c_str());
}

template <class _Allocator>
__zb::error_result path<_Allocator>::mktree() const {
  return path_detail::mktree(_path.c_str());
}

template <class _Allocator>
__zb::error_result path<_Allocator>::unlink() const {
  return path_detail::unlink(_path.c_str());
}

template <class _Allocator>
__zb::error_result path<_Allocator>::rmdir() const {
  return path_detail::rmdir(_path.c_str());
}

template <class _Allocator>
__zb::error_result path<_Allocator>::rename(const char* new_path) const {
  return path_detail::rename(_path.c_str(), new_path);
}

template <class _Allocator>
size_t path<_Allocator>::file_size() const {
  return path_detail::file_size(_path.c_str());
}

class path_ref {
public:
  using value_type = char;

  inline path_ref() noexcept = delete;

  inline path_ref(const path_ref&) = default;
  inline path_ref(path_ref&&) = default;

  inline path_ref(const char* p)
      : _path(p) {}

  template <class _Allocator>
  inline path_ref(const std::basic_string<char, std::char_traits<char>, _Allocator>& p)
      : _path(p.c_str()) {}

  template <class _StdPath = std::filesystem::path>
    requires std::is_same_v<std::filesystem::path, _StdPath>
      and std::is_same_v<std::filesystem::path::value_type, char>
  inline path_ref(const _StdPath& p)
      : _path(p.c_str()) {}

  inline path_ref& operator=(const path_ref&) = default;
  inline path_ref& operator=(path_ref&&) = default;

  /// @note This method acceses the filesystem.
  ///
  /// Checks if this is a directory.
  /// Returns false if the referenced path does not exist rather than erroring out.
  inline bool is_directory() const;

  /// Checks if this path is a filesystem root. On UNIX, this
  /// is the case if the path consists solely of one slash, on
  /// Windows this is the case if the path looks like this:
  /// "<letter>:/".
  inline bool is_root() const;

  /// @note This method accesses the filesystem.
  ///
  /// Checks if this is a file. Returns false
  /// if the referenced path does not exist rather
  /// than erroring out.
  inline bool is_file() const;

  /// @note This method acceses the filesystem.
  ///
  /// Checks if this file is a symbolic link; also
  /// works with NTFS symlinks on Windows. Returns false
  /// rather than erroring out if the referenced path does
  /// not exist.
  inline bool is_symlink() const;

  /// Checks if this is an absolute path, i.e. one that
  /// starts with a / on all systems or with X:/
  /// only on Windows, where `X` is a drive letter.
  ///
  /// Note that / on Windows is the root of the current drive
  /// and hence also an absolute path.
  inline bool is_absolute() const;

  /// @note This method acceses the filesystem.
  ///
  /// Checks if the file exists. Note that if you don’t have
  /// sufficient rights for the check on the given path, this
  /// method will throw an exception.
  inline bool exists() const;

  inline bool has_filename() const;
  inline bool has_stem() const;
  inline bool has_extension() const;

  /// Returns the path’s filename, i.e. the last component
  /// of the path, including the file excention.
  ///
  /// For example, "/foo/bar.txt" has a basename of "bar.txt",
  /// and "/foo/bar" has a filename of "bar".
  ///
  /// @returns a new Path instance with only the filename.
  ///
  /// @see dirname()
  inline __zb::string_view filename() const;

  inline __zb::string_view stem() const;

  /// This method returns the file extension of the path,
  /// if possible; otherwise it returns an empty string.
  /// Filenames that consist entirely of a "file extension",
  /// i.e. ".txt" or "/foo/.txt" will return an empty string.
  inline __zb::string_view extension() const;

  /// Returns the path’s dirname, i.e. all components of the
  /// path except for the basename component (see basename()).
  ///
  /// For example, "/foo/bar/baz.txt" has a dirname of "/foo/bar",
  /// and "/foo/bar/baz" has a dirname of "/foo/bar".
  ///
  /// @returns a new Path instance with only the dirname.
  ///
  /// @see basename() parent()
  inline __zb::string_view dirname() const;

  /// Returns the filesystem root for this path. On UNIX,
  /// this will always return /, but on Windows it will
  /// return X:/ if the referenced path is an absolute path
  /// with drive letter, and / if the referenced path is
  /// a relative path or an absolute path on the current
  /// drive.
  inline __zb::string_view root() const;

  /// @note This method writes to the filesystem.
  ///
  /// Sets the file’s modification and access times to the
  /// current time. If the file does not yet exist, it is created.
  ///
  /// This is akin to the UNIX `touch` command.
  inline __zb::error_result touch() const;

  /**
   * \note This method writes to the filesystem.
   *
   * Creates the referenced directory non-recursively,
   * i.e. parent directories are not created. Trying
   * to create a directory below a nonexistant directory
   * will result in an ErrnoError exception.
   *
   * \remark UNIX note: The directory is created with RWX permissions
   * for everyone, but filtered by your current `umask` before applied
   * to disk.
   *
   * \see mktree()
   */
  inline __zb::error_result mkdir() const;

  /**
   * \note This method writes to the file system.
   *
   * This method provides a functionality akin to the UNIX `mkdir -p`
   * command, i.e. it creates the referenced directory, and if necessary,
   * also creates all parent directories. Note this method does not
   * throw an ErrnoError if the referenced directory already exists;
   * it just does nothing.
   *
   * \see mkdir()
   */
  inline __zb::error_result mktree() const;

  /**
   * \note This method writes to the filesystem.
   *
   * Deletes the referenced file. This cannot be used to
   * delete a directory rather than a file.
   *
   * \see remove() rmdir()
   */
  inline __zb::error_result unlink() const;

  /**
   * \note This method writes to the filesystem.
   *
   * Deletes the referenced directory, which is required
   * to be empty, if not, an ErrnoError will be thrown.
   *
   * This cannot be used to delete a file rather than a
   * directory.
   *
   * \see remove() unlink()
   */
  inline __zb::error_result rmdir() const;

  /**
   * \note This method writes to the file system.
   *
   * Renames a file to another name without involving file streams.
   *
   * \param[in] newname The new name of the file.
   */
  inline __zb::error_result rename(const char* new_path) const;

  /**
   * \note This method accesses the file system.
   *
   * Returns the file size.
   */
  inline size_t file_size() const;

  const char* _path;
};

bool path_ref::is_directory() const { return path_detail::is_directory(_path); }

bool path_ref::is_root() const { return path_detail::is_root(_path); }

bool path_ref::is_file() const { return path_detail::is_file(_path); }

bool path_ref::is_symlink() const { return path_detail::is_symlink(_path); }

bool path_ref::is_absolute() const { return path_detail::is_absolute(_path); }

bool path_ref::exists() const { return path_detail::exists(_path); }

bool path_ref::has_filename() const { return path_detail::has_filename(_path); }

bool path_ref::has_stem() const { return path_detail::has_stem(_path); }

bool path_ref::has_extension() const { return path_detail::has_extension(_path); }

__zb::string_view path_ref::filename() const { return path_detail::get_filename(_path); }

__zb::string_view path_ref::stem() const { return path_detail::get_stem(_path); }

__zb::string_view path_ref::extension() const { return path_detail::get_extension(_path); }

__zb::string_view path_ref::dirname() const { return path_detail::get_dirname(_path); }

__zb::string_view path_ref::root() const { return path_detail::get_root(_path); }

__zb::error_result path_ref::touch() const { return path_detail::touch(_path); }

__zb::error_result path_ref::mkdir() const { return path_detail::mkdir(_path); }

__zb::error_result path_ref::mktree() const { return path_detail::mktree(_path); }

__zb::error_result path_ref::unlink() const { return path_detail::unlink(_path); }

__zb::error_result path_ref::rmdir() const { return path_detail::rmdir(_path); }

__zb::error_result path_ref::rename(const char* new_path) const {
  return path_detail::rename(_path, new_path);
}

size_t path_ref::file_size() const { return path_detail::file_size(_path); }

ZBASE_END_SUB_NAMESPACE(sys)
