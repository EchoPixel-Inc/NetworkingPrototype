# Module for locating the ZSpace library
#
# Read-Only variables:
#   ZSPACE_FOUND
#     Indicates whether the library has been found.
#
#   ZSPACE_INCLUDE_DIRS
#     Points to the OpenVR include directory.
#
#   ZSPACE_LIBRARIES
#     Points to the OpenVR libraries that should be passed to
#     target_link_libararies.
#
#   In accordance with modern CMake practice, this Find module also creates an
#   zspace interface target under an zspace namespace that can be passed to
#   target_link_libraries that passes all the necessary include directories and
#   libraries
#
# Copyright (c) 2020 Jeffrey Kasten (jeffrey@echopixeltech.com)
set(_ZSPACE_POSSIBLE_DIRS ${ZSPACE_ROOT_DIR})
set(_ZSPACE_POSSIBLE_INCLUDE_SUFFIXES Inc)
set(_ZSPACE_POSSIBLE_LIB_SUFFIXES Lib/x64)

find_path(ZSPACE_ROOT_DIR
    NAMES zSpaceSDKsUninstall.exe
    PATHS ${_ZSPACE_POSSIBLE_DIRS}
    DOC "zSpace root directory"
)

set(ZSPACE_INCLUDE_DIR ${ZSPACE_ROOT_DIR}/Inc)
set(ZSPACE_LIB_DIR ${ZSPACE_ROOT_DIR}/Lib/x64)
mark_as_advanced(ZSPACE_INCLUDE_DIR)

find_library(ZSPACE_LIB
    NAMES  zSpaceApi64
    PATHS ${ZSPACE_ROOT_DIR}
    PATH_SUFFIXES ${_ZSPACE_POSSIBLE_LIB_SUFFIXES}
    DOC "zSpace library"
)
mark_as_advanced(ZSPACE_LIB)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZSPACE REQUIRED_VARS ZSPACE_INCLUDE_DIR
    ZSPACE_LIB)

if (ZSPACE_FOUND)
    set(ZSPACE_INCLUDE_DIRS "${ZSPACE_INCLUDE_DIR}")
    set(ZSPACE_LIBRARIES "${ZSPACE_LIB}")
    if (NOT TARGET zspace::zspace)
        add_library(zspace::zspace INTERFACE IMPORTED)
        target_include_directories(zspace::zspace INTERFACE
            "${ZSPACE_INCLUDE_DIR}")
        target_link_libraries(zspace::zspace INTERFACE "${ZSPACE_LIB}")
    endif()
endif()
