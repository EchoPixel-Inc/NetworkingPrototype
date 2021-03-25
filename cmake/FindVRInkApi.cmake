# Module for locating the Logitech VRInkAPI library
#
# Read-Only variables:
#   VRInkAPI_FOUND
#     Indicates whether the library has been found.
#
#   VRInkAPI_INCLUDE_DIRS
#     Points to the VRInkAPI include directory.
#
#   VRInkAPI_LIBRARIES
#     Points to the VRInkAPI libraries that should be passed to
#     target_link_libararies.
#
#   In accordance with modern CMake practice, this Find module also creates an
#   VRInkAPI interface target under a VRInkAPI namespace that can be passed to
#   target_link_libraries that passes all the necessary include directories and
#   libraries
#
# Copyright (c) 2020 Jeffrey Kasten (jeffrey@echopixeltech.com)

include(FindPackageHandleStandardArgs)

set(_VRInkAPI_POSSIBLE_INCLUDE_SUFFIXES include)
set(_VRInkAPI_POSSBLE_LIB_SUFFIXES lib)
set(_VRInkAPI_POSSIBLE_RUNTIME_SUFFIXES bin)

find_path(VRInkAPI_ROOT_DIR
    vr_ink_api.h
    NAMES
    DOC "VR Ink API root directory"
    REQUIRED
)
mark_as_advanced(VRInkAPI_ROOT_DIR)

set(VRInkAPI_INCLUDE_DIR ${VRInkAPI_ROOT_DIR})

find_library(VRInkAPI_LIBRARY
    NAMES vr_ink_api
    PATHS ${VRInkAPI_ROOT_DIR}
    PATH_SUFFIXES lib lib/x64 lib/x64/Release
    DOC "VR Ink API library"
)
mark_as_advanced(VRKInkAPI_LIBRARY)

find_file(VRInkAPI_RUNTIME
    NAMES vr_ink_api.dll
    PATHS ${VRInkAPI_ROOT_DIR}
    PATH_SUFFIXES bin bin/x64 bin/x64/Release
    DOC "VR Ink API runtime"
)
mark_as_advanced(VRInkAPI_RUNTIME)

find_package_handle_standard_args(VRInkAPI
    REQUIRED_VARS
    VRInkAPI_INCLUDE_DIR
    VRInkAPI_LIBRARY
    VRInkAPI_RUNTIME
)

if (VRInkAPI_FOUND)
    if (NOT TARGET VRInkAPI::VRInkAPI)
        add_library(VRInkAPI::VRInkAPI SHARED IMPORTED)
        set_target_properties(VRInkAPI::VRInkAPI PROPERTIES
            IMPORTED_IMPLIB ${VRInkAPI_LIBRARY}
            IMPORTED_LOCATION ${VRInkAPI_RUNTIME}
            INTERFACE_INCLUDE_DIRECTORIES ${VRInkAPI_INCLUDE_DIR}
        )
    endif()
endif()
