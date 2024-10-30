#!/bin/bash

if [ "$1" ]; then
  if [ "$1" = "clean" ]; then
    rm -rf build-xcode
  fi
fi

if [ "$2" ]; then
  if [ "$2" = "clean" ]; then
    rm -rf build-xcode
  fi
fi

mkdir -p build-xcode
cd build-xcode
cmake -GXcode ..

if [ "$1" ]; then
  if [ "$1" = "open" ]; then
    open zs.xcodeproj
  fi
fi

if [ "$2" ]; then
  if [ "$2" = "open" ]; then
    open zs.xcodeproj
  fi
fi
