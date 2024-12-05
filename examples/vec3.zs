@module vector

const math = import("math");

struct vec3 {
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;

  constructor() = default;

  constructor(number x, number y, number z) {
    this.x = zs::to_float(x);
    this.y = zs::to_float(y);
    this.z = zs::to_float(z);
  }

  constructor(const var<vec3> v) {
    this.x = v.x;
    this.y = v.y;
    this.z = v.z;
  }

  constructor(number v) {
    this.x = v;
    this.y = v;
    this.z = v;
  }

  const function cross(const var<vec3> v) {
    return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  const function dot(const var<vec3> v) {
    return x * v.x + y * v.y + z * v.z;
  }

  const function norm() {
    return math.sqrt(x * x + y * y + z * z);
  }

  const function normalized() {
    var n = norm();
    return vec3(x / n, y / n, z / n);
  }

  const function proj(const var<vec3> v) {
    const nv = normalized();
    return nv * nv.dot(v);
  }

  const function reflect(const var<vec3> v) {
    return this - (this - v.proj(this)) * 2.0;
  }

  const function __add(const var<vec3, number> v) {
    if(zs::is_number(v)) {
      return vec3(x + v, y + v, z + v);
    }
    else if(typeof(v) == "vec3") {
      return vec3(x + v.x, y + v.y, z + v.z);
    }

    return null;
  }

  const function __sub(const var<vec3, number> v) {
    if(zs::is_number(v)) {
      return vec3(x - v, y - v, z - v);
    }
    else if(typeof(v) == "vec3") {
      return vec3(x - v.x, y - v.y, z - v.z);
    }

    return null;
  }

  const function __mul(const var<vec3, number> v) {
    zs::print("vec3 __mul");
    if(zs::is_number(v)) {
      return vec3(x * v, y * v, z * v);
    }
    else if(typeof(v) == "vec3") {
      return vec3(x * v.x, y * v.y, z * v.z);
    }

    return null;
  }

  function __mul_eq(const var<vec3, number> v) {
    zs::print("vec3 __mul_eq", this, v);
    if(zs::is_number(v)) {
      this.x *= v;
      this.y *= v;
      this.z *= v;
      return this;
    }
    else if(typeof(v) == "vec3") {
      this.x *= v.x;
      this.y *= v.y;
      this.z *= v.z;
      return this;
    }

    return null;
  }

  const function __div(const var<vec3, number> v) {
    if(zs::is_number(v)) {
      return vec3(x / v, y / v, z / v);
    }
    else if(typeof(v) == "vec3") {
      return vec3(x / v.x, y / v.y, z / v.z);
    }

    return null;
  }

  function __add_eq(const var<vec3, number> v) {
    zs::print("vec3 __add_eq");
    if(zs::is_number(v)) {
      this.x += v;
      this.y += v;
      this.z += v;
      return this;
    }
    else if(typeof(v) == "vec3") {
      this.x += v.x;
      this.y += v.y;
      this.z += v.z;
      return this;
    }

    return null;
  }
};

export vec3;