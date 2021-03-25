project(vtkUtils
    LANGUAGES CXX
)

list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vtkUtils/vtkGeneralizedCallbackCommand.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vtkUtils/vtkErrorObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vtkUtils/vtkCommonConversions.h
)

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/vtkErrorObserver.cpp
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC common ${VTK_LIBRARIES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES
    ${VTK_LIBRARIES}
)