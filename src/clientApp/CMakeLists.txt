project(clientApp)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

set(${PROJECT_NAME}_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/darkStyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/peerDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/appInitializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clientApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/peerConnectionWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networkSessionSelectionDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networkSessionConnectionDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ipv4Widget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/credentialsDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trackingManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trackerEventProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uiActions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/applicationActionsWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/autostereoscopicOpenGLRenderWindow.cpp
)

set(${PROJECT_NAME}_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/darkStyle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/peerDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/appInitializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/clientApp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/mainWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/peerConnectionWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/networkSessionSelectionDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/networkSessionConnectionDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/ipv4Widget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/credentialsDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/trackingManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/trackerEventProcessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/uiActions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/applicationActionsWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clientApp/autostereoscopicOpenGLRenderWindow.h
)

set(${PROJECT_NAME}_UI
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/mainWindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/credentialsDialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/peerConnectionWindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/networkSessionSelectionDialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/networkSessionConnectionDialog.ui
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS}
${${PROJECT_NAME}_HDRS} ${${PROJECT_NAME}_UI})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE networking common config interaction widgets
    display tracking appcore ${VTK_LIBRARIES})
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
