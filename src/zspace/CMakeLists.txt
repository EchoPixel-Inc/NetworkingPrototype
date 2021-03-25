project(zspace
    LANGUAGES CXX
)

list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/zspace/zSpaceUtilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/zspace/zSpaceContextManager.h

)

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/zSpaceUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zSpaceContextManager.cpp
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBS} common zspace::zspace)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PUBLIC USE_ZSPACE)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})
