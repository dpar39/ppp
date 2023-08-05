#!/usr/bin/env bash
set -e

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source $REPO_ROOT/dock.sh "$*"

pushd $REPO_ROOT

# Extract protected data if password available
if [[ "$MUGSHOT_ZIP_PASSWORD" != "" ]] && [[ ! -f "research/mugshot_frontal_original_all/130_frontal.jpg" ]]; then
    pushd $REPO_ROOT/research > /dev/null
    _run_ unzip -qq -o -P "$MUGSHOT_ZIP_PASSWORD" mugshot_frontal_original_all_1.zip
    _run_ unzip -qq -o -P "$MUGSHOT_ZIP_PASSWORD" mugshot_frontal_original_all_2.zip
    _run_ unzip -qq -o -P "$MUGSHOT_ZIP_PASSWORD" mugshot_frontal_original_all_3.zip
    popd > /dev/null
fi

# Build and run native unit tests
_run_ npm run build:native:release

# Run native tests
_run_ npm run test:native

# Build webassembly binary
_run_ npm run wasm:all

# Build webapp and run tests
pushd $REPO_ROOT/webapp > /dev/null
_run_ npm install \
    && _run_ npm run gen-app-info \
    && _run_ npm run gen-pwa-icons \
    && _run_ npx ng test --browsers=ChromeHeadless --watch=false \
    && _run_ npx ionic build --prod

# Deploy to firebase
if [ "$BUILD_SOURCEBRANCH" == "refs/heads/main" ]; then
    _success_ "Deploying to firebase ... "
    npx firebase deploy --token "$FIREBASE_TOKEN" --non-interactive --project myphotoidapp
fi

popd > /dev/null
_success_ "---------------------- DONE ----------------------"

