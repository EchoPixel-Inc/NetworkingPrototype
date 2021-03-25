project(appcore)

set(${PROJECT_NAME}_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationObjects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/messageEncoder.cpp
)

set(${PROJECT_NAME}_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/appcore/messages.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/appcore/applicationObjects.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/appcore/serializationTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/appcore/serializationHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/appcore/messageEncoder.h
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS}
${${PROJECT_NAME}_HDRS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE common widgets networking 
    cereal::cereal ${VTK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES
    ${VTK_LIBRARIES}
)
