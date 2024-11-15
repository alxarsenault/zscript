// local utils = dofile("/Users/aarsenault/develop/wp/modules/script/tests/resources/utils.nut")
// local utils = import("/Users/aarsenault/develop/wp/modules/script/tests/resources/utils.nut");
local utils = import("utils");
local path = import("path");

local function path_to_string(p) {
	if (type(p) == "instance" && path.path.is_instance(p)) {
		return p.tostring();
	}

	return p;
}

function get_current_source_path() {
	return wp_fs_get_current_source_path();
}

function get_current_source_directory() {
	return wp_fs_get_current_source_directory();
}

function get_working_directory() {
	return wp_fs_get_working_directory();
}

function create_directory(dir) {
	dir = path_to_string(dir);
	return wp_fs_create_directory(dir);
}

function list_directory(dir, exts = null) {

	dir = path_to_string(dir);
	// if (path.is_instance(dir)) {
	// 	dir = dir.tostring();
	// }

	if (type(dir) != "string") {
		error("Wrong parameter");
	}

	return wp_fs_list_directory(dir, exts);
}



// Target.
class target {
	name = "";
	sources = [];

  _include_directories = [];
	_private_include_directories = [];

  _link_directories = [];
	_private_link_directories = [];

  _link_libraries = [];
  _private_link_libraries = [];

	constructor(name, sources = []) {
		this.name = name;
		this.sources = sources;
	}

	function include_directory(dir, private = false) {
		dir = path_to_string(dir);

		if (type(dir) != "string") {
			error("Wrong parameter");
		}

		local dir_list = private ? this._private_include_directories : this._include_directories;

		if (dir_list.find(dir) == null) {
			dir_list.append(dir);
		}
	}

	function include_directories(dirs, private = false) {
		dirs = path_to_string(dirs);

		local dtype = type(dirs);

		if (dtype == "string") {
			include_directory(dirs, private);
		} else if (dtype == "array") {
			for (local i = 0; i < dirs.len(); i++) {
				include_directory(dirs[i], private);
			}
		} else {
			error("Wrong parameter");
		}
	}

	//
	function link_directory(dir, private = false) {
		dir = path_to_string(dir);

		if (type(dir) != "string") {
			error("Wrong parameter");
		}

		local dir_list = private ? this._private_link_directories : this._link_directories;
		if (dir_list.find(dir) == null) {
			dir_list.append(dir);
		}
	}

  //
	function link_directories(dirs, private = false) {
		dirs = path_to_string(dirs);

		local dtype = type(dirs);

		if (dtype == "string") {
			link_directory(dirs, private);
		} else if (dtype == "array") {
			for (local i = 0; i < dirs.len(); i++) {
				link_directory(dirs[i], private);
			}
		} else {
			error("Wrong parameter");
		}
	}




  //
	function link_library(lib, private = false) {


		if (type(lib) != "string") {
			error("Wrong parameter");
		}

		local lib_list = private ? this._private_link_libraries : this._link_libraries;
		if (lib_list.find(lib) == null) {
			lib_list.append(lib);
		}
	}

  //
	function link_libraries(libs, private = false) {
		local dtype = type(libs);

		if (dtype == "string") {
			link_library(libs, private);
		} else if (dtype == "array") {
			for (local i = 0; i < libs.len(); i++) {
				link_library(libs[i], private);
			}
		} else {
			error("Wrong parameter");
		}
	}
};


// Library.
class library extends target {
	constructor(name, sources = []) {
		base.constructor(name, sources);
		printf("Library name: %s\n", this.name);
		utils.print_array(this.sources);
	}
};

// Executable.
class executable extends target {
	constructor(name, sources = []) {
		base.constructor(name, sources);
		printf("Executable name: %s\n", this.name);
		utils.print_array(this.sources);
	}
};

// Project.
class project {
	name = "";
	_project_path = "";
	_project_directory = "";
	_executables = [];
	_libraries = [];

	constructor(name) {
		this.name = name;
		this._project_path = wp_fs_get_calling_source_path();
		this._project_directory = wp_fs_get_calling_source_directory();
		printf("Project name: %s\n", this.name);

		// printf("Project name: %s\n", wp_fs_get_calling_source_path());

	}

	// function path() {
	// 	return this._project_path;
	// }

	function directory() {
		return path.path(this._project_directory);
	}

	function create_library(name, sources = []) {
		return library(name, sources);
	}

	function create_executable(name, sources = []) {
		local exe = executable(name, sources);
		this._executables.append(exe);
		return exe;
	}

	function generate_executable(exe) {
		utils.print(exe.name);

		local build_directory = this.directory() / "build";
		local makefile_path = build_directory / "compile.json";


		wp_project_generate_executable(build_directory.tostring(), makefile_path.tostring(), exe);

		// local res = create_directory(build_directory);
		// local myfile = file((build_directory / "Makefile").tostring(), "w+");


		// local data = blob();
		// data.writestring("Alex\n");

		// myfile.writeblob(data);

		// myfile.close();

	}

	function generate() {

		for (local i = 0; i < this._executables.len(); i++) {
			generate_executable(this._executables[i]);
		}
	}
};