# vcpkg overlay triplet for building sailpy wheels on macOS (Apple Silicon).
# Shared libraries (delocate grafts them into the wheel), release only.
# Keep VCPKG_OSX_DEPLOYMENT_TARGET in sync with MACOSX_DEPLOYMENT_TARGET in the workflow.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET 11.0)
set(VCPKG_FIXUP_MACHO_RPATH ON)
set(VCPKG_BUILD_TYPE release)
