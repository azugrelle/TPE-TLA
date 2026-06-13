#! /bin/bash

set -euo pipefail

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

INPUT="$1"
shift 1
LOGGING_LEVEL="WARNING" ".build/Flex-Bison-Compiler" "$@" < "$INPUT"
