#!/bin/bash
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/" >/dev/null 2>&1 && pwd )"

_end_() { echo -e "\033[0;31mCommand '$*' Failed!\033[0m" ; popd > /dev/null ; exit 1 ; }
_echo_() { echo -e "\033[0;33m$*\033[0m"; $* || _end_ $* ; }

pushd "$THIS_DIR" > /dev/null

[[ -f "$THIS_DIR/.venv/bin/activate" ]] || python -m venv "$THIS_DIR/.venv"
source "$THIS_DIR/.venv/bin/activate"
_echo_ pip install -r requirements.txt