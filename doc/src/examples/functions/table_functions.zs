// Option 1.
var t1 = {
  // Ceci est une fonction dans une table.
  my_func = function() { return 0; }
};

// Option 2.
var t2 = {
  // Ceci est aussi une fonction dans une table.
  function my_func() { return 0; }
};

local t3 = {};

// Option 3.
t3.my_func = function() { return 0; };

// Option 4 (Je ne suis pas certain entre 4 et 5).
function t3.my_func() { return 0; };

// Option 5.
function t3::my_func() { return 0; };

