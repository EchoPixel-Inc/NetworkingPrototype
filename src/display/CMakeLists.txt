project(display
    LANGUAGES CXX
)

list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display/displayInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display/displayUtilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display/EDIDInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display/displayInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display/genericDisplay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display/barcoDisplay.h
)

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/displayInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/displayUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/genericDisplay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parseEDID.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/barcoDisplay.cpp
)

if (USE_ZSPACE)
    list(APPEND ${PROJECT_NAME}_headerList
        ${CMAKE_CURRENT_SOURCE_DIR}/include/display/zSpaceDisplay.h
    )
    list(APPEND ${PROJECT_NAME}_sourceList
        ${CMAKE_CURRENT_SOURCE_DIR}/zSpaceDisplay.cpp
    )
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBS} common)
target_link_libraries(${PROJECT_NAME} PRIVATE barco)

if (USE_ZSPACE)
    target_link_libraries(${PROJECT_NAME} PRIVATE zspace)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})
