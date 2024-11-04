#pragma once

#include <zscript/zscript.h>

namespace zs::path_library {
/// @note This method acceses the filesystem.
///
/// Checks if this is a directory. Returns false if the
/// referenced path does not exist rather than erroring out.
bool path_is_directory(const zs::string& path);

/// Checks if this path is a filesystem root. On UNIX, this
/// is the case if the path consists solely of one slash, on
/// Windows this is the case if the path looks like this:
/// "<letter>:/".
bool path_is_root(std::string_view path);

/// @note This method accesses the filesystem.
///
/// Checks if this is a file. Returns false
/// if the referenced path does not exist rather
/// than erroring out.
bool path_is_file(const zs::string& path);

/// @note This method acceses the filesystem.
///
/// Checks if this file is a symbolic link; also
/// works with NTFS symlinks on Windows. Returns false
/// rather than erroring out if the referenced path does
/// not exist.
bool path_is_symlink(const zs::string& path);

bool path_is_socket(const zs::string& path);

bool path_is_fifo(const zs::string& path);

/// Checks if this is an absolute path, i.e. one that
/// starts with a / on all systems or with X:/
/// only on Windows, where `X` is a drive letter.
///
/// Note that / on Windows is the root of the current drive
/// and hence also an absolute path.
bool path_is_absolute(const zs::string& path);

/// @note This method acceses the filesystem.
///
/// Checks if the file exists. Note that if you don’t have
/// sufficient rights for the check on the given path, this
/// method will throw an exception.
bool path_exists(const zs::string& path);

/// Returns the path’s filename, i.e. the last component
/// of the path, including the file excention.
///
/// For example, "/foo/bar.txt" has a basename of "bar.txt",
/// and "/foo/bar" has a filename of "bar".
///
/// @returns a new Path instance with only the filename.
///
/// @see dirname()
std::string_view path_filename(zb::string_view path);

bool path_has_filename(std::string_view path);

std::string_view path_stem(zb::string_view path);

/// Returns the filesystem root for this path. On UNIX,
/// this will always return /, but on Windows it will
/// return X:/ if the referenced path is an absolute path
/// with drive letter, and / if the referenced path is
/// a relative path or an absolute path on the current
/// drive.
std::string_view path_root(std::string_view path);

/// Returns the path’s dirname, i.e. all components of the
/// path except for the basename component (see basename()).
///
/// For example, "/foo/bar/baz.txt" has a dirname of "/foo/bar",
/// and "/foo/bar/baz" has a dirname of "/foo/bar".
///
/// @returns a new Path instance with only the dirname.
///
/// @see basename() parent()
std::string_view path_dirname(std::string_view path);

/// This method returns the file extension of the path,
/// if possible; otherwise it returns an empty string.
/// Filenames that consist entirely of a "file extension",
/// i.e. ".txt" or "/foo/.txt" will return an empty string.
std::string_view path_extension(std::string_view path);

/// @note This method writes to the filesystem.
///
/// Sets the file’s modification and access times to the
/// current time. If the file does not yet exist, it is created.
///
/// This is akin to the UNIX `touch` command.
zs::error_result path_touch(const zs::string& path);

/// This method, removes all occurences of . and .. from the path,
/// leaving a clean filesystem path.
///
/// Note that neither an absolute path is created, nor
/// are shortcuts other than . and .. expanded.
///
/// This method does not access file filesystem, and thus does not
/// know about symbolic links. Therefore, if the path contains symlinks,
/// the result may not be the way you expect it. Use real() if
/// you need to resolve all your symbolic links in the path.
///
/// For example, if you have a directory `/tmp/foo`, which contains a
/// symbolic link `bar` that points to `/tmp/bar`, then a path of
/// `/tmp/foo/bar/..` will be prune()d to `/tmp/foo`, although the
/// canonically correct result is `/tmp`. The latter is what you will
/// get if you use real().
///
/// @returns A new string with . and .. removed.
///
/// @see expand() real()
zs::string path_prune(const zs::string& path);

/// @note This method writes to the filesystem.
///
/// Creates the referenced directory non-recursively,
/// i.e. parent directories are not created. Trying
/// to create a directory below a nonexistant directory
/// will result in an ErrnoError exception.
///
/// @remark UNIX note: The directory is created with RWX permissions
/// for everyone, but filtered by your current `umask` before applied
/// to disk.
///
/// @see mktree()
zs::error_result path_mkdir(const zs::string& path);

/// @note This method writes to the file system.
///
/// This method provides a functionality akin to the UNIX `mkdir -p`
/// command, i.e. it creates the referenced directory, and if necessary,
/// also creates all parent directories. Note this method does not
/// throw an ErrnoError if the referenced directory already exists;
/// it just does nothing.
///
/// @see mkdir()
zs::error_result path_mktree(const zs::string& path);

/// @note This method writes to the filesystem.
///
/// Deletes the referenced file. This cannot be used to
/// delete a directory rather than a file.
///
/// @see remove() rmdir()
zs::error_result path_unlink(const zs::string& path);

/// @note This method writes to the filesystem.
///
/// Deletes the referenced directory, which is required
/// to be empty, if not, an ErrnoError will be thrown.
///
/// This cannot be used to delete a file rather than a
/// directory.
///
/// @see remove() unlink()
zs::error_result path_rmdir(const zs::string& path);

/// @note This method writes to the filesystem.
///
/// This method, which is akin to the UNIX "rm -r" command, removes
/// the entire referenced directory hierarchy recursively, including
/// any files and directories contained therein.
zs::error_result path_rmtree(const zs::string& path);

/// @note This method writes to the file system.
///
/// Renames a file to another name without involving file streams.
///
/// @param[in] newname The new name of the file.
zs::error_result path_rename(const zs::string& path, const zs::string& new_path);

zs::error_result path_copy(const zs::string& path, const zs::string& new_path);

zs::error_result path_remove(const zs::string& path);

/// @note This method accesses the file system.
///
/// Returns the file size.
int_t path_size(const zs::string& path);

/**
 * Returns the number of components in the path string, or
 * in different words, counts the slashes and adds one for
 * the last element, except if the path is just the root
 * (see is_root()).
 *
 * The return value of this method minus one is the last
 * possible index for operator[].
 */
size_t path_component_count(zb::string_view path);

/// @note This method accesses the file system.
///
/// Returns the file’s last access time. The value is not
/// really reliable.
time_t path_access_time(const zs::string& path);

/// @note This method accesses the file system.
///
/// Returns the file’s last modification time.
time_t path_modified_time(const zs::string& path);

/// @note This method accesses the file system.
///
/// Returns the file’s creation time.
time_t path_creation_time(const zs::string& path);
} // namespace zs::path_library.
