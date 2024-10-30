
if(! ::rawin("std")) {
 ::std <- {};
}
//

local std = ::std;

// Path.
class std.path {

	filepath = "";

	constructor(fpath, ...) {
		this.filepath = fpath;

		for (local i = 0; i < vargv.len(); i++) {
			if (!endswith(this.filepath, "/")) {
				this.filepath += "/";
			}

			this.filepath += vargv[i];
		}
	}

	static is_instance = function(p) {
		return p instanceof(std.path);
	}

	function _div(spath) {
		return std.path(this.filepath + "/" + spath);
	}

	function extension() {
		return wp_path_get_extension(this.filepath);
	}

	function root_directory() {
		return wp_path_get_root_directory(this.filepath);
	}

	function parent_path() {
		return wp_path_get_parent_path(this.filepath);
	}

	function filename() {
		return wp_path_get_filename(this.filepath);
	}

	function stem() {
		return wp_path_get_stem(this.filepath);
	}

	function relative_path() {
		return wp_path_get_relative_path(this.filepath);
	}

	function is_absolute() {
		return wp_path_is_absolute(this.filepath);
	}

	function _tostring() {
		return this.filepath;
	}
};
// ::std <- std;
return std;