@echo off
setlocal enabledelayedexpansion

set "EXE_PATH=Build\TODOAPP\TODOAPP.exe"

if exist "%EXE_PATH%" goto :run

echo [INFO] TODOAPP.exe not found, building with Release configuration...

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "delims=" %%i in ('call "%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe') do set "MSBUILD=%%i"

if not defined MSBUILD (
    echo [ERROR] MSBuild not found!
    pause
    exit /b 1
)

"%MSBUILD%" "Gear Engine.sln" /p:Configuration=Release /p:Platform=x64 /t:TODOAPP /v:minimal

if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo [INFO] Build succeeded.

:run
cd TODOAPP
start "" "../Build/TODOAPP/TODOAPP.exe"