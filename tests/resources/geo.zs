/// @module geo

struct Point {
 var x = 0;
 var y = 0;

 constructor(var x = 0, var y = 0) {
  this.x = x;
  this.y = y;
 }
};

struct Size {
 var width = 0;
 var height = 0;

 constructor(var w = 0, var h = 0) {
  this.width = w;
  this.height = h;
 }
};

function CreateSize(w, h) {
    return Size(w, h);
}

struct Rect {
 var x = 0;
 var y = 0;
 var width = 0;
 var height = 0;

 constructor(var x = 0, var y = 0, var w = 0, var h = 0) {
  this.x = x;
  this.y = y;
  this.width = w;
  this.height = h;
 }

 function size() {
  return CreateSize(width, height);
 }
};

return {
  Point = Point,
  Size = Size,
  Rect = Rect
};