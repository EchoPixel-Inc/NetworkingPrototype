set(TEST_NAME testNetworkUtilities)

add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/testMessageParser.cpp)
target_link_libraries(${TEST_NAME} gtest gmock gtest_main networking)
gtest_discover_tests(${TEST_NAME})
