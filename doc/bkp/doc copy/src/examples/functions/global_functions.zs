// Option 1.
global function main(var args) { return 0; }

// Le `var` n'est pas oubligatoire ici.
global function main(args) { return 0; }

// Option 2.
function global::main(var args) { return 0; }

// Option 3.
function global.main(var args) { return 0; }

// Option 4.
function ::main(var args) { return 0; }

// Option 5.
// Celui la va probablement toujours fonctionner.
global.main = function(var args) { return 0; }
