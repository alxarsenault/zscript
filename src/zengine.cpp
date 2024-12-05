#include <zscript.h>
#include <zbase/sys/path.h>

namespace zs {
namespace {
  struct engine_pimpl_proxy_tag {};
} // namespace.

template <>
struct internal::proxy<engine_pimpl_proxy_tag> {

  enum class objects { registry, import_directories, count };
  using enum objects;

  using objects_array = std::array<zs::object, (size_t)objects::count>;
  static_assert(sizeof(objects_array) == sizeof(engine::_objects));

  template <objects Obj>
  static inline object& get_object(const engine* eng) {
    return *((object*)(eng->_objects.data() + constants::k_object_size * (size_t)Obj));
  }

  static inline void init_objects(engine* eng) {
    zb_placement_new(eng->_objects.data()) objects_array();
    get_object<import_directories>(eng) = zs::_a(eng, 0);
    get_object<registry>(eng) = zs::_t(eng);
  }

  static inline void destroy_objects(engine* eng) {
    ((objects_array*)eng->_objects.data())->~objects_array();
  }
};

namespace {
  using engine_proxy = internal::proxy<engine_pimpl_proxy_tag>;
} // namespace.

engine::engine(allocate_t alloc_cb, raw_pointer_t user_pointer, raw_pointer_release_hook_t user_release,
    stream_getter_t stream_getter, engine_initializer_t initializer)
    : _allocator(alloc_cb)
    , _user_pointer(user_pointer)
    , _user_pointer_release(user_release)
    , _stream_getter(stream_getter)
    , _initializer(initializer) //
    ZS_IF_GARBAGE_COLLECTOR(, _gc(this)) {

  engine_proxy::init_objects(this);

  if (_initializer) {
    _initializer(this);
  }
}

engine::~engine() {

  engine_proxy::destroy_objects(this);

  ZS_IF_GARBAGE_COLLECTOR(_gc.finalize());

  if (_user_pointer_release) {
    (*_user_pointer_release)(_allocator, _user_pointer);
  }

  _user_pointer = nullptr;
  _user_pointer_release = nullptr;

  ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(zbase_warning(
      _global_ref_count == 0, "Invalid reference count (", _global_ref_count, ") should be zero"));
}

void* engine::allocate(size_t size, alloc_info_t ainfo) {
  zbase_assert(_allocator, "invalid allocator callback");
  return (*_allocator)(this, _user_pointer, nullptr, size, 0, ainfo);
}

void* engine::reallocate(void* ptr, size_t size, size_t old_size, alloc_info_t ainfo) {
  zbase_assert(_allocator, "invalid allocator callback");
  return (*_allocator)(this, _user_pointer, ptr, size, old_size, ainfo);
}

void engine::deallocate(void* ptr, alloc_info_t ainfo) {
  zbase_assert(_allocator, "invalid allocator callback");
  (*_allocator)(this, _user_pointer, ptr, 0, 0, ainfo);
}

table_object& engine::get_registry_table_object() noexcept {
  return engine_proxy::get_object<engine_proxy::registry>(this).as_table();
}

object& engine::get_registry_table() noexcept {
  return engine_proxy::get_object<engine_proxy::registry>(this);
}

const object& engine::get_registry_table() const noexcept {
  return engine_proxy::get_object<engine_proxy::registry>(this);
}

object engine::get_registry_object(const zs::object& name) const noexcept {
  auto& tbl = engine_proxy::get_object<engine_proxy::registry>(this).as_table();
  auto it = tbl.find(name);
  return it == tbl.end() ? nullptr : it->second;
}

object engine::get_registry_object(std::string_view name) const noexcept {
  auto& tbl = engine_proxy::get_object<engine_proxy::registry>(this).as_table();
  auto it = tbl.find(zs::_sv(name));
  return it == tbl.end() ? nullptr : it->second;
}

void engine::set_user_pointer(raw_pointer_t uptr) { _user_pointer = uptr; }

void engine::set_stream_getter(stream_getter_t stream_getter) { _stream_getter = stream_getter; }

zs::error_result engine::add_import_directory(const std::filesystem::path& directory) {

  std::filesystem::path canonical_path;

  if (zs::error_result err
      = ([](const std::filesystem::path& directory, std::filesystem::path& canonical_path) {
          try {

            std::error_code ec;
            canonical_path = std::filesystem::weakly_canonical(directory, ec);

            if (ec) {
              return zs::error_code::invalid_include_file;
            }
          } catch (const std::filesystem::filesystem_error& e) {
            zbase_error(e.what());
            return zs::error_code::invalid_include_file;
          }

          return zs::error_code::success;
        }(directory, canonical_path))) {
    canonical_path = directory;
  }

  std::string_view ipath;
  zs::string strbuffer((zs::string_allocator(this, memory_tag::nt_engine)));
  if constexpr (std::is_same_v<std::filesystem::path::value_type, char>) {
    ipath = canonical_path.native();
  }
  else {
    strbuffer = canonical_path.generic_string<char, std::char_traits<char>, zs::string_allocator>(
        zs::string_allocator(this, memory_tag::nt_engine));
    ipath = std::string_view(strbuffer);
  }

  if (zb::sys::path_ref(ipath.data()).has_extension()) {
    return zs::error_code::invalid_include_directory;
  }

  zs::array_object& dirs = engine_proxy::get_object<engine_proxy::import_directories>(this).as_array();
  if (dirs.contains(ipath)) {
    return zs::error_code::already_exists;
  }

  dirs.emplace_back(this, ipath);
  return {};
}

std::ostream& engine::get_stream() { return _stream_getter(this, _user_pointer); }

zs::error_result engine::resolve_file_path(std::string_view import_value, object& result) {

  using path_type = zb::sys::path<zs::allocator<char>>;

  path_type fpath(import_value, this);
  zs::small_vector<path_type, 4> paths((this));

  std::string_view ext = fpath.extension();

  if (ext == ".zs") {
    paths.push_back(fpath.with_extension(".zsc"));
  }
  else {

    paths.push_back(fpath + ".zsc");
    paths.push_back(fpath + ".zs");

    if (fpath.string().contains('.')) {
      zs::string fpath_str = fpath.string();
      std::replace(fpath_str.begin(), fpath_str.end(), '.', '/');

      paths.push_back(fpath_str + ".zsc");
      paths.push_back(fpath_str + ".zs");
    }
  }

  paths.push_back(std::move(fpath));

  for (const auto& p : paths) {
    if (p) {
      result = zs::_s(this, p.string());
      return {};
    }
  }

  const zs::array_object& dirs = engine_proxy::get_object<engine_proxy::import_directories>(this).as_array();

  const int_t dirs_count = dirs.size();

  for (size_t i = 0; i < dirs_count; i++) {
    const path_type dirpath(dirs[i].get_string_unchecked(), this);
    for (const auto& p : paths) {
      if (path_type abs_path = dirpath.join(p)) {
        result = zs::_s(this, abs_path.string());
        return {};
      }
    }
  }

  result = zs::_ss("");
  return zs::errc::not_found;
}

garbage_collector::garbage_collector(zs::engine* eng)
    : zs::engine_holder(eng)
    , _objs(zb::aligned_type_storage_construct_tag{}, (zs::allocator<zs::reference_counted_object*>(eng))) {}

void garbage_collector::finalize() {
  {
    zs::vector<zs::object> objs((zs::allocator<zs::object>(_engine)));
    objs.reserve(_objs.get().size());

    for (auto it : _objs.get()) {
      objs.emplace_back(it, true);
    }

    for (auto& obj : objs) {
      if (obj.is_table()) {
        obj.as_table().clear();
      }
      else if (obj.is_array()) {
        obj.as_array().clear();
      }
      else if (obj.is_closure()) {
        obj.as_closure().clear();
      }
    }
  }

  _objs.destroy();
}

void garbage_collector::add(zs::engine* eng, zs::reference_counted_object* obj) {
  ZS_IF_GARBAGE_COLLECTOR(eng->_gc._objs.data()->insert(obj));
}

void garbage_collector::remove(zs::engine* eng, zs::reference_counted_object* obj) {
  ZS_IF_GARBAGE_COLLECTOR(eng->_gc._objs.data()->erase(obj));
}
} // namespace zs.
