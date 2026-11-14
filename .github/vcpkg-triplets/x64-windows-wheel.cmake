# vcpkg overlay triplet for building sailpy wheels on Windows.
# DLLs with the dynamic CRT (/MD, same as Python), release only.
# setup.py copies the DLLs next to _libsail.pyd.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_BUILD_TYPE release)
