project(interaction
    LANGUAGES CXX
)

list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/interaction/interactor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/interaction/customQEvents.h
)

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/interactor.cpp
)

add_library(interaction STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(interaction PUBLIC common ${VTK_LIBRARIES})
target_compile_features(interaction PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES
    ${VTK_LIBRARIES}
)
