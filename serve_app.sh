#!/bin/bash

pushd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/webapp"

if [ "$1" == "remote" ]; then
    npx ionic serve --no-browser --address 0.0.0.0 --port 4200 --disable-host-check
else
    npx ionic serve $*
fi

popd
