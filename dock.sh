#!/bin/bash
set -e

_run_() { echo -e "\033[0;33m$*\033[0m"; $*; }
_info_() { echo -e "\033[1;34m$*\033[0m"; }
_error_() { echo -e "\033[0;31m$*\033[0m"; }
_success_() { echo -e "\033[0;32m$*\033[0m"; }

if [ ! -f /.dockerenv ]; then
    if ! command -v docker &> /dev/null; then
        _error_ "Docker is not available. Please install it in your system."
        exit 1
    fi

    DOCK_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
    REPO_ROOT="$(cd $DOCK_DIR &> /dev/null && pwd)"
    SCRIPT_DIR_REL=$(realpath --relative-to=$REPO_ROOT "$PWD")

    if [ -z ${SKIP_DOCKER_BUILD+x} ]; then
        _run_ docker build --progress=plain -t docker-ci -f $DOCK_DIR/Dockerfile.builder \
        --build-arg "USER_NAME=$(whoami)" \
        --build-arg "USER_UID=$(id -u)" \
        --build-arg "USER_GID=$(id -g)" \
        "$DOCK_DIR"
    fi

    if [ -t 1 ] ; then IT_ARGS=-it; else IT_ARGS= ; fi
    if (return 0 2>/dev/null) ; then # sourced by another script
        _run_ docker run $IT_ARGS \
            --env-file <(env) \
            -v "$REPO_ROOT:/src" \
            -w /src/$SCRIPT_DIR_REL docker-ci $0 $@
    else # invoked directly
         _run_ docker run $IT_ARGS \
            --env-file <(env) \
            -v "$REPO_ROOT:/src" \
            -w /src/$SCRIPT_DIR_REL docker-ci $@
    fi
    exit $?
fi