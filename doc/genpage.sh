#!/usr/bin/env bash

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
ROOT_DIR=$(realpath "$SCRIPT_DIR/..")
APP_PATH="$ROOT_DIR/output/zscript"
DOC_PATH="$ROOT_DIR/doc"
DOC_FILE="$DOC_PATH/src/genpage.zs"

# Execute.
$APP_PATH "$DOC_FILE"
