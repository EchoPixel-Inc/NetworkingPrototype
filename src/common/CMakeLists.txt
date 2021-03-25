project(common
    LANGUAGES CXX
)

set(${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common/coreTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common/crcUtils.h
)

set(${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crcUtils.cpp
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBS} Eigen3::Eigen)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})
