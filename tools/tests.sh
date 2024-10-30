#!/usr/bin/env bash

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
ROOT_DIR=$(realpath "$SCRIPT_DIR/..")
APP_PATH="$ROOT_DIR/output/zscript"
DOC_PATH="$ROOT_DIR/doc"
DOC_FILE="$DOC_PATH/src/gendoc.zs"

echo $APP_PATH $DOC_FILE
# EXAMPLES_DIR="$SCRIPT_DIR/../../examples"

# for f in $EXAMPLES_DIR/*.zs; do
#   $APP_PATH "$f"
# done


# zscript/doc/src

# $APP_PATH "$EXAMPLES_DIR/example_01.zs"
# $APP_PATH "$EXAMPLES_DIR/example_02.zs"
$APP_PATH "$DOC_FILE" -D ZSCRIPT_DOC_DIRECTORY="$DOC_PATH"
