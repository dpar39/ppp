pushd %~dp0\..
python build.py -a x64 wasm web --test
popd