python build.py --gen_vs_sln --build_config=debug --arch_name=x64 %1

::if not exist $(ProjectDir)dist mklink /J $(ProjectDir)dist $(SolutionDir)..\webapp\dist
::if not exist $(ProjectDir)node_modules mklink /J $(ProjectDir)node_modules $(SolutionDir)..\webapp\node_modules