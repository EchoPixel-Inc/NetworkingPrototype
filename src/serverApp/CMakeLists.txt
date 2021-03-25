project(serverApp)

set(${PROJECT_NAME}_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serverApp.cpp
)

set(${PROJECT_NAME}_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/serverApp/serverApp.h
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS}
${${PROJECT_NAME}_HDRS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE networking common widgets
    appcore ${VTK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES
    ${VTK_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
)
