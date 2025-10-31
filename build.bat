@echo off
setlocal enabledelayedexpansion

set "ROOT_DIR=%~dp0"
set "BUILD_DIR=%ROOT_DIR%out\build\x64-Release"
set "CONFIG=Release"

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%" 2>nul
    if errorlevel 1 (
        echo Failed to create build directory "%BUILD_DIR%".
        exit /b 1
    )
)

echo Configuring CMake project...
cd /d "%ROOT_DIR%"
cmake -S . -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo.
    echo CMake configuration failed.
    exit /b 1
)

echo Building %CONFIG% configuration...
cmake --build "%BUILD_DIR%" --config %CONFIG%
if errorlevel 1 (
    echo.
    echo Build failed.
    exit /b 1
)

echo.
echo Build completed successfully.
echo Output: "%BUILD_DIR%\Release\AppModuleCore.dll"
echo.

if exist "%ROOT_DIR%obfuscate.bat" (
    echo Starting post-build obfuscation...
    call "%ROOT_DIR%obfuscate.bat"
)

exit /b 0