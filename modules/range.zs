@module     "range"
@brief      "Range struct."
@author     "Alexandre Arsenault"
@version    "1.0.0"
@date       "2024-12-10"
@copyright  "Meta-Sonic"

struct range {
  constructor() = default;

  constructor(number s, number e) {
    if(s <= e) {
      this.start = s;
      this.end = e;
    }
    else {
      this.start = e;
      this.end = s;
    }
  }

  constructor(range r) {
    this.start = r.start;
    this.end = r.end;
  }

  constructor(array arr) {
    this.start = arr[0];
    this.end = arr[1];
  }

  constructor(table tbl) {
    this.start = tbl.start;
    this.end = tbl.end;
  }

  const function length() {
    return this.end - this.start;
  }

  const function middle() {
    return this.start + (this.end - this.start) * 0.5;
  }

  const function is_symmetric() {
    return this.start == -this.end;
  }

  const function is_sorted() {
    return this.start <= this.end;
  }

  const function contains(var<number | range> x) {
    if(typeof(x) == "range") {
      return x.start >= this.start && x.start <= this.end &&
             x.end >= this.start && x.end <= this.end;
    }

    return x >= this.start && x <= this.end;
  }

  const function is_strictly_inside(number x) {
    return x > this.start && x < this.end;
  }

  const function is_strictly_inside_or_start(number x) {
    return x >= this.start && x < this.end;
  }

  const function is_strictly_inside_or_end(number x) {
    return x > this.start && x <= this.end;
  }

  const function clip_value(number x) {
    return x > this.end ? this.end : x < this.start ? this.start : x;
  }

  const function with_length(number len) {
    return range(this.start, this.start + len);
  }

  const function with_start(number s) {
    return range(s, this.end);
  }

  const function with_end(number e) {
    return range(this.start, e);
  }

  const function with_shifted_start(number delta) {
    return range(this.start + delta, this.end);
  }

  const function with_shifted_end(number delta) {
    return range(this.start, this.end + delta);
  }

  /// Set the length starting from start.
  function set_length(number len) {
    this.end = this.start + len;
    return this;
  }

  /// Shift start and end by delta.
  function shift(number delta) {
    this.start += delta;
    this.end += delta;
    return this;
  }

  /// Shift start only by delta.
  function shift_start(number delta) {
    this.start += delta;
    return this;
  }

  /// Shift end only by delta.
  function shift_end(number delta) {
    this.end += delta;
    return this;
  }

  function sort() {
    if(this.start > this.end) {
      var s = this.start;
      this.start = this.end;
      this.end = s;
    }

    return this;
  }

  function move_to(number s) {
    var len = this.end - this.start;
    this.start = s;
    this.end = s + len;
    return this;
  }

  number start = 0;
  number end = 0;
};

return range;