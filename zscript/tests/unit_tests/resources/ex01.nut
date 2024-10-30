local p = project("Peter");


local dir = current_directory();
printf("%s\n", dir);

local src = list_directory(dir, "*.nut");

for(local i = 0; i < src.len(); i++) {
  printf("%d : %s\n", i, src[i]);
}

local lib = p.create_library("wplib");