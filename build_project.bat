@echo off
setlocal enabledelayedexpansion

echo [BUILD] Checking for tools...

:: 1. Find CMake
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set "CMAKE_CMD=cmake"
) else (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "CMAKE_CMD=C:\Program Files\CMake\bin\cmake.exe"
    ) else (
        echo [ERROR] CMake not found via PATH or Standard Location.
        exit /b 1
    )
)

:: 2. Find Clang (LLVM)
where clang >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set "CC_CMD=clang"
) else (
    if exist "C:\Program Files\LLVM\bin\clang.exe" (
        set "CC_CMD=C:\Program Files\LLVM\bin\clang.exe"
    ) else (
        echo [ERROR] Clang not found via PATH or Standard Location.
        exit /b 1
    )
)

echo [INFO] Using CMake: "!CMAKE_CMD!"
echo [INFO] Using Clang: "!CC_CMD!"

:: 3. Build
if not exist build mkdir build
cd build

echo [BUILD] Generating build files...
:: We use "MinGW Makefiles" if available, or "NMake Makefiles", or just let CMake pick.
:: Forcing the compiler is crucial here.
"%CMAKE_CMD%" -G "MinGW Makefiles" -DCMAKE_C_COMPILER="!CC_CMD!" .. 
if %ERRORLEVEL% NEQ 0 (
    echo [WARN] MinGW Generator failed. Trying default generator...
    "%CMAKE_CMD%" -DCMAKE_C_COMPILER="!CC_CMD!" ..
)

echo [BUILD] Compiling...
"%CMAKE_CMD%" --build . --config Debug

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete.
    if exist "Debug\tnrm1n4l.exe" (
        echo [INFO] Run with: build\Debug\tnrm1n4l.exe
    ) else (
        echo [INFO] Run with: build\tnrm1n4l.exe
    )
) else (
    echo [ERROR] Compilation failed.
)
cd ..

