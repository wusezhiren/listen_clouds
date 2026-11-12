#!/usr/bin/env bash
set -euo pipefail
root="$(dirname "$(dirname "$(realpath "$0")")")"
json="${CJSON_SOURCE_DIR:-$root/../gateway/managed_components/espressif__cjson/cJSON}"
if [[ ! -f "$json/cJSON.c" ]]; then
  printf 'Set CJSON_SOURCE_DIR to a directory containing cJSON.c and cJSON.h\n' >&2
  exit 1
fi
build="$(mktemp -d)"
trap 'rm -rf "$build"' EXIT
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I"$root/include" -I"$json" "$root/tests/protocol_test.c" \
  "$root/src/protocol.c" "$json/cJSON.c" -o "$build/protocol_test"
"$build/protocol_test"
