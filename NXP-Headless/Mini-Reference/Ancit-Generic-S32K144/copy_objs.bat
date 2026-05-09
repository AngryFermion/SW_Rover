@echo off

:: Check if ancit_obj exists, if not, create it
if not exist "%CD%\ancit_obj" (
    echo Creating ancit_obj directory...
    mkdir "%CD%\ancit_obj"
)

:: Copy from ancit_lib
	echo Copying from Debug_FLASH\ancit_lib
for /r "%CD%\Debug_FLASH\ancit_lib" %%i in (*.o) do (
    echo                            Copying: %%~nxi
    copy /Y "%%i" "%CD%\ancit_obj\"
)

:: Copy from ancit_device_drivers
echo.
echo Copying from Debug_FLASH\ancit_device_drivers
for /r "%CD%\Debug_FLASH\ancit_device_drivers" %%i in (*.o) do (
    echo                          Copying: %%~nxi
    copy /Y "%%i" "%CD%\ancit_obj\"
)

echo Copy operation completed.
pause
