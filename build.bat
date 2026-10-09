@echo off
setlocal

where qmake >nul 2>nul
if errorlevel 1 (
    echo qmake was not found. Add the Qt bin directory to PATH.
    exit /b 1
)

where mingw32-make >nul 2>nul
if errorlevel 1 (
    echo mingw32-make was not found. Add the MinGW bin directory to PATH.
    exit /b 1
)

cd /d "%~dp0"
if not exist build mkdir build
cd build

qmake ..\MoeDiskScreenSaver.pro -spec win32-g++ "CONFIG+=release"
if errorlevel 1 exit /b 1

mingw32-make -j4
if errorlevel 1 exit /b 1

set "RELEASE=%CD%\release"
where windeployqt >nul 2>nul
if not errorlevel 1 windeployqt --release --no-translations "%RELEASE%\MoeDiskScreenSaver.exe"

copy /Y "%RELEASE%\MoeDiskScreenSaver.exe" "%RELEASE%\MoeDiskScreenSaver.scr" >nul

echo.
echo Built: %RELEASE%\MoeDiskScreenSaver.exe
echo Screen saver: %RELEASE%\MoeDiskScreenSaver.scr
echo Install: copy MoeDiskScreenSaver.scr to a folder and select it in Windows screen saver settings.
endlocal
