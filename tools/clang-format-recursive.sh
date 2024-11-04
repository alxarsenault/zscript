#!/bin/bash

if [ "$#" -eq 0 ]; then
  echo "No input"
  exit 0
fi

# Recursive directory.
if [[ "$#" -eq 1 && -d $1 ]]; then
  FILES=$(find $1 | grep -E ".*(\.cpp|\.c|\.cc|\.h|\.hpp|\.hh|\.mm)$")

  for f in $FILES; do
    if [ "$(basename "$f")" != "catch2.h" ] && [ "$(basename "$f")" != "xxhash.h" ]; then
      TRIMMED=$(dirname "$f")"/"$(basename "$f")
      clang-format --verbose -i "${TRIMMED}"
    fi
  done

  exit 0
fi

# Single file.
if [[ "$#" -eq 1 && -f $1 ]]; then
  clang-format --verbose -i "$1"
  exit 0
fi

# Invalid file input.
if [[ "$#" -eq 1 ]]; then
  echo "Input file doesn't exists"
  exit 0
fi

# Multiple files input.
for f in $@; do
  clang-format --verbose -i "${f}"
done
