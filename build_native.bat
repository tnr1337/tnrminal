@echo off
if not exist build mkdir build
gcc src/main.c src/utils.c src/cmd_sys.c src/cmd_file.c src/cmd_data.c src/cmd_net.c src/cmd_fun.c src/cmd_extra.c -o build/tnrm1n4l.exe
if %ERRORLEVEL% NEQ 0 (
    echo Build failed.
    exit /b %ERRORLEVEL%
)
echo Build success.
copy /Y build\tnrm1n4l.exe .
