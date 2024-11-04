
const values_key = "__values";
const getters_key = "__getters";
const setters_key = "__setters";

const tbl_default = zs::get_table_default_delegate();

var property_object_delegate = {
    __get = function(key, d) {
        // zs::print("*access to element", key);

        if(zs::is_string(key)) {
            if(key.starts_with("get_")) {
                var pname = key.replace_first("get_", "");

                if(zs::contains(this[getters_key], pname)) {
                    return this[getters_key][pname];
                }

                return ()=> { return this[values_key][key]; };
            }

            if(zs::contains(this[getters_key], key)) {
                return this[getters_key][key]();
            }
        }

        // zs::print("*access to original", key);
        return this[values_key][key];
    }
};

//function default_setter(obj, property_name, value) {
//  //zs::print("SET");
//}


//function default_setter(property_name, value) {
//}

function create_property_object(properties) {
  var names = zs::get_table_keys(properties);

  zs::set_delegate(properties, none, false);

  var pobj = {
    [values_key] = properties,
    [getters_key] = {},
    [setters_key] = {},
  };

  return zs::set_delegate(pobj, property_object_delegate);
}

var pobj = create_property_object({
  a = 10,
  b = 20,
  c = 30
});

pobj[getters_key]["a"] = function() {
  // zs::print("ONLUDHDJS");
  return pobj[values_key]["a"];
}

//zs::print("VALUE", pobj.get_a());
//zs::print("VALUE A", pobj.a);
//zs::print("VALUE A", pobj.b);
//zs::print(pobj.a);
//zs::print(zs::get_table_keys({a = 32, b = 43}));
//var t = {
//  a = 678
//};

return pobj;