@echo off
setlocal
set "PROJECT_ROOT=%cd%"
set "TCC_EXE=%PROJECT_ROOT%\tools\tcc\tcc.exe"

echo [BUILD] Compiling with TCC...
echo Path: "%TCC_EXE%"

if exist "%TCC_EXE%" goto found

echo [ERROR] TCC executable not found at expected path.
echo [INFO] contents of tools folder:
dir "%PROJECT_ROOT%\tools"
exit /b 1

:found
"%TCC_EXE%" "%PROJECT_ROOT%\src\main.c" -o "%PROJECT_ROOT%\tnrm1n4l.exe" -luser32 -lkernel32 -ladvapi32 -lshell32
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation Failed.
    exit /b 1
)

echo [SUCCESS] TNRM1N4L Kernel Built Successfully.
echo [INFO] Run command: tnrm1n4l.exe
