#include "lib/zfs/zpath.h"
#include <zbase/sys/path.h>
#include "zvirtual_machine.h"

#include <stdlib.h>

#if !defined(ZS_UNIX) \
    && (defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(BSD))
#define ZS_UNIX 1
#else
#deine ZS_UNIX 0
#endif

#if ZS_UNIX
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h> // defines "BSD" macro on BSD systems
#include <pwd.h>
#include <glob.h>
#include <fnmatch.h>
#endif

#ifdef BSD
#define ZS_BSD 1
#include <sys/time.h>
#include <sys/sysctl.h>
#else
#define ZS_BSD 0
#endif

#ifdef __APPLE__
#define ZS_APPLE 1
#include <mach-o/dyld.h>
#else
#define ZS_APPLE 0
#endif

namespace zs::path_library {

static std::string_view k_path_method_list_recursive = "list_recursive";
static std::string_view k_path_method_rmdir_recursive = "rmdir_recursive";
static std::string_view k_path_method_component_count = "component_count";
static std::string_view k_path_method_is_refular_file = "is_refular_file";

zs::string* get_path(zs::vm_ref vm) {
  const zs::object& path_obj = vm[0];
  if (!path_obj.is_user_data() or path_obj._udata->get_uid() != k_path_uid) {
    return nullptr;
  }

  return path_obj._udata->data<zs::string>();
}

zs::string* get_path(zs::vm_ref vm, int_t idx) {
  const zs::object& path_obj = vm[idx];
  if (!path_obj.is_user_data() or path_obj._udata->get_uid() != k_path_uid) {
    return nullptr;
  }

  return path_obj._udata->data<zs::string>();
}

inline static constexpr const char* file_type_to_string(std::filesystem::file_type ft) noexcept {
  switch (ft) {
  case std::filesystem::file_type::none:
    return "unknown";
  case std::filesystem::file_type::not_found:
    return "not_found";
  case std::filesystem::file_type::regular:
    return "regular";
  case std::filesystem::file_type::directory:
    return "directory";
  case std::filesystem::file_type::symlink:
    return "symlink";
  case std::filesystem::file_type::block:
    return "block";
  case std::filesystem::file_type::character:
    return "character";
  case std::filesystem::file_type::fifo:
    return "fifo";
  case std::filesystem::file_type::socket:
    return "socket";
  case std::filesystem::file_type::unknown:
    return "unknown";
  }

  return "unknown";
}

// bool path_is_directory(const zs::string& path) {
//
// #if ZS_UNIX
//   struct stat s;
//   //  std::string nstr = native();
//
//   if (::stat(path.c_str(), &s) < 0) {
//     int errsav = errno;
//
//     // "Not found" means it isn’t a directory.
//     if (errsav == ENOENT)
//       return false;
//     else
//       return false;
//     //      throw(Pathie::ErrnoError(errsav));
//   }
//
//   if (S_ISDIR(s.st_mode)) {
//     return true;
//   }
//   else {
//     return false;
//   }
// #elif defined(_WIN32)
//   struct _stat s;
//   std::wstring utf16 = utf8_to_utf16(m_path);
//   if (_wstat(utf16.c_str(), &s) < 0) {
//     int errsav = errno;
//
//     if (errsav == ENOENT)
//       return false;
//     else
//       throw(Pathie::ErrnoError(errsav));
//   }
//
//   return s.st_mode & S_IFDIR;
// #else
// #error Unsupported system.
// #endif
// }

bool path_is_root(std::string_view path) {
#if ZS_UNIX
  return path.size() == 1 && path[0] == '/';
#elif defined(_WIN32)
  // / on Windows is root on current drive
  if (path.size() == 1 && path[0] == '/') {
    return true;
  }

  // X:/ is root including drive letter
  return path.size() == 3 && path[1] == ':';
#else
#error Unsupported platform.
#endif
}

bool path_is_file(const zs::string& path) {
#if ZS_UNIX
  struct stat s;

  if (::stat(path.c_str(), &s) < 0) {
    int errsav = errno;

    if (errsav == ENOENT)
      return false;
    else
      return false;
  }

  return S_ISREG(s.st_mode);

#elif defined(_WIN32)
  struct _stat s;
  std::wstring utf16 = utf8_to_utf16(m_path);
  if (_wstat(utf16.c_str(), &s) < 0) {
    int errsav = errno;

    if (errsav == ENOENT)
      return false;
    else
      throw(Pathie::ErrnoError(errno));
  }

  return s.st_mode & S_IFREG;
#else
#error Unsupported system.
#endif
}

bool path_is_absolute(const zs::string& path) {
#if ZS_UNIX
  return !path.empty() and path[0] == '/';
#elif defined(_WIN32)
  // / is root on current drive
  if (path[0] == '/') {
    return true;
  }

  // This is the only position where : is allowed on windows, and if it is
  // there, the path is absolute with a drive letter (X:/)
  return path[1] == ':';
#else
#error Unsupported system.
#endif
}

bool path_exists(const zs::string& path) {

#if ZS_UNIX

  if (::access(path.c_str(), F_OK) == -1) {
    int errsav = errno;
    if (errsav == ENOENT) {
      return false;
    }
    else {
      return false;
      //      throw(Pathie::ErrnoError(errsav));
    }
  }
  else
    return true;
#elif defined(_WIN32)
  std::wstring utf16 = utf8_to_utf16(m_path);
  if (_waccess(utf16.c_str(), F_OK) == -1) {
    int errsav = errno;
    if (errsav == ENOENT) {
      return false;
    }
    else {
      throw(Pathie::ErrnoError(errsav));
    }
  }
  else
    return true;
#else
#error Unsupported system.
#endif
}

std::string_view path_filename(zb::string_view path) {

  if (path == "." or path == ".." or path_is_root(path)) {
    return path;
  }

  if (size_t pos = path.rfind("/"); pos != std::string_view::npos) {
    return path.substr(pos + 1);
  }

  return path;
}

std::string_view path_stem(zb::string_view path) {

  if (path == "." or path == ".." or path_is_root(path)) {
    return path;
  }

  if (size_t pos = path.rfind("/"); pos != std::string_view::npos) {
    path = path.substr(pos + 1);
  }

  if (size_t pos = path.find("."); pos != std::string_view::npos) {
    return path.substr(0, pos);
  }

  return path;
}

bool path_has_filename(std::string_view path) {

  if (path == "." or path == ".." or path_is_root(path)) {
    return false;
  }

  if (size_t pos = path.rfind("/"); pos != std::string_view::npos) {
    return !path.substr(pos + 1).empty();
  }

  return !path.empty();
}

std::string_view path_root(std::string_view path) {
#if ZS_UNIX
  return "/";
#elif defined(_WIN32)
  // Check if we have an absolute path with drive,
  // otherwise return the root for the current drive.
  if (path[1] == ':') {
    // Colon is on Windows only allowed here to denote a preceeding drive letter
    // => absolute path
    return path.substr(0, 3);
  }
  else {
    return "/";
  }
#else
#error Unsupported system.
#endif
}

/**
 * Returns the path’s dirname, i.e. all components of the
 * path except for the basename component (see basename()).
 *
 * For example, "/foo/bar/baz.txt" has a dirname of "/foo/bar",
 * and "/foo/bar/baz" has a dirname of "/foo/bar".
 *
 * \returns a new Path instance with only the dirname.
 *
 * \see basename() parent()
 */
std::string_view path_dirname(std::string_view path) {

  if (path == ".") {
    return ".";
  }
  else if (path == "..") {
    return ".";
  }
  else if (path_is_root(path)) {
    return path;
  }

  if (size_t pos = path.rfind("/"); pos != std::string_view::npos) {
    if (pos == 0) {
      return path_root(path);
    }
#ifdef _WIN32
    else if (pos == 1 && path[1] == ':') { // X:/foo
      return path_root(path);
    }
#endif
    else {
      return path.substr(0, pos);
    }
  }
  else {
    return ".";
  }
}

/**
 * This method returns the file extension of the path,
 * if possible; otherwise it returns an empty string.
 * Filenames that consist entirely of a "file extension",
 * i.e. ".txt" or "/foo/.txt" will return an empty string.
 */
std::string_view path_extension(std::string_view path) {
  if (path.empty()) {
    return "";
  }

  if (path == "." || path == "..") {
    return "";
  }

  if (size_t pos = path.rfind("."); pos != std::string_view::npos) {
    // .foo and foo.
    if (pos == 0 || pos == path.size() - 1) {
      return "";
    }
    else {
      // foo/.txt
      if (path[pos - 1] == '/') {
        return "";
      }
      else {
        return path.substr(pos);
      }
    }
  }

  return "";
}

bool path_has_extension(std::string_view path) {
  if (path.empty()) {
    return false;
  }

  if (path == "." || path == "..") {
    return false;
  }

  if (size_t pos = path.rfind("."); pos != std::string_view::npos) {
    // .foo and foo.
    if (pos == 0 || pos == path.size() - 1) {
      return false;
    }
    else {
      // foo/.txt
      if (path[pos - 1] == '/') {
        return false;
      }
      else {
        return !path.substr(pos).empty();
      }
    }
  }

  return false;
}

zs::string path_prune(const zs::string& path) {
  zs::string newpath(path, path.get_allocator());
  size_t pos = 0;
  while ((pos = newpath.find("/.", pos)) != std::string::npos) {
    if (std::string_view(newpath).substr(pos, 3) == "/..") {

      // Weird path like /..foo or foo/..bar, which are NOT relative paths
      if (newpath.length() > pos + 3 && newpath[pos + 3] != '/') {
        // Do not reset `pos' -- this has to stay. Advance to the next char.
        pos++;
        continue;
      }

      if (pos == 0) {
        // /.. at beginning of string, replace with root / (/ on Windows is root
        // on current drive)
        newpath.erase(pos, 3);

        // Whoops -- the entire string was just "/.."
        if (newpath.empty()) {
          newpath.append("/");
        }
      }
#ifdef _WIN32
      // Cater for paths with drive X:/ on Windows
      else if (pos == 2 && newpath[1] == ':') { // ":" is on Windows only allowed at pos 1, where it
        // signifies the preceding char is a drive letter
        // X:/. or X:/.. at beginning of string
        if (newpath.length() > 4 && newpath[4] == '.') { // X:/..
          // Prevent special case "X:/..foo", which is directory "..foo" under
          // the root
          if (newpath.length() <= 5 || newpath[5] != '/') {
            // X:/.. or X:/../foo/bar at beginning of string, replace with drive
            // root
            newpath.erase(pos, 3);
          }
        }
        else { // X:/./foo/bar X:/..foo
          // Prevent special case "X:/.foo", which is directory ".foo" under the
          // root
          if (newpath.length() <= 4 || newpath[4] != '/') {
            // X:/. or X:/./foo/bar at beginning of string, replace with drive
            // root
            newpath.erase(pos, 2);
          }
        }

        if (newpath.length() == 2) {
          // Whoops -- the entire string was just "X:/.." or "X:/."
          newpath.append("/");
        }
      }
#endif
      else {
        size_t pos2 = 0;
        if ((pos2 = newpath.rfind("/", pos - 1)) != std::string::npos) { // assignment intended
          // Remove parent directory.
          newpath.erase(pos2, pos - pos2 + 3);
        }
        else { // ../ for relative path (as in foo/../baz.txt)
          newpath.erase(0, pos + 4);
        }
      }
    }
    else { // Single /.

      // Weird path like /..foo or foo/..bar, which are NOT relative paths
      if (newpath.length() > pos + 2 && newpath[pos + 2] != '/') {
        // Do not reset `pos' -- this has to stay. Advance to the next char.
        pos++;
        continue;
      }

      newpath.erase(pos, 2);

      // Whoops -- the entire string was just "/."
      if (newpath.empty()) {
        newpath.append("/");
      }
    }

    // Reset as we have modified the string and might need to go again over it
    pos = 0;
  }

  /* If we are empty now, the original string was a one-element
   * relative path with .. appended. We cannot know what to set
   * without referring to pwd(), which is external access and
   * forbidden for this method. So instead, we do the one sane thing
   * and just use ".". */
  if (newpath.empty()) {
    newpath = ".";
  }

  return newpath;
}

size_t path_component_count(zb::string_view path) {
  if (path_is_root(path)) {
    return 1;
  }

  size_t result = 0;
  size_t pos = 0;
  while ((pos = path.find("/", pos)) != zb::string_view::npos) {
    result++;
    pos++;
  }

  return ++result;
}

/**
 * \note Under specific circumstances (see below), this method
 * accesses the file system.
 *
 * This method creates an absolute path by use of prune(), but
 * additionally expands any expandable strings. If one of the
 * following substitution sequences are encountered, it will be
 * replaced accordingly.
 *
 * "~" is expanded to the user’s home directory, see home().
 *
 * \returns a new instance with everything expanded.
 *
 * \remark This method uses prune() to expand ".." entries, therefore
 * it will not consider symbolic links when resolving those. Use
 * real() if you need to do that.
 *
 * \see prune() real()
 */
// zs::string path_expand(const zs::string& path) {
//   zs::string newpath(path, path.get_allocator());
//
//   if (m_path[0] != '~')
//     newpath = newpath.absolute();
//
//   std::string str = path.str();
//   if (str[0] == '~') {
//     Path homepath = home();
//
//     if (str[1] == '/' || str.length() == 1) {
//       // User home requested
//       str.replace(0, 1, homepath.m_path);
//     }
//
//     path = Path(str);
//   }
//
//   return path.prune();
// }

zs::error_result path_mkdir(const zs::string& path) {
#if ZS_UNIX
  //  std::string nstr = native();

  if (::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
    return zs::error_code::invalid;
  }

  return {};
#elif defined(_WIN32)
  std::wstring utf16 = utf8_to_utf16(m_path);

  if (_wmkdir(utf16.c_str()) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif
}

zs::error_result path_mktree(const zs::string& path) {
  // Root is required to exist
  if (path_is_root(path)) {
    return {};
  }

  if (zb::sys::path_ref(path.c_str()).is_directory()) {
    return {};
  }

  zs::string p(path_dirname(path), path.get_allocator());

  if (!zb::sys::path_ref(p.c_str()).is_directory()) {
    if (auto err = path_mktree(p)) {
      return err;
    }
  }

  if (!path_has_extension(path)) {
    if (auto err = path_mkdir(path)) {
      return err;
    }
  }
  return {};
}

/**
 * \note This method writes to the filesystem.
 *
 * Deletes the referenced file. This cannot be used to
 * delete a directory rather than a file.
 *
 * \see remove() rmdir()
 */
zs::error_result path_unlink(const zs::string& path) {
#if ZS_UNIX
  if (::unlink(path.c_str()) < 0)

  {
    return zs::error_code::invalid;
  }

  return {};

#elif defined(_WIN32)
  std::wstring utf16 = utf8_to_utf16(m_path);
  if (_wunlink(utf16.c_str()) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif
}

/**
 * \note This method writes to the filesystem.
 *
 * This method, which is akin to the UNIX "rm -r" command, removes
 * the entire referenced directory hierarchy recursively, including
 * any files and directories contained therein.
 */
zs::error_result path_rmtree(const zs::string& path) {

  if (zb::sys::path_ref(path.c_str()).is_directory()) {
    //    std::vector<Path> kids = children();

    std::filesystem::path filepath(path);

    for (const std::filesystem::directory_entry& dir_entry :
        std::filesystem::directory_iterator{ filepath }) {
      path_rmtree(zs::string(dir_entry.path().c_str(), path.get_allocator()));
    }

    //    for(std::vector<Path>::iterator iter=kids.begin(); iter != kids.end();
    //    iter++) {
    //       join(*iter).rmtree();
    //    }

    zb::sys::path_ref(path).rmdir();
  }
  else {
    // file or similar.
    path_unlink(path);
  }

  return {};
}

/**
 * \note This method writes to the file system.
 *
 * Renames a file to another name without involving file streams.
 *
 * \param[in] newname The new name of the file.
 */
zs::error_result path_rename(const zs::string& path, const zs::string& new_path) {

#if ZS_UNIX
  //  std::string nstr = native();
  //  std::string newname_nstr = newname.native();

  if (::rename(path.c_str(), new_path.c_str()) != 0) {
    return zs::error_code::invalid;
  }
  return {};
#elif defined(_WIN32)
  std::wstring utf16_oldname = utf8_to_utf16(m_path);
  std::wstring utf16_newname = utf8_to_utf16(newname.m_path);

  if (_wrename(utf16_oldname.c_str(), utf16_newname.c_str()) != 0)
    throw Pathie::ErrnoError(errno);
#else
#error Unsupported system.
#endif
}

zs::error_result path_remove(const zs::string& path) {
#if ZS_UNIX
  if (::remove(path.c_str()) != 0) {
    return zs::error_code::invalid;
  }

  return {};
#elif defined(_WIN32)
  return zs::error_code::unimplemented;
#else
#error Unsupported system.
#endif
}

/**
 * \note This method acceses the filesystem.
 *
 * Checks if this file is a symbolic link; also
 * works with NTFS symlinks on Windows. Returns false
 * rather than erroring out if the referenced path does
 * not exist.
 */
bool path_is_symlink(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::lstat(path.c_str(), &s) < 0) {
    return false;
  }

  return S_ISLNK(s.st_mode);

#elif defined(_WIN32)
  if (!path_exists(path)) {
    return false;
  }

  return false;
  // ntifs.h is currently not included in msys2
  // std::wstring path = utf8_to_utf16(m_path);
  // return is_ntfs_symlink(path.c_str());
#else
#error Unsupported system.
#endif
}

/**
 * \note This method acceses the filesystem.
 *
 * Checks if this file is a socket; also
 * Returns false
 * rather than erroring out if the referenced path does
 * not exist.
 */
bool path_is_socket(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::lstat(path.c_str(), &s) < 0) {
    return false;
  }

  return S_ISSOCK(s.st_mode);

#elif defined(_WIN32)
  if (!path_exists(path)) {
    return false;
  }

  return false;
  // ntifs.h is currently not included in msys2
  // std::wstring path = utf8_to_utf16(m_path);
  // return is_ntfs_symlink(path.c_str());
#else
#error Unsupported system.
#endif
}

bool path_is_fifo(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::lstat(path.c_str(), &s) < 0) {
    return false;
  }

  return S_ISFIFO(s.st_mode);

#elif defined(_WIN32)
  if (!path_exists(path)) {
    return false;
  }

  return false;
  // ntifs.h is currently not included in msys2
  // std::wstring path = utf8_to_utf16(m_path);
  // return is_ntfs_symlink(path.c_str());
#else
#error Unsupported system.
#endif
}

/**
 * \note This method accesses the file system.
 *
 * Returns the file’s last access time. The value is not
 * really reliable.
 */
time_t path_access_time(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::stat(path.c_str(), &s) < 0) {
    return -1;
  }

//    throw(Pathie::ErrnoError(errno));
#elif defined(_WIN32)
  struct _stat s;
  std::wstring utf16 = utf8_to_utf16(m_path);

  if (_wstat(utf16.c_str(), &s) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif

  return s.st_atime;
}

/**
 * \note This method accesses the file system.
 *
 * Returns the file’s last modification time.
 */
time_t path_modified_time(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::stat(path.c_str(), &s) < 0) {
    return -1;
  }

//    throw(Pathie::ErrnoError(errno));
#elif defined(_WIN32)
  struct _stat s;
  std::wstring utf16 = utf8_to_utf16(m_path);

  if (_wstat(utf16.c_str(), &s) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif

  return s.st_mtime;
}

/**
 * \note This method accesses the file system.
 *
 * Returns the file’s creation time.
 */
time_t path_creation_time(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::stat(path.c_str(), &s) < 0) {
    return -1;
  }

//    throw(Pathie::ErrnoError(errno));
#elif defined(_WIN32)
  struct _stat s;
  std::wstring utf16 = utf8_to_utf16(m_path);

  if (_wstat(utf16.c_str(), &s) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif

  return s.st_ctime;
}

/**
 * \note This method accesses the file system.
 *
 * Returns the file size.
 */
int_t path_size(const zs::string& path) {
#if ZS_UNIX
  struct stat s;
  if (::stat(path.c_str(), &s) < 0) {
    return 0;
  }

#elif defined(_WIN32)
  struct _stat s;
  std::wstring utf16 = utf8_to_utf16(m_path);

  if (_wstat(utf16.c_str(), &s) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif

  return s.st_size;
}

/**
 * Replaces the current extension with the given new extension
 * and returns the result. If the referenced path doesn’t have
 * a file extension currently, the new extension is appended.
 *
 * \param new_extension The new extension. If the leading point
 * is missing, it will automatically be prepended.
 *
 * \returns The new Path instance.
 */
zs::error_result path_replace_extension(zs::string& path, std::string_view new_ext) {

  std::string_view old_extension = path_extension(path);

  if (old_extension.empty()) {
    if (new_ext.find('.') == std::string_view::npos) {
      path.push_back('.');
    }
    path += new_ext;
    return {};
  }

  size_t pos = path.find(old_extension);

  path = zs::string(std::string_view(path).substr(0, pos), path.get_allocator());
  if (new_ext.find('.') == std::string_view::npos) {
    path.push_back('.');
  }
  path += new_ext;

  return {};
}

/**
 * This method allows you to access a specific component in the
 * path string. The first component has the index 0; for an
 * absolute path, it will be the / entry.
 *
 * If you specify an index that is beyond the end of the path,
 * an std::out_of_range exception will be thrown.
 *
 * \param index Index of the component to retrieve.
 *
 * \see component_count()
 *
 * \remark This operator loops over the path string internally
 * each time you request an element. If you want to index the
 * path consecutively, you might consider using burst(), which
 * can be more performant as it only loops once over the path
 * string.
 */
std::string_view path_get_component(std::string_view path, int_t idx) {
  if (path.empty()) {
    return "";
  }

  if (idx < 0) {
    const size_t count = path_component_count(path);
    idx = count + idx;
    if (idx < 0) {
      return "";
    }
  }

  // Absolute path index 0 needs special treatment.
  if (idx == 0 && path[0] == '/') {
    return "/";
  }

  size_t pos = 0;
  size_t last_pos = 0;
  int_t i = 0;
  while ((pos = path.find('/', pos)) != std::string_view::npos) {
    if (i == idx) {
      return path.substr(last_pos, pos - last_pos);
    }

    last_pos = ++pos;
    i++;
  }

  // Last element requested
  if (idx == i) {
    return path.substr(last_pos);
  }

  // Out of range.
  return "";
}

//
//
//

#define ZS_GET_PATH()                       \
  get_path(vm);                             \
  if (ZBASE_UNLIKELY(!path)) {              \
    vm.set_error("Invalid fs.path object"); \
    return -1;                              \
  }

static int_t path_file_permission_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();

  std::filesystem::path filepath(*path);

  std::filesystem::file_status res;
  try {
    res = std::filesystem::status(filepath);
  } catch (const std::exception& e) {
    vm.push_integer(0);
    return 1;
  }

  vm.push_integer((int_t)res.permissions());
  return 1;
}

static int_t path_file_permission_string_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();

  std::filesystem::path filepath(*path);
  std::filesystem::file_status res;

  try {
    res = std::filesystem::status(filepath);
  } catch (const std::exception& e) {
    vm.push_null();
    return 1;
  }

  char ss[10];
  ss[0] = '-';

  std::filesystem::perms p = res.permissions();

#define PERM_LETTER(index, name, letter) \
  ss[index] = ((std::filesystem::perms::none == (std::filesystem::perms::name & p)) ? '-' : letter)

  PERM_LETTER(1, owner_read, 'r');
  PERM_LETTER(2, owner_write, 'w');
  PERM_LETTER(3, owner_exec, 'x');
  PERM_LETTER(4, group_read, 'r');
  PERM_LETTER(5, group_write, 'w');
  PERM_LETTER(6, group_exec, 'x');
  PERM_LETTER(7, others_read, 'r');
  PERM_LETTER(8, others_write, 'w');
  PERM_LETTER(9, others_exec, 'x');

#undef PERM_LETTER

  vm.push_string(std::string_view(ss, 10));
  return 1;
}

static int_t path_file_type_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();

  std::filesystem::path filepath(*path);

  std::filesystem::file_status res;
  try {
    res = std::filesystem::status(filepath);
  } catch (const std::exception& e) {
    vm.push_string("unknown");
    return 1;
  }

  vm.push_string(file_type_to_string(res.type()));
  return 1;
}

static int_t path_touch_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  return vm.push_bool(!zb::sys::path_ref(*path).touch());
}

static int_t path_mkdir_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(!path_mktree(*path));
  return 1;
}

static int_t path_rmdir_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(!zb::sys::path_ref(*path).rmdir());
  return 1;
}

static int_t path_rmtree_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(!path_rmtree(*path));
  return 1;
}

static int_t path_prune_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm->push(create_path(vm, path_prune(*path)));
  return 1;
}

static int_t path_tostring_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm->push(zs::_s(vm.get_engine(), *path));
  return 1;
}

static int_t path_get_impl(zs::vm_ref vm) {
  // vm[0] should be the user_data.
  // vm[1] should be the key.

  if (vm.stack_size() != 2) {
    return -1;
  }

  zs::string* path = ZS_GET_PATH();
  const object& key = vm[1];

  if (key.is_integer()) {
    int_t idx = key._int;

    std::string_view component = path_get_component(*path, idx);
    if (component.empty()) {
      return -1;
    }

    vm.push_string(component);
    return 1;
  }

  else if (key == "extension") {
    vm.push_string(path_extension(*path));
    return 1;
  }
  else if (key == "filename") {
    vm.push_string(path_filename(*path));
    return 1;
  }
  else if (key == "stem") {
    vm.push_string(path_stem(*path));
    return 1;
  }
  else if (key == "parent") {
    //    vm.push_string(path_dirname(*path));
    vm.push(create_path(vm, path_dirname(*path)));
    return 1;
  }
  else if (key == "dirname") {
    //    vm.push_string(path_dirname(*path));
    vm.push_string(path_stem(path_dirname(*path)));
    return 1;
  }
  else if (key == "content") {
    zs::file_loader loader(vm.get_engine());

    if (auto err = loader.open(*path)) {
      return 0;
    }

    vm.push_string(loader.content());
    return 1;
  }

  else if (key == "type") {

    std::filesystem::path filepath(*path);

    std::filesystem::file_status res;
    try {
      res = std::filesystem::status(filepath);
    } catch (const std::exception& e) {
      vm.push_string("unknown");
      return 1;
    }

    vm.push_string(file_type_to_string(res.type()));
    return 1;
  }

  else if (key == "owner_read") {
    std::filesystem::path filepath(*path);
    std::filesystem::file_status res;

    try {
      res = std::filesystem::status(filepath);
    } catch (const std::exception& e) {
      vm.push_bool(false);
      return 1;
    }

    std::filesystem::perms p = res.permissions();
    vm.push_bool(std::filesystem::perms::owner_read == (std::filesystem::perms::owner_read & p));
    return 1;
  }
  else if (key == "owner_write") {
    std::filesystem::path filepath(*path);
    std::filesystem::file_status res;

    try {
      res = std::filesystem::status(filepath);
    } catch (const std::exception& e) {
      vm.push_bool(false);
      return 1;
    }

    std::filesystem::perms p = res.permissions();
    vm.push_bool(std::filesystem::perms::owner_write == (std::filesystem::perms::owner_write & p));
    return 1;
  }
  else if (key == "owner_exec") {
    std::filesystem::path filepath(*path);
    std::filesystem::file_status res;

    try {
      res = std::filesystem::status(filepath);
    } catch (const std::exception& e) {
      vm.push_bool(false);
      return 1;
    }

    std::filesystem::perms p = res.permissions();
    vm.push_bool(std::filesystem::perms::owner_exec == (std::filesystem::perms::owner_exec & p));
    return 1;
  }
  else if (key == "owner_all") {
    std::filesystem::path filepath(*path);
    std::filesystem::file_status res;

    try {
      res = std::filesystem::status(filepath);
    } catch (const std::exception& e) {
      vm.push_bool(false);
      return 1;
    }

    std::filesystem::perms p = res.permissions();
    vm.push_bool(std::filesystem::perms::owner_all == (std::filesystem::perms::owner_all & p));
    return 1;
  }

  return -1;
}

static int_t path_set_impl(zs::vm_ref vm) {
  // vm[0] should be the user_data.
  // vm[1] should be the key.
  // vm[2] should be the value.

  if (vm.stack_size() != 3) {
    return -1;
  }

  zs::string* path = ZS_GET_PATH();
  const zs::object& key = vm[1];
  const object& value = vm[2];

  if (key == "extension") {
    path_replace_extension(*path, value.get_string_unchecked());
    return 0;
  }

  return -1;
}

static int_t path_is_root_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_is_root(*path));
  return 1;
}

static int_t path_is_directory_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(zb::sys::path_ref(path->c_str()).is_directory());
  return 1;
}

static int_t path_is_file_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(zb::sys::path_ref(path->c_str()).is_file());
  return 1;
}

static int_t path_is_symlink_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_is_symlink(*path));
  return 1;
}

static int_t path_is_socket_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_is_socket(*path));
  return 1;
}

static int_t path_is_fifo_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_is_fifo(*path));
  return 1;
}

static int_t path_is_absolute_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_is_absolute(*path));
  return 1;
}

static int_t path_exists_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_exists(*path));
  return 1;
}

static int_t path_has_filename_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_has_filename(*path));
  return 1;
}

static int_t path_has_extension_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(path_has_extension(*path));
  return 1;
}

static int_t path_has_stem_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(std::filesystem::path(*path).has_stem());
  return 1;
}

static int_t path_component_count_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_integer((int_t)path_component_count(*path));
  return 1;
}

static int_t path_get_filename_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_string(path_filename(*path));
  return 1;
}

static int_t path_get_extension_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_string(path_extension(*path));
  return 1;
}

static int_t path_get_stem_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_string(path_stem(*path));
  return 1;
}

static int_t path_get_dirname_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_string(path_dirname(*path));
  return 1;
}

static int_t path_split_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  zs::object arr = zs::_a(vm.get_engine(), 0);
  zs::vector<object>& vec = arr.as_array();

  std::filesystem::path filepath(*path);
  for (const auto& it : filepath) {
    vec.push_back(zs::_s(vm.get_engine(), it.c_str()));
  }

  vm->push(arr);
  return 1;
}

static int_t path_list_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();

  const size_t nargs = vm.stack_size();

  if (nargs >= 2 and vm[1].is_array() and !vm[1]._array->empty()) {
    const zs::vector<zs::object>& ext_arr = vm[1].as_array();

    const auto is_in_ext_vec = [&](std::string_view p) {
      if (p.empty()) {
        return false;
      }

      for (const auto& it : ext_arr) {
        std::string_view ext_str = it.get_string_unchecked();
        if (ext_str == p || (ext_str == p.substr(1))) {
          return true;
        }
      }

      return false;
    };

    zs::object arr = zs::_a(vm.get_engine(), 0);

    std::filesystem::path filepath(*path);
    for (const std::filesystem::directory_entry& dir_entry :
        std::filesystem::directory_iterator{ filepath }) {

      const std::filesystem::path& p = dir_entry.path();

      if (is_in_ext_vec(p.extension().c_str())) {
        arr.as_array().push_back(create_path(vm, p.c_str()));
      }
    }

    vm->push(arr);
    return 1;
  }
  else if (nargs >= 2 and vm[1].is_string()) {

    std::string_view ext_str = vm[1].get_string_unchecked();

    zs::object arr = zs::_a(vm.get_engine(), 0);

    std::filesystem::path filepath(*path);
    for (const std::filesystem::directory_entry& dir_entry :
        std::filesystem::directory_iterator{ filepath }) {

      const std::filesystem::path& p = dir_entry.path();
      std::string_view ext = p.extension().c_str();

      if (ext_str == ext || (!ext.empty() && ext_str == ext.substr(1))) {
        arr.as_array().push_back(create_path(vm, p.c_str()));
      }
    }

    vm->push(arr);
    return 1;
  }
  else {
    zs::object arr = zs::_a(vm.get_engine(), 0);

    std::filesystem::path filepath(*path);
    for (const std::filesystem::directory_entry& dir_entry :
        std::filesystem::directory_iterator{ filepath }) {
      arr.as_array().push_back(create_path(vm, dir_entry.path().c_str()));
    }

    vm->push(arr);
    return 1;
  }
}

static int_t path_list_recursive_impl(zs::vm_ref vm) {
  using check_ext = bool (*)(std::string_view, const void*);

  zs::string* path = ZS_GET_PATH();

  std::filesystem::path filepath(*path);
  zs::object arr = zs::_a(vm.get_engine(), 0);

  const size_t nargs = vm.stack_size();

  const void* data = nullptr;
  check_ext is_valid_extension = nullptr;
  std::string_view ext_str;

  if (nargs >= 2 and vm[1].is_array() and !vm[1]._array->empty()) {
    const zs::vector<zs::object>& ext_arr = vm[1].as_array();
    data = &ext_arr;

    is_valid_extension = [](std::string_view ext, const void* data) {
      if (ext.empty()) {
        return false;
      }
      std::string_view sext = ext.substr(1);

      for (const auto& it : *(const zs::vector<zs::object>*)data) {
        if (zb::is_one_of(it.get_string_unchecked(), ext, sext)) {
          return true;
        }
      }

      return false;
    };
  }
  else if (nargs >= 2 and vm[1].is_string()) {
    ext_str = vm[1].get_string_unchecked();
    data = &ext_str;

    is_valid_extension = [](std::string_view ext, const void* data) {
      return !ext.empty() && zb::is_one_of(*(std::string_view*)data, ext, ext.substr(1));
    };
  }

  if (is_valid_extension) {
    for (const std::filesystem::directory_entry& dir_entry : std::filesystem::recursive_directory_iterator{
             filepath, std::filesystem::directory_options::skip_permission_denied }) {

      const std::filesystem::path& p = dir_entry.path();
      std::string_view ext = path_extension(p.c_str());

      if (is_valid_extension(ext, data)) {
        arr.as_array().push_back(create_path(vm, p.c_str()));
      }
    }
  }
  else {
    for (const std::filesystem::directory_entry& dir_entry : std::filesystem::recursive_directory_iterator{
             filepath, std::filesystem::directory_options::skip_permission_denied }) {
      arr.as_array().push_back(create_path(vm, dir_entry.path().c_str()));
    }
  }

  vm->push(arr);
  return 1;
}

static int_t path_rename_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();

  const size_t nargs = vm.stack_size();
  if (nargs != 2) {
    return -1;
  }

  zs::string new_path(vm[1].get_string_unchecked(), zs::allocator<char>(vm.get_engine()));

  vm.push_bool(!path_rename(*path, new_path));
  return 1;
}

static int_t path_remove_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_bool(!path_remove(*path));
  return 1;
}

static int_t path_file_size_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_integer(path_size(*path));
  return 1;
}

static int_t path_length_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_integer((int_t)path->size());
  return 1;
}

static int_t path_access_time_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_integer((int_t)path_access_time(*path));
  return 1;
}

static int_t path_modified_time_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_integer((int_t)path_modified_time(*path));
  return 1;
}

static int_t path_creation_time_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  vm.push_integer((int_t)path_creation_time(*path));
  return 1;
}

static int_t path_read_all_impl(zs::vm_ref vm) {
  zs::string* path = ZS_GET_PATH();
  zs::file_loader loader(vm.get_engine());

  if (auto err = loader.open(*path)) {
    return 0;
  }

  vm.push_string(loader.content());
  return 1;
}

static void set_path_delegate_methods(zs::vm_ref vm, zs::object& path_delegate,
    const std::initializer_list<std::pair<zs::object, zs::native_cpp_closure_t>>& list) {
  zs::engine* eng = vm->get_engine();
  zs::table_object* tbl = path_delegate._table;

  for (auto& n : list) {
    tbl->set(std::move(n.first), zs::_nc(eng, n.second));
  }
}

static zs::object create_path_delegate(zs::vm_ref vm) {
  zs::engine* eng = vm->get_engine();

  zs::object delegate_key = zs::_sv(k_path_delegate_name);

  zs::object_unordered_map<zs::object>& registry_map = eng->get_registry_table()._table->get_map();
  if (auto it = registry_map.find(delegate_key); it != registry_map.end()) {
    return it->second;
  }

  zs::object path_delegate = zs::object::create_table(eng);

  set_path_delegate_methods(vm, path_delegate,
      {
          { zs::_ss("get_filename"), path_get_filename_impl }, //
          { zs::_ss("get_extension"), path_get_extension_impl }, //
          { zs::_ss("get_stem"), path_get_stem_impl }, //
          { zs::_ss("get_dirname"), path_get_dirname_impl }, //
          { zs::_ss("get_parent"), path_get_dirname_impl }, //
          { zs::_ss("is_directory"), path_is_directory_impl }, //
          { zs::_ss("is_root"), path_is_root_impl }, //
          { zs::_ss("is_file"), path_is_file_impl }, //
          { zs::_sv(k_path_method_is_refular_file), path_is_file_impl }, //
          { zs::_ss("is_symlink"), path_is_symlink_impl }, //
          { zs::_ss("is_socket"), path_is_socket_impl }, //
          { zs::_ss("is_fifo"), path_is_fifo_impl }, //
          { zs::_ss("file_type"), path_file_type_impl }, //
          { zs::_ss("is_absolute"), path_is_absolute_impl }, //
          { zs::_ss("has_filename"), path_has_filename_impl }, //
          { zs::_ss("has_stem"), path_has_stem_impl }, //
          { zs::_ss("has_extension"), path_has_extension_impl }, //
          { zs::_ss("exists"), path_exists_impl }, //
          { zs::_ss("touch"), path_touch_impl }, //
          { zs::_ss("prune"), path_prune_impl }, //
          { zs::_ss("split"), path_split_impl }, //
          { zs::_ss("mkdir"), path_mkdir_impl }, //
          { zs::_ss("list"), path_list_impl }, //
          { zs::_sv(k_path_method_list_recursive), path_list_recursive_impl }, //
          { zs::_ss("rmdir"), path_rmdir_impl }, //
          { zs::_sv(k_path_method_rmdir_recursive), path_rmtree_impl }, //
          { zs::_ss("rename"), path_rename_impl }, //
          { zs::_ss("remove"), path_remove_impl }, //
          { zs::_ss("file_size"), path_file_size_impl }, //
          { zs::_ss("length"), path_length_impl }, //
          { zs::_sv(k_path_method_component_count), path_component_count_impl }, //
          { zs::_ss("access_time"), path_access_time_impl }, //
          { zs::_ss("modified_time"), path_modified_time_impl }, //
          { zs::_ss("creation_time"), path_creation_time_impl }, //
          { zs::_ss("perm"), path_file_permission_impl }, //
          { zs::_ss("perm_string"), path_file_permission_string_impl }, //
          { zs::_ss("read_all"), path_read_all_impl }, //
          { zs::_ss("tostring"), path_tostring_impl }, //
          { zs::constants::get<meta_method::mt_tostring>(), path_tostring_impl }, //
          { zs::constants::get<meta_method::mt_get>(), path_get_impl }, //
          { zs::constants::get<meta_method::mt_set>(), path_set_impl } //
      });

  return (registry_map[delegate_key] = std::move(path_delegate));
}

zs::object create_path(zs::vm_ref vm, std::string_view str_path) {
  zs::engine* eng = vm.get_engine();

  zs::object path_obj = zs::object::create_user_data<zs::string>(eng, str_path, zs::allocator<char>(eng));

  path_obj._udata->set_to_string_callback(
      [](const object_base& obj, std::ostream& stream) -> zs::error_result {
        stream << zb::quoted(obj._udata->data_ref<zs::string>());
        return {};
      });

  path_obj.set_delegate(create_path_delegate(vm));
  path_obj._udata->set_uid(zs::_sv(k_path_uid));

  return path_obj;
}

} // namespace zs::path_library.