#include "ztests.h"

ZS_CODE_TEST("class_01", R"""(
 class a {
 john = function() {
  }

  function peter() {
  print("JOHNSON");
  }

  constructor() {
  }
};

return a;
)""") {
  //  zb::print(value);
  REQUIRE(value.is_class());

  //  zb::print(value.as_class());

  zs::var inst = value.as_class().create_instance();
  REQUIRE(inst.is_instance());

  zs::var dest;
  REQUIRE(!vm->get(inst, zs::_ss("peter"), dest));
}

ZS_CODE_TEST("class_02", R"""(
 var a = class {
 john = function() {
  }

  function peter() {
  }

  constructor() {
  }
};

return a;
)""") {
  //  zb::print(value);
  REQUIRE(value.is_class());

  //  zb::print(value.as_class());
}

ZS_CODE_TEST("class_03", R"""(
var b = {};
 class b.a {
 john = function() {
  }

  function peter() {
  }

  constructor() {
  }
};

return b.a;
)""") {
  //  zb::print(value);
  REQUIRE(value.is_class());

  //  zb::print(value.as_class());
}

ZS_CODE_TEST("class_04", R"""(
class a {
  constructor(v) {
    //print("constructor", v, base, this);
    this.banana = v;
  }

  john = function() {
  }

  function peter() {
    //print("JOHNSON");
  }
};

var i = a(32);
return i;
)""") {
  REQUIRE(value.is_instance());
  //  zb::print(value.as_instance());
  //  zb::print(value.as_instance().get_class().as_class());
}

ZS_CODE_TEST("ZS_DOC_GENERATOR", R"""(
var doc = {
  sections = [
    {
      title = "Section Title"
    },
    {
      title = "Section Title 2"
    }
  ],
};

//print(doc);

return doc;
)""") {
  REQUIRE(value.is_table());
  //  zb::print(value.as_instance());
  //  zb::print(value.as_instance().get_class().as_class());
}
