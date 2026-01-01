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

:: 3. Find Ninja
where ninja >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set "NINJA_CMD=ninja"
) else (
    if exist "C:\Program Files\Ninja\ninja.exe" (
        set "NINJA_CMD=C:\Program Files\Ninja\ninja.exe"
    ) else (
        if exist "%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe\ninja.exe" (
            set "NINJA_CMD=%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe\ninja.exe"
        ) else (
            echo [ERROR] Ninja not found. Please install Ninja.
            exit /b 1
        )
    )
)

echo [INFO] Using Ninja: "!NINJA_CMD!"

:: 4. Find llvm-rc (Resource Compiler)
if exist "C:\Program Files\LLVM\bin\llvm-rc.exe" (
    set "RC_CMD=C:\Program Files\LLVM\bin\llvm-rc.exe"
) else (
    echo [ERROR] llvm-rc not found.
    exit /b 1
)

:: Normalize paths for CMake (Replace \ with /)
set "NINJA_CMD=!NINJA_CMD:\=/!"
set "CC_CMD=!CC_CMD:\=/!"
set "RC_CMD=!RC_CMD:\=/!"

:: 5. Build
if not exist build mkdir build
cd build

echo [BUILD] Generating build files...
"%CMAKE_CMD%" -G "Ninja" -DCMAKE_MAKE_PROGRAM="!NINJA_CMD!" -DCMAKE_C_COMPILER="!CC_CMD!" -DCMAKE_RC_COMPILER="!RC_CMD!" .. 
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake generation failed.
    exit /b 1
)

echo [BUILD] Compiling...
"%CMAKE_CMD%" --build . --config Debug

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete.
    echo [INFO] Run with: build\tnrm1n4l.exe
) else (
    echo [ERROR] Compilation failed.
)
cd ..

