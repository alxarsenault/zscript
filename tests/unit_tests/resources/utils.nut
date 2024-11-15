local utils = {};

if (!::rawin("std")) {
	::std <- {};
}

local std = ::std;


utils.print <-  function(...) {
	::print(vargv.reduce(function(previousValue, currentValue) {
		return (previousValue + " " + currentValue);
	}) + "\n");
}

utils.print_array <-  function(arr) {
	for (local i = 0; i < arr.len(); i++) {
		::print(i.tostring() + " : " + arr[i].tostring() + "\n");
	}
}


/**
 * Convert a hex string (with or without '0x' prefix) to an integer.
 *
 * @param {string} hs - The hex string
 *
 * @returns {integer} The value of the hex string
 *
 */
utils.hexStringToInteger <-  function(hs) {
	// Check input string type
	if (typeof hs != "string") throw "utilities.hexStringToInteger() requires a string";
	hs = hs.tolower();
	if (hs.slice(0, 2) == "0x") hs = hs.slice(2);
	local i = 0;
	foreach(c in hs) {
		local n = c - '0';
		if (n > 9) n = ((n & 0x1F) - 7);
		i = (i << 4) + n;
	}
	return i;
}

/**
 * Convert a decimal integer into a hex string.
 *
 * @param {integer} i   - The integer
 * @param {integer} [n] - The number of characters in the hex string. Default: 2
 *
 * @returns {string} The hex string representation
 *
 */
utils.integerToHexString <-  function(i, n = 2) {
	if (typeof i != "integer") throw "utilities.integerToHexString() requires an integer";
	if (n % 2 != 0) n++;
	local fs = "0x%0" + n.tostring() + "x";
	return format(fs, i);
}

utils.uuid <- function() {
  // Randomize 16 bytes (128 bits)
  local rnds = blob(16);
  for (local i = 0 ; i < 16 ; i++) rnds.writen((256.0 * math.rand() / math.RAND_MAX).tointeger(), 'b');

  // Adjust certain bits according to RFC 4122 section 4.4
  rnds[6] = 0x40 | (rnds[6] & 0x0F);
  rnds[8] = 0x80 | (rnds[8] & 0x3F);

  // Create an return the UUID string
  local s = "";
  for (local i = 0 ; i < 16 ; i++) {
      s = s + format("%02X", rnds[i]);
      if (i == 3 || i == 5 || i == 7 || i == 9) s = s + "-";
  }
  return s;
}

std.print <- utils.print;
std.print_array <- utils.print_array;

return utils;