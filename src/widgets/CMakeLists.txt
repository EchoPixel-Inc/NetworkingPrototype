project(widgets
    LANGUAGES CXX
)

list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/widgetInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/laserWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/splineWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/volumeWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/planeWidget.h
)

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/laserWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/splineWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/planeWidget.cpp
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC common vtkUtils interaction 
    cereal::cereal ${VTK_LIBRARIES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES
    ${VTK_LIBRARIES}
)
