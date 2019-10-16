pushd %~dp0
python build.py --test
python build.py --test --web
popd
