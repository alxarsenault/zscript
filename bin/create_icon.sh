#!/bin/bash


if [[ -z "$1" ]]
then
  echo "Missing input file path"
  exit 1
fi

OUTPUT_ICONSET_FILE="icon.iconset"
INPUT_FILE="$1"

mkdir "$OUTPUT_ICONSET_FILE"

magick "$INPUT_FILE" -resize 16x16 icon.iconset/icon_16x16.png
magick "$INPUT_FILE" -resize 32x32 icon.iconset/icon_16x16@2x.png
magick "$INPUT_FILE" -resize 32x32 icon.iconset/icon_32x32.png
magick "$INPUT_FILE" -resize 64x64 icon.iconset/icon_32x32@2x.png
magick "$INPUT_FILE" -resize 128x128 icon.iconset/icon_128x128.png
magick "$INPUT_FILE" -resize 256x256 icon.iconset/icon_128x128@2x.png
magick "$INPUT_FILE" -resize 256x256 icon.iconset/icon_256x256.png
magick "$INPUT_FILE" -resize 512x512 icon.iconset/icon_256x256@2x.png
magick "$INPUT_FILE" -resize 512x512 icon.iconset/icon_512x512.png
magick "$INPUT_FILE" -resize 1024x1024 icon.iconset/icon_512x512@2x.png

iconutil -c icns "$OUTPUT_ICONSET_FILE"
rm -rf "$OUTPUT_ICONSET_FILE"