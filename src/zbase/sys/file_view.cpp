#include <zbase/sys/file_view.h>

#include <errno.h>
#include <stdio.h>
#include <zbase/utility/print.h>

// FSX_PRAGMA_PUSH()
// FSX_PRAGMA_DISABLE_WARNING_CLANG("-Wunused-macros")

#undef __FSX_FILE_VIEW_USE_WINDOWS_MEMORY_MAP
#undef __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP

#if defined(_WIN32)
#define __FSX_FILE_VIEW_USE_WINDOWS_MEMORY_MAP 1
#define __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP 0

// #elif __FSX_UNISTD__
#elif __has_include("unistd.h")
#define __FSX_FILE_VIEW_USE_WINDOWS_MEMORY_MAP 0
#include <unistd.h>

#ifdef HAVE_MMAP
#define __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP HAVE_MMAP
#elif _POSIX_VERSION >= 199506L
#define __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP 1
#else
#define __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP 0
#endif

// Copy to memory.
#else
#define __FSX_FILE_VIEW_USE_WINDOWS_MEMORY_MAP 0
#define __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP 0
#endif

#if __FSX_FILE_VIEW_USE_WINDOWS_MEMORY_MAP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#elif __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#endif

ZBASE_BEGIN_NAMESPACE

namespace {

static inline constexpr __zb::error_code errno_to_error_code(int err) {
  switch (err) {
  case EPERM:
    return error_code::permission_denied;
  case ENOENT:
    return error_code::no_such_file_or_directory;
  case ESRCH:
    return error_code::invalid_data;

  case EBADF:
    return error_code::bad_file_descriptor;

  case ENOMEM:
    return error_code::not_enough_memory;
  case EACCES:
    return error_code::permission_denied;
  case EFAULT:
    return error_code::bad_address;

  case EEXIST:
    return error_code::file_exists;
  case ENOTDIR:
    return error_code::not_a_directory;
  case EISDIR:
    return error_code::is_a_directory;
  case EINVAL:
    return error_code::invalid_argument;

  default:
    return error_code::unknown;
  }
  // #define EPERM           1               /* Operation not permitted */
  // #define ENOENT          2               /* No such file or directory */
  // #define ESRCH           3               /* No such process */
  // #define EINTR           4               /* Interrupted system call */
  // #define EIO             5               /* Input/output error */
  // #define ENXIO           6               /* Device not configured */
  // #define E2BIG           7               /* Argument list too long */
  // #define ENOEXEC         8               /* Exec format error */
  // #define EBADF           9               /* Bad file descriptor */
  // #define ECHILD          10              /* No child processes */
  // #define EDEADLK         11              /* Resource deadlock avoided */
  //                                     /* 11 was EAGAIN */
  // #define ENOMEM          12              /* Cannot allocate memory */
  // #define EACCES          13              /* Permission denied */
  // #define EFAULT          14              /* Bad address */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define ENOTBLK         15              /* Block device required */
  // #endif
  // #define EBUSY           16              /* Device / Resource busy */
  // #define EEXIST          17              /* File exists */
  // #define EXDEV           18              /* Cross-device link */
  // #define ENODEV          19              /* Operation not supported by device */
  // #define ENOTDIR         20              /* Not a directory */
  // #define EISDIR          21              /* Is a directory */
  // #define EINVAL          22              /* Invalid argument */
  // #define ENFILE          23              /* Too many open files in system */
  // #define EMFILE          24              /* Too many open files */
  // #define ENOTTY          25              /* Inappropriate ioctl for device */
  // #define ETXTBSY         26              /* Text file busy */
  // #define EFBIG           27              /* File too large */
  // #define ENOSPC          28              /* No space left on device */
  // #define ESPIPE          29              /* Illegal seek */
  // #define EROFS           30              /* Read-only file system */
  // #define EMLINK          31              /* Too many links */
  // #define EPIPE           32              /* Broken pipe */
  //
  ///* math software */
  // #define EDOM            33              /* Numerical argument out of domain */
  // #define ERANGE          34              /* Result too large */
  //
  ///* non-blocking and interrupt i/o */
  // #define EAGAIN          35              /* Resource temporarily unavailable */
  // #define EWOULDBLOCK     EAGAIN          /* Operation would block */
  // #define EINPROGRESS     36              /* Operation now in progress */
  // #define EALREADY        37              /* Operation already in progress */
  //
  ///* ipc/network software -- argument errors */
  // #define ENOTSOCK        38              /* Socket operation on non-socket */
  // #define EDESTADDRREQ    39              /* Destination address required */
  // #define EMSGSIZE        40              /* Message too long */
  // #define EPROTOTYPE      41              /* Protocol wrong type for socket */
  // #define ENOPROTOOPT     42              /* Protocol not available */
  // #define EPROTONOSUPPORT 43              /* Protocol not supported */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define ESOCKTNOSUPPORT 44              /* Socket type not supported */
  // #endif
  // #define ENOTSUP         45              /* Operation not supported */
  // #if !__DARWIN_UNIX03 && !defined(KERNEL)
  ///*
  //* This is the same for binary and source copmpatability, unless compiling
  //* the kernel itself, or compiling __DARWIN_UNIX03; if compiling for the
  //* kernel, the correct value will be returned.  If compiling non-POSIX
  //* source, the kernel return value will be converted by a stub in libc, and
  //* if compiling source with __DARWIN_UNIX03, the conversion in libc is not
  //* done, and the caller gets the expected (discrete) value.
  //*/
  // #define EOPNOTSUPP       ENOTSUP        /* Operation not supported on socket */
  // #endif /* !__DARWIN_UNIX03 && !KERNEL */
  //
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EPFNOSUPPORT    46              /* Protocol family not supported */
  // #endif
  // #define EAFNOSUPPORT    47              /* Address family not supported by protocol family */
  // #define EADDRINUSE      48              /* Address already in use */
  // #define EADDRNOTAVAIL   49              /* Can't assign requested address */
  //
  ///* ipc/network software -- operational errors */
  // #define ENETDOWN        50              /* Network is down */
  // #define ENETUNREACH     51              /* Network is unreachable */
  // #define ENETRESET       52              /* Network dropped connection on reset */
  // #define ECONNABORTED    53              /* Software caused connection abort */
  // #define ECONNRESET      54              /* Connection reset by peer */
  // #define ENOBUFS         55              /* No buffer space available */
  // #define EISCONN         56              /* Socket is already connected */
  // #define ENOTCONN        57              /* Socket is not connected */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define ESHUTDOWN       58              /* Can't send after socket shutdown */
  // #define ETOOMANYREFS    59              /* Too many references: can't splice */
  // #endif
  // #define ETIMEDOUT       60              /* Operation timed out */
  // #define ECONNREFUSED    61              /* Connection refused */
  //
  // #define ELOOP           62              /* Too many levels of symbolic links */
  // #define ENAMETOOLONG    63              /* File name too long */
  //
  ///* should be rearranged */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EHOSTDOWN       64              /* Host is down */
  // #endif
  // #define EHOSTUNREACH    65              /* No route to host */
  // #define ENOTEMPTY       66              /* Directory not empty */
  //
  ///* quotas & mush */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EPROCLIM        67              /* Too many processes */
  // #define EUSERS          68              /* Too many users */
  // #endif
  // #define EDQUOT          69              /* Disc quota exceeded */
  //
  ///* Network File System */
  // #define ESTALE          70              /* Stale NFS file handle */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EREMOTE         71              /* Too many levels of remote in path */
  // #define EBADRPC         72              /* RPC struct is bad */
  // #define ERPCMISMATCH    73              /* RPC version wrong */
  // #define EPROGUNAVAIL    74              /* RPC prog. not avail */
  // #define EPROGMISMATCH   75              /* Program version wrong */
  // #define EPROCUNAVAIL    76              /* Bad procedure for program */
  // #endif
  //
  // #define ENOLCK          77              /* No locks available */
  // #define ENOSYS          78              /* Function not implemented */
  //
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EFTYPE          79              /* Inappropriate file type or format */
  // #define EAUTH           80              /* Authentication error */
  // #define ENEEDAUTH       81              /* Need authenticator */
  //
  ///* Intelligent device errors */
  // #define EPWROFF         82      /* Device power is off */
  // #define EDEVERR         83      /* Device error, e.g. paper out */
  // #endif
  //
  // #define EOVERFLOW       84              /* Value too large to be stored in data type */
  //
  ///* Program loading errors */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EBADEXEC        85      /* Bad executable */
  // #define EBADARCH        86      /* Bad CPU type in executable */
  // #define ESHLIBVERS      87      /* Shared library version mismatch */
  // #define EBADMACHO       88      /* Malformed Macho file */
  // #endif
  //
  // #define ECANCELED       89              /* Operation canceled */
  //
  // #define EIDRM           90              /* Identifier removed */
  // #define ENOMSG          91              /* No message of desired type */
  // #define EILSEQ          92              /* Illegal byte sequence */
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define ENOATTR         93              /* Attribute not found */
  // #endif
  //
  // #define EBADMSG         94              /* Bad message */
  // #define EMULTIHOP       95              /* Reserved */
  // #define ENODATA         96              /* No message available on STREAM */
  // #define ENOLINK         97              /* Reserved */
  // #define ENOSR           98              /* No STREAM resources */
  // #define ENOSTR          99              /* Not a STREAM */
  // #define EPROTO          100             /* Protocol error */
  // #define ETIME           101             /* STREAM ioctl timeout */
  //
  // #if __DARWIN_UNIX03 || defined(KERNEL)
  ///* This value is only discrete when compiling __DARWIN_UNIX03, or KERNEL */
  // #define EOPNOTSUPP      102             /* Operation not supported on socket */
  // #endif /* __DARWIN_UNIX03 || KERNEL */
  //
  // #define ENOPOLICY       103             /* No such policy registered */
  //
  // #if __DARWIN_C_LEVEL >= 200809L
  // #define ENOTRECOVERABLE 104             /* State not recoverable */
  // #define EOWNERDEAD      105             /* Previous owner died */
  // #endif
  //
  // #if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
  // #define EQFULL          106             /* Interface output queue is full */
  // #define ELAST           106             /* Must be equal largest errno */
  // #endif
}

struct file_view_impl {

#if __FSX_FILE_VIEW_USE_WINDOWS_MEMORY_MAP
  static __fsx::status open(const char* file_path, uint8_t*& _data, size_t& _size) noexcept {
    HANDLE hFile = CreateFileA(
        file_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
      switch (GetLastError()) {
      case ERROR_FILE_NOT_FOUND:
        return __fsx::status_code::no_such_file_or_directory;
      case ERROR_PATH_NOT_FOUND:
        return __fsx::status_code::no_such_file_or_directory;
      case ERROR_ACCESS_DENIED:
        return __fsx::status_code::permission_denied;
      default:
        return __fsx::status_code::bad_file_descriptor;
      }
    }

    DWORD file_size = GetFileSize(hFile, nullptr);
    if (file_size == INVALID_FILE_SIZE || file_size == 0) {
      CloseHandle(hFile);
      return __fsx::status_code::unknown;
    }

    HANDLE hMap = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, file_size, nullptr);
    if (!hMap) {
      CloseHandle(hFile);
      return __fsx::status_code::unknown;
    }

    uint8_t* ptr = (uint8_t*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, file_size);

    // We can call CloseHandle here, but it will not be closed until we unmap
    // the view.
    CloseHandle(hMap);
    CloseHandle(hFile);
    _data = ptr;
    _size = (size_t)file_size;

    return __fsx::status_code::success;
  }

  static inline void close(uint8_t*& _data, size_t& _size) noexcept {
    __fsx::unused(_size);
    UnmapViewOfFile(_data);
  }

  //
  // mmap
  //
#elif __FSX_FILE_VIEW_USE_POSIX_MEMORY_MAP
  static __zb::error_result open(const char* file_path, uint8_t*& _data, size_t& _size) noexcept {
    int fd = ::open(file_path, O_RDONLY);
    if (fd < 0) {
      return errno_to_error_code(errno);
    }

    // Get file size.
    off_t size = lseek(fd, 0, SEEK_END);
    if (size <= 0) {
      __zb::error_code ec = errno_to_error_code(errno);
      ::close(fd);
      return ec;
    }

    // Create file map.
    uint8_t* data = (uint8_t*)mmap(nullptr, (size_t)size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (data == MAP_FAILED) {
      __zb::error_code ec = errno_to_error_code(errno);
      ::close(fd);
      return ec;
    }

    ::close(fd);
    _data = data;
    _size = (size_t)size;
    return __zb::error_code::success;
  }

  static inline void close(uint8_t*& _data, size_t& _size) noexcept { munmap(_data, _size); }

  //
  // Using c FILE*
  //
#else
  static __zb::error_result open(const char* file_path, uint8_t*& _data, size_t& _size) noexcept {
    FILE* fd = nullptr;

#ifdef _WIN32
    {
      errno_t err;
      if ((err = fopen_s(&fd, file_path, "rb")) != 0) {
        return errno_to_error_code(err);
      }
    }
#else
    fd = ::fopen(file_path, "rb");
    if (!fd) {
      return errno_to_error_code(errno);
    }
#endif // _WIN32

    // Get file size.
    ::fseek(fd, 0, SEEK_END);
    ptrdiff_t __size = ::ftell(fd);
    if (__size <= 0) {
      __zb::error_code ec = errno_to_error_code(errno);
      ::fclose(fd);
      return ec;
    }

    ::rewind(fd);
    uint8_t* __data = (uint8_t*)::malloc(__size);

    if (!__data) {
      __zb::error_code ec = errno_to_error_code(errno);
      ::fclose(fd);
      return ec;
    }

    // Copy content into data.
    // std::fread returns the number of objects read successfully.
    if (::fread(__data, __size, 1, fd) != 1) {
      __zb::error_code ec = errno_to_error_code(errno);
      std::free(__data);

      ::fclose(fd);
      return ec;
    }

    _data = __data;
    _size = (size_t)__size;
    return __zb::error_code::success;
  }

  static inline void close(uint8_t*& _data, size_t&) noexcept { std::free(_data); }
#endif
};
} // namespace

__zb::error_result file_view::open(const char* file_path) noexcept {
  close();
  return file_view_impl::open(file_path, _data, _size);
}

void file_view::close() noexcept {
  if (_data) {
    file_view_impl::close(_data, _size);
  }

  _data = nullptr;
  _size = 0;
}
ZBASE_END_NAMESPACE
// FSX_PRAGMA_POP()
