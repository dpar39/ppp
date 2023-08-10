#!/usr/bin/env bash
set -e

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $REPO_ROOT/dock.sh "$*"

# Avoid modifying the PATH variable and LD_LIBRARY_PATH
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

TARGET=$1
if [[ "$TARGET" == "compdb" ]]; then
    bazel --output_user_root=$REPO_ROOT/.bazel run :refresh_compile_commands -- -c dbg
elif [[ "$TARGET" == "native:debug" ]]; then
    bazel --output_user_root=$REPO_ROOT/.bazel build -c dbg //:ppp_test
elif [[ "$TARGET" == "native:release" ]]; then
    bazel --output_user_root=$REPO_ROOT/.bazel build -c opt //:ppp_test
elif [[ "$TARGET" == "wasm:debug" ]]; then
    bazel --output_user_root=$REPO_ROOT/.bazel build -c dbg //:ppp-wasm-wrap --config=wasm
elif [[ "$TARGET" == "wasm:release" ]]; then
    bazel --output_user_root=$REPO_ROOT/.bazel build -c opt //:ppp-wasm-wrap --config=wasm
else
    _fail_ 'Usage: ./build.sh [compdb|native:debug|native:release|wasm:debug|wasm:release]'
fi
