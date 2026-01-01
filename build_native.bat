@echo off
setlocal
set "PROJECT_ROOT=%cd%"
set "ZIG_DIR=%PROJECT_ROOT%\tools\zig"

echo [BUILD] Compiling with Zig CC (Local)...

if not exist "%ZIG_DIR%\zig.exe" (
    echo [ERROR] Zig not found at %ZIG_DIR%
    exit /b 1
)

"%ZIG_DIR%\zig.exe" cc "%PROJECT_ROOT%\src\main.c" -o "%PROJECT_ROOT%\tnrm1n4l.exe" -lkernel32 -luser32 -g
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build Failed.
    exit /b 1
)

echo [SUCCESS] TNRM1N4L Kernel Built.
echo [INFO] Run: tnrm1n4l.exe
