local utils = {
  function print(...) {
    ::print(vargv.reduce(function(previousValue, currentValue) {
      return (previousValue + " " + currentValue);
    }) + "\n");
  }
}

assert((3.2 ^ 2.1) == 11.503101568235094);
assert((3.2 ^ (1.1 + 1.0)) == 11.503101568235094);
assert((1 ^ 2 ^ 3) == 1);
assert((2 ^ (2 ^ 3)) == 256);
assert(((2 ^ 2) ^ 3) == 64);
assert((2 ^ 2 ^ 3) == 256);

{
  local v1 = 3.2;
  v1 ^= 2.1;
  assert(v1 == 11.503101568235094);
}

local tbase = {
  fct _exp(obj) {
    return 32;
  }
};

local a = {};
local b = {};
a.setdelegate(tbase);

local c = a ^ b;
assert(c == 32);

local k1 = 0;
local k2 = 1;
local k3 = k2 << k1;

class P {

};


// print(v6+"\n");
// return a;
// class mat {
//   _dims = null;
//   _data = null;

//   constructor(...) {
//     this._dims = vargv;

//     local n = 1;

//     for(local i = 0; i < vargv.size(); i++) {
//       n *= vargv[i];
//     }

//     utils.print("N", n);
//     this._data = ::array(n);
//   }

//   fct size() {
//     return _data.size();
//   }

//   fct depth() {
//     return _dims.size();
//   }

//   // function _get(idx) {

//   //   utils.print("Get", idx);
//   //   return this._data[idx];
//   //   // if(idx < this._data.size()) {
//   //   //   return this._data[idx];
//   //   // }

//   //   // if(idx == "_data") {
//   //   //   return this.rawget("_data");
//   //   // }

//   //   // if (idx < this.rows.size()) {
//   //   //   return this.rows[idx];
//   //   // }

//   //   // return this.rawget(idx);
//   //   // throw(null);
//   // }

//   // function _set(idx, val) {
//   //   utils.print("Set");

//   //   if(idx == "_data") {
//   //     // ::rawset("_data", value);
//   //   }

//   //   // if (idx < this.rows.size()) {
//   //   //   this.rows[idx] = value;
//   //   // }
//   // }

//   // function _newslot(key, value) {
//   //   utils.print("_newslot");
//   //   // if(key == "_data") {
//   //   //   this.rawset("_data", value);
//   //   // }
//   // }
// };

// local m1 = mat(2, 2);
// utils.print("SIZE", m1.size());
// utils.print("DEPTH", m1.depth());
// local a = m1["0"];
