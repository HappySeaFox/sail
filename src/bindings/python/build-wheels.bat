@echo off
REM Build wheels for PyPI distribution on Windows using vcpkg

setlocal enabledelayedexpansion

@cd /d "%~dp0"

REM Set up virtual environment
if not exist "venv\Scripts\activate.bat" (
    echo Creating virtual environment...
    python -m venv venv
    if errorlevel 1 (
        echo ERROR: Failed to create virtual environment
        exit /b 1
    )
)

if "%VIRTUAL_ENV%"=="" (
    echo Activating virtual environment...
    call venv\Scripts\activate
    if errorlevel 1 (
        echo ERROR: Failed to activate virtual environment
        exit /b 1
    )
    echo.
)

echo =====================================
echo =  Building Python Wheels for PyPI  =
echo =====================================
echo.
echo Platform: Windows
echo.

echo [1/4] Cleaning previous builds...
if exist dist rmdir /s /q dist
if exist build rmdir /s /q build
if exist build-python rmdir /s /q build-python
if exist wheelhouse rmdir /s /q wheelhouse
if exist sailpy.egg-info rmdir /s /q sailpy.egg-info
del /q sailpy\*.pyd 2>nul
del /q sailpy\*.dll 2>nul

echo [2/4] Installing build tools...
python -m pip install --upgrade pip
python -m pip install --upgrade build twine wheel setuptools pybind11
if errorlevel 1 (
    echo ERROR: Failed to install build tools
    exit /b 1
)

echo [3/4] Setting up vcpkg environment...

REM Set vcpkg paths (vcpkg is in ../vcpkg relative to the project root)
set "PROJECT_ROOT=%~dp0..\..\..\"
set "VCPKG_ROOT=%PROJECT_ROOT%..\vcpkg"
set "CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
set "VCPKG_TARGET_TRIPLET=x64-windows"

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo ERROR: vcpkg not found at %VCPKG_ROOT%
    echo Please ensure vcpkg is cloned to ../vcpkg relative to the SAIL project root
    exit /b 1
)

echo vcpkg root: %VCPKG_ROOT%
echo CMAKE toolchain: %CMAKE_TOOLCHAIN_FILE%
echo Target triplet: %VCPKG_TARGET_TRIPLET%
echo.

REM Export environment variables for CMake (used by setup.py)
set CMAKE_ARGS=-DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE% -DVCPKG_TARGET_TRIPLET=%VCPKG_TARGET_TRIPLET% -DCMAKE_C_FLAGS=-MP -DCMAKE_CXX_FLAGS=-MP

echo Building wheel...
echo NOTE: Building wheel only (sdist is not usable for this package)
python -m build --wheel
if errorlevel 1 (
    echo ERROR: Failed to build wheel
    exit /b 1
)

set "WHEEL_DIR=dist"

echo [4/4] Checking wheel...
python -m twine check %WHEEL_DIR%\*.whl
if errorlevel 1 (
    echo ERROR: Wheel check failed
    exit /b 1
)

echo.
echo =================================
echo   Wheels ready for distribution
echo =================================
echo.

echo Wheels location: %WHEEL_DIR%\
dir /b %WHEEL_DIR%\*.whl

echo.
echo Next steps:
echo   Install locally: python -m pip install --force-reinstall %WHEEL_DIR%\*.whl
echo   Run tests: pytest tests
echo.
echo   Upload to Test PyPI:    twine upload --repository testpypi %WHEEL_DIR%\*.whl
echo   Install from Test PyPI: pip install --index-url https://test.pypi.org/simple sailpy
echo.
echo   Upload to PyPI:    twine upload %WHEEL_DIR%\*.whl
echo   Install from PyPI: pip install --index-url https://pypi.org/simple sailpy
echo.

endlocal
