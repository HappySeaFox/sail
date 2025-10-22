@echo off
REM Build wheels for multiple Python versions on Windows using vcpkg
REM Requires: pyenv-win for multi-version builds
REM For single version, current Python is used

setlocal enabledelayedexpansion

@cd /d "%~dp0"

REM Python versions to build for
set PYTHON_VERSIONS=3.9 3.10 3.11 3.12 3.13 3.14

REM Check if pyenv is available for multi-version builds
set MULTI_VERSION=false
where pyenv >nul 2>&1
if not errorlevel 1 (
    set MULTI_VERSION=true
)

echo =====================================
echo =  Building Python Wheels for PyPI  =
echo =====================================
echo.
echo Platform: Windows

if "%MULTI_VERSION%"=="true" (
    echo Python Manager: pyenv
    echo Target versions: %PYTHON_VERSIONS%
) else (
    echo Mode: Single Python version ^(current^)
    python --version
    echo.
    echo To build for multiple Python versions, install:
    echo   pyenv-win: https://github.com/pyenv-win/pyenv-win
)
echo.

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

echo [1/3] Cleaning previous builds...
if exist dist rmdir /s /q dist
if exist build rmdir /s /q build
if exist build-python rmdir /s /q build-python
if exist wheelhouse rmdir /s /q wheelhouse
if exist sailpy.egg-info rmdir /s /q sailpy.egg-info
del /q sailpy\*.pyd 2>nul
del /q sailpy\*.dll 2>nul

REM Create output directory
mkdir wheelhouse 2>nul

echo [2/3] Building wheels...

if "%MULTI_VERSION%"=="true" (
    pyenv update
    if errorlevel 1 (
        echo Error: Failed to update pyenv
        exit /b 1
    )

    REM Build for multiple Python versions
    for %%v in (%PYTHON_VERSIONS%) do (
        echo.
        echo ========================================
        echo Building for Python %%v
        echo ========================================

        REM Install Python version if not available
        pyenv versions | findstr "%%v" >nul
        if errorlevel 1 (
            echo Installing Python %%v with pyenv...
            pyenv install %%v
            if errorlevel 1 (
                echo Warning: Failed to install Python %%v, skipping...
                goto :next_version
            )
        )

        REM Set Python version
        pyenv local %%v
        if errorlevel 1 (
            echo Warning: Failed to set Python %%v, skipping...
            goto :next_version
        )

        REM Create temporary virtual environment
        set "TEMP_VENV=venv-%%v"
        if exist "!TEMP_VENV!" rmdir /s /q "!TEMP_VENV!"
        python -m venv "!TEMP_VENV!"
        if errorlevel 1 (
            echo Warning: Failed to create virtual environment for Python %%v, skipping...
            goto :next_version
        )

        REM Activate virtual environment
        call "!TEMP_VENV!\Scripts\activate"
        if errorlevel 1 (
            echo Warning: Failed to activate virtual environment for Python %%v, skipping...
            goto :next_version
        )

        REM Install build tools
        python -m pip install --upgrade pip
        python -m pip install --upgrade build twine wheel setuptools pybind11 cmake numpy
        if errorlevel 1 (
            echo Warning: Failed to install build tools for Python %%v, skipping...
            call "!TEMP_VENV!\Scripts\deactivate"
            goto :next_version
        )

        REM Build wheel
        echo Building wheel for Python %%v...
        python -m build --wheel
        if errorlevel 1 (
            echo Warning: Failed to build wheel for Python %%v, skipping...
            call "!TEMP_VENV!\Scripts\deactivate"
            goto :next_version
        )

        REM Copy wheels to wheelhouse
        copy dist\*.whl wheelhouse\ >nul
        if errorlevel 1 (
            echo Warning: Failed to copy wheels for Python %%v
        ) else (
            echo ✓ Completed Python %%v
        )

        REM Clean up
        call "!TEMP_VENV!\Scripts\deactivate"
        rmdir /s /q "!TEMP_VENV!" 2>nul
        rmdir /s /q dist 2>nul
        rmdir /s /q build 2>nul
        rmdir /s /q build-python 2>nul
        rmdir /s /q sailpy.egg-info 2>nul
        del /q sailpy\*.pyd 2>nul
        del /q sailpy\*.dll 2>nul

        :next_version
        echo.
    )
) else (
    REM Build for current Python only
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

    echo Installing build tools...
    python -m pip install --upgrade pip
    python -m pip install --upgrade build twine wheel setuptools pybind11
    if errorlevel 1 (
        echo ERROR: Failed to install build tools
        exit /b 1
    )

    echo Building wheel...
    echo NOTE: Building wheel only ^(sdist is not usable for this package^)
    python -m build --wheel
    if errorlevel 1 (
        echo ERROR: Failed to build wheel
        exit /b 1
    )

    REM Copy wheel to wheelhouse
    copy dist\*.whl wheelhouse\ >nul
)

echo [3/3] Checking all wheels...
REM Create temporary venv for checking if not already in one
if "%VIRTUAL_ENV%"=="" (
    echo Creating temporary venv for checking...
    python -m venv venv
    call venv\Scripts\activate
    pip install --upgrade pip twine >nul
)

for %%f in (wheelhouse\*.whl) do (
    echo Checking: %%f
    twine check "%%f"
    if errorlevel 1 (
        echo Warning: Wheel check failed for %%f
    )
)

echo.
echo =====================================
echo  ✓  Wheels ready for distribution  ✓
echo =====================================
echo.

echo Wheels location: wheelhouse\
dir /b wheelhouse\*.whl

echo.
echo Next steps:
echo   Upload to Test PyPI:    twine upload --repository testpypi wheelhouse\*.whl
echo   Install from Test PyPI: pip install --index-url https://test.pypi.org/simple sailpy
echo.
echo   Upload to PyPI:    twine upload wheelhouse\*.whl
echo   Install from PyPI: pip install --index-url https://pypi.org/simple sailpy
echo.

endlocal
