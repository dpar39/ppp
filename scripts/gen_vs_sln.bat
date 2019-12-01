pushd %~dp0\..
python build.py --gen_vs_sln --build_config=debug -a x64 %1
popd
