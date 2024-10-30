local uuid = {};

uuid.gen <- function() {
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

return uuid;