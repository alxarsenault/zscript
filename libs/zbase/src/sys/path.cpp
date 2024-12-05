#include <zbase/sys/path.h>
#include <zbase/sys/assert.h>

#if !defined(ZBASE_PATH_UNIX) \
    && (defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__) || defined(BSD))
#define ZBASE_PATH_UNIX 1
#else
#deine ZBASE_PATH_UNIX 0
#endif

#if ZBASE_PATH_UNIX
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
#define ZBASE_PATH_BSD 1
#include <sys/time.h>
#include <sys/sysctl.h>
#else
#define ZBASE_PATH_BSD 0
#endif

#ifdef __APPLE__
#define ZBASE_PATH_APPLE 1
#include <mach-o/dyld.h>
#else
#define ZBASE_PATH_APPLE 0
#endif

ZBASE_BEGIN_SUB_NAMESPACE(sys)

bool path_detail::is_root(__zb::string_view path) {
#if ZBASE_PATH_UNIX
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

bool path_detail::is_directory(const char* path) {
#if ZBASE_PATH_UNIX
  struct stat s;
  if (::stat(path, &s) < 0) {
    int errsav = errno;

    // "Not found" means it isn’t a directory.
    if (errsav == ENOENT)
      return false;
    else
      return false;
    //      throw(Pathie::ErrnoError(errsav));
  }

  if (S_ISDIR(s.st_mode)) {
    return true;
  }
  else {
    return false;
  }

#elif defined(_WIN32)
  struct _stat s;
  std::wstring utf16 = utf8_to_utf16(m_path);
  if (_wstat(utf16.c_str(), &s) < 0) {
    int errsav = errno;

    if (errsav == ENOENT)
      return false;
    else
      throw(Pathie::ErrnoError(errsav));
  }

  return s.st_mode & S_IFDIR;
#else
#error Unsupported system.
#endif
}

bool path_detail::is_file(const char* path) {
#if ZBASE_PATH_UNIX
  struct stat s;

  if (::stat(path, &s) < 0) {
    int errsav = errno;

    if (errsav == ENOENT)
      return false;
    else
      return false;
  }

  if (S_ISREG(s.st_mode))
    return true;
  else
    return false;
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

bool path_detail::is_symlink(const char* path) {
#if ZBASE_PATH_UNIX
  struct stat s;
  //  std::string nstr = native();

  if (::lstat(path, &s) < 0) {
    int errsav = errno;

    if (errsav == ENOENT)
      return false;
    else

    {
      return false;
    }
  }

  if (S_ISLNK(s.st_mode)) {
    return true;
  }
  else {
    return false;
  }
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

bool path_detail::is_absolute(const char* path) {
#if ZBASE_PATH_UNIX
  return path[0] == '/';
#elif defined(_WIN32)
  // / is root on current drive
  if (path[0] == '/') {
    return true;
  }

  // This is the only position where : is allowed on windows, and if it is there, the path is absolute with a
  // drive letter (X:/)
  return path[1] == ':';
#else
#error Unsupported system.
#endif
}

bool path_detail::exists(const char* path) {
#if ZBASE_PATH_UNIX
  if (::access(path, F_OK) == -1) {
    int errsav = errno;
    if (errsav == ENOENT) {
      return false;
    }
    else {
      return false;
      //      throw(Pathie::ErrnoError(errsav));
    }
  }
  else {
    return true;
  }
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

bool path_detail::has_filename(__zb::string_view path) {
  // TODO: Make this faster.
  return !get_filename(path).empty();
}

bool path_detail::has_stem(__zb::string_view path) {
  // TODO: Make this faster.
  return !get_stem(path).empty();
}

bool path_detail::has_extension(__zb::string_view path) {
  // TODO: Make this faster.
  return !get_extension(path).empty();
}

__zb::string_view path_detail::get_filename(__zb::string_view path) {
  if (path == "." or path == ".." or is_root(path)) {
    return path;
  }

  if (size_t pos = path.rfind("/"); pos != __zb::string_view::npos) {
    return path.substr(pos + 1);
  }

  return path;
}

__zb::string_view path_detail::get_stem(__zb::string_view path) {
  if (path == "." or path == ".." or is_root(path)) {
    return path;
  }

  if (size_t pos = path.rfind("/"); pos != __zb::string_view::npos) {
    path = path.substr(pos + 1);
  }

  if (size_t pos = path.find("."); pos != __zb::string_view::npos) {
    return path.substr(0, pos);
  }

  return path;
}

__zb::string_view path_detail::get_extension(__zb::string_view path) {
  if (path.empty()) {
    return "";
  }

  if (path == "." || path == "..") {
    return "";
  }

  if (size_t pos = path.rfind("."); pos != __zb::string_view::npos) {
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

__zb::string_view path_detail::get_root(__zb::string_view path) {
#if ZBASE_PATH_UNIX
  return "/";
#elif defined(_WIN32)
  // Check if we have an absolute path with drive,
  // otherwise return the root for the current drive.
  if (path[1] == ':') {
    // Colon is on Windows only allowed here to denote a preceeding drive letter => absolute path
    return path.substr(0, 3);
  }
  else {
    return "/";
  }
#else
#error Unsupported system.
#endif
}

__zb::string_view path_detail::get_dirname(__zb::string_view path) {

  if (path == ".") {
    return ".";
  }
  else if (path == "..") {
    return ".";
  }
  else if (is_root(path)) {
    return path;
  }

  if (size_t pos = path.rfind("/"); pos != __zb::string_view::npos) {
    if (pos == 0) {
      return get_root(path);
    }
#ifdef _WIN32
    else if (pos == 1 && path[1] == ':') { // X:/foo
      return get_root(path);
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

__zb::error_result path_detail::touch(const char* path) {
  // FreeBSD didn’t have futimens() yet as of testing (december 2014)
#if ZBASE_PATH_BSD
  FILE* p_file = ::fopen(path, "a");

  if (!p_file) {
    return __zb::error_code::not_found;
  }

  if (::futimes(::fileno(p_file), nullptr) < 0) {
    ::fclose(p_file);
    return __zb::error_code::no_such_file_or_directory;
  }

  ::fclose(p_file);
  return {};

#elif ZBASE_PATH_UNIX
  FILE* p_file = ::fopen(path.c_str(), "a");

  if (!p_file) {
    return zs::error_code::not_found;
  } // futimens() is considered the modern variant of doing this
  // (at least according to utimes(2) on my Linux system).
  if (::futimens(::fileno(p_file), nullptr) < 0) {
    ::fclose(p_file);
    return zs::error_code::open_file_error;
  }

  ::fclose(p_file);

  return {};

#elif defined(_WIN32)
  // Create file if it does not exist yet
  if (!exists()) {
    FILE* p_file = Path::fopen("a");
    fclose(p_file);
  }

  SYSTEMTIME currenttime;
  GetSystemTime(&currenttime);

  FILETIME newtime;
  if (SystemTimeToFileTime(&currenttime, &newtime) == 0) {
    DWORD err = GetLastError();
    throw(Pathie::WindowsError(err));
  }

  std::wstring utf16 = utf8_to_utf16(m_path);
  HANDLE filehandle = CreateFileW(utf16.c_str(), FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (filehandle == INVALID_HANDLE_VALUE) {
    DWORD err = GetLastError();
    throw(Pathie::WindowsError(err));
  }

  if (SetFileTime(filehandle, NULL, &newtime, &newtime) == 0) {
    int errsav = GetLastError();
    CloseHandle(filehandle);
    throw(Pathie::WindowsError(errsav));
  }

  CloseHandle(filehandle);
#else
#error Unsupported system.
#endif
}

__zb::error_result path_detail::mkdir(const char* path) {
#if ZBASE_PATH_UNIX
  //  std::string nstr = native();

  if (::mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
    return __zb::error_code::invalid;
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

__zb::error_result path_detail::mktree(const char* path) {
  // Root is required to exist
  if (is_root(path)) {
    return {};
  }

  if (is_directory(path)) {
    return {};
  }

  // TODO: Fix this.
  std::string p(get_dirname(path));

  if (!is_directory(p.c_str())) {
    if (auto err = mktree(p.c_str())) {
      return err;
    }
  }

  if (!has_extension(path)) {
    if (auto err = mkdir(path)) {
      return err;
    }
  }

  return {};
}

__zb::error_result path_detail::unlink(const char* path) {
#if ZBASE_PATH_UNIX
  if (::unlink(path) < 0)

  {
    return __zb::error_code::invalid;
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

__zb::error_result path_detail::rmdir(const char* path) {

#if ZBASE_PATH_UNIX

  if (::rmdir(path) < 0) {
    return __zb::error_code::invalid;
  }

  return {};
#elif defined(_WIN32)
  std::wstring utf16 = utf8_to_utf16(m_path);
  if (_wrmdir(utf16.c_str()) < 0)
    throw(Pathie::ErrnoError(errno));
#else
#error Unsupported system.
#endif
}

__zb::error_result path_detail::rename(const char* path, const char* new_path) {

#if ZBASE_PATH_UNIX
  if (::rename(path, new_path) != 0) {
    return __zb::error_code::invalid;
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
int64_t path_detail::path_conf(const char* path) {
  int64_t size = (int64_t)::pathconf(path, _PC_PATH_MAX);
  return size == -1 ? PATH_MAX : size;
}

const char* path_detail::getcwd(char* buf, size_t size) {
#if ZBASE_PATH_UNIX
  return ::getcwd(buf, size);

#elif defined(_WIN32)
  return nullptr;
#else
#error Unsupported system.
#endif
}

size_t path_detail::file_size(const char* path) {
#if ZBASE_PATH_UNIX
  struct stat s;
  if (::stat(path, &s) < 0) {
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

ZBASE_END_SUB_NAMESPACE(sys)
