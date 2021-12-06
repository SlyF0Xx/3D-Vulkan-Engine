@echo off
cd %~dp0

set build_dir=build
set vcpkg_dir=C:\src\vcpkg
set /p vcpkg_dir=Enter the path to the vcpkg: 

cd vendor\Edyn
md %build_dir%
cd %build_dir%
@echo on
cmake.exe .. "-DCMAKE_TOOLCHAIN_FILE=%vcpkg_dir%\scripts\buildsystems\vcpkg.cmake" -B .
cmake.exe --build . --target INSTALL
pause