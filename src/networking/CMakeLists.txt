project(networking
    LANGUAGES CXX
)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

set(${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/networking/connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/networking/connectionImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/networking/networkMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/networking/networkMessageParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/networking/tcpServer.h
)

set(${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/networkMessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/networkMessageParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tcpServer.cpp
)

set(${PROJECT_NAME}_uiList
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList}
    ${${PROJECT_NAME}_uiList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBS} cereal::cereal PRIVATE common)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})
