@echo off
setlocal
set "PROJECT_ROOT=%cd%"
set "ZIG_DIR=C:\Users\ACER\AppData\Local\Temp\WinGet\zig.zig.0.15.2\extracted\zig-x86_64-windows-0.15.2"

echo [BUILD] Compiling with Zig CC...

if not exist "%ZIG_DIR%\zig.exe" (
    echo [ERROR] Zig not found at %ZIG_DIR%
    exit /b 1
)

pushd "%ZIG_DIR%"
.\zig.exe cc "%PROJECT_ROOT%\src\main.c" -o "%PROJECT_ROOT%\tnrm1n4l.exe" -lkernel32 -luser32 -g
if %ERRORLEVEL% NEQ 0 (
    popd
    echo [ERROR] Build Failed with code %ERRORLEVEL%
    exit /b 1
)
popd

echo [SUCCESS] TNRM1N4L Kernel Built.
echo [INFO] Run: tnrm1n4l.exe
