
@echo off
echo Testing TOUCH...
echo ORIGINAL CONTENT > touch_test.txt
echo touch touch_test.txt | tnrm1n4l.exe
type touch_test.txt
echo.

echo Testing SORT WARNING...
echo Creating large file...
(for /l %%x in (1, 1, 105) do echo %%x) > sort_test.txt
echo sort sort_test.txt | tnrm1n4l.exe > sort_out.txt
find "Warning" sort_out.txt

echo.
echo Cleaning up...
del touch_test.txt sort_test.txt sort_out.txt
