find_package(OpenEXR ${SAIL_CODEC_OPENEXR_REQUIRED_OPTION})

if (NOT OpenEXR_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(OpenEXR REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES OpenEXR::OpenEXR)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,OpenEXR,OpenEXR::OpenEXR" PARENT_SCOPE)

# Common codec configuration
#
sail_codec(NAME openexr
            SOURCES helpers.h helpers.cpp openexr.cpp
            ICON openexr.png
            DEPENDENCY_INCLUDE_DIRS ${OPENEXR_INCLUDE_DIRS}
            DEPENDENCY_LIBS OpenEXR::OpenEXR)

set_target_properties(sail-codec-openexr PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)
