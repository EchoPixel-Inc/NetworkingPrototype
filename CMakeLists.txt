cmake_minimum_required(VERSION 3.18)

project(networkViewer
    VERSION 0.1
    DESCRIPTION "A networked stereoscopic image viewer"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to handle the moc code generator automatically
set(CMAKE_AUTOMOC ON)

# Instruct CMake to handle the uic code generator automatically
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Xml CONFIG REQUIRED)
find_package(Qt5OpenGL CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5PrintSupport CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5WebSockets CONFIG REQUIRED)
find_package(Qt5SerialPort CONFIG REQUIRED)

set(QT_LIBS
    Qt5::Widgets
    Qt5::Xml
    Qt5::OpenGL
    Qt5::Network
    Qt5::Sql
    Qt5::PrintSupport
    Qt5::Gui
    Qt5::WebSockets
    Qt5::SerialPort
)

find_package(VTK REQUIRED NO_MODULE)
find_package(cereal REQUIRED)
find_package(Eigen3 REQUIRED)

set(USE_ZSPACE 0 CACHE BOOL "Use ZSpace Tracking library")
message(STATUS "ZSpace Tracking: ${USE_ZSPACE}")
if (USE_ZSPACE)
    find_package(ZSPACE REQUIRED)
endif()

set(USE_VRINK 0 CACHE BOOL "Enable Logitech VR Ink Tracking")
message(STATUS "Logitech VRInk Tracking: ${USE_VRINK}")
if (USE_VRINK)
    find_package(VRInkAPI REQUIRED)
endif()

set(EXECUTABLE_INSTALL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set(LIBRARY_INSTALLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_subdirectory(src)

option(BUILD_TESTS "Build unit tests" ON)
message(STATUS "Build unit testing: ${BUILD_TESTS}")

if (BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
      	GIT_TAG release-1.10.0
    )

    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
    add_subdirectory(test)
endif()

## Get all properties that cmake supports
execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)
## Convert command output into a CMake list
STRING(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
STRING(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)

function(print_target_properties tgt)
    if(NOT TARGET ${tgt})
      message(STATUS "There is no target named '${tgt}'")
      return()
    endif()

    foreach (prop ${CMAKE_PROPERTY_LIST})
        get_target_property(propval ${tgt} ${prop})
        if (propval)
            get_target_property(propval ${tgt} ${prop})
            message (STATUS "${tgt} ${prop} = ${propval}")
        endif()
    endforeach(prop)
endfunction(print_target_properties)

function(install_imported_runtime_libs target_list destination_folder)
    foreach(trg ${target_list})
        get_target_property(trg_type ${trg} TYPE)
        if (NOT trg_type STREQUAL "INTERFACE_LIBRARY")
            install(FILES $<TARGET_FILE:${trg}> DESTINATION ${destination_folder})
        endif()
    endforeach()
endfunction()

# Installation
install_imported_runtime_libs("${VTK_LIBRARIES}"
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
)
install_imported_runtime_libs("${QT_LIBS}"
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
)
if (USE_VRINK)
    install_imported_runtime_libs(VRInkAPI::VRInkAPI
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
    )
endif()
