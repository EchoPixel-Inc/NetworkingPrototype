project(barco
    LANGUAGES CXX
)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

set(${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/barco/barcoSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/barco/barcoSystemImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/barco/barcoUartFrame.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/barco/barcoV2Protocol.h
)

set(${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/barcoSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/barcoUartFrame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/barcoV2Protocol.cpp
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList}
    ${${PROJECT_NAME}_uiList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBS} common config)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})
