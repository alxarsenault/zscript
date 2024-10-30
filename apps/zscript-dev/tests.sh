#!/usr/bin/env bash

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
APP_PATH=$(realpath "$SCRIPT_DIR/../../../../build-xcode/projects/zscript/apps/zscript-app/Debug/zscript")

EXAMPLES_DIR="$SCRIPT_DIR/../../examples"

# for f in $EXAMPLES_DIR/*.zs; do
#   $APP_PATH "$f"
# done

# $APP_PATH "$EXAMPLES_DIR/example_01.zs"
# $APP_PATH "$EXAMPLES_DIR/example_02.zs"
$APP_PATH "$EXAMPLES_DIR/gendoc.zs" -D ZSCRIPT_DOC_DIRECTORY="/Users/alexarse/Develop/wp/projects/zscript/doc2"
