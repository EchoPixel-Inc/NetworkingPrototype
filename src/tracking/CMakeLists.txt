project(tracking
    LANGUAGES CXX
)

list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/trackingTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/trackingUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/headTargetInterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/headTargetBuilderInterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/zSpaceHeadTargetBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/interactionDeviceInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/interactionDeviceBuilderInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/zSpaceInteractionDeviceBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/leapMotionClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/leapMotionInteractionDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/leapMotionInteractionDeviceBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/barcoHeadTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/barcoHeadTargetBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/vrInkInteractionDeviceBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/ewmaFilter.h
)

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/headTargetInterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/zSpaceHeadTargetBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interactionDeviceInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zSpaceInteractionDeviceBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/leapMotionClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/leapMotionInteractionDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/leapMotionInteractionDeviceBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/barcoHeadTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/barcoHeadTargetBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vrInkInteractionDeviceBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ewmaFilter.cpp
)

if (USE_ZSPACE)
    list(APPEND ${PROJECT_NAME}_headerList
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/zSpaceHeadTarget.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/zSpaceInteractionDevice.h
    )

    list(APPEND ${PROJECT_NAME}_sourceList
        ${CMAKE_CURRENT_SOURCE_DIR}/zSpaceHeadTarget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/zSpaceInteractionDevice.cpp
    )
endif()

if (USE_VRINK)
    list(APPEND ${PROJECT_NAME}_headerList
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tracking/vrInkInteractionDevice.h
    )

list(APPEND ${PROJECT_NAME}_sourceList
    ${CMAKE_CURRENT_SOURCE_DIR}/vrInkInteractionDevice.cpp
    )
endif()

add_library(tracking STATIC ${${PROJECT_NAME}_sourceList}
    ${${PROJECT_NAME}_headerList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBS} common)
target_link_libraries(${PROJECT_NAME} PRIVATE barco)

if (USE_ZSPACE)
    target_link_libraries(${PROJECT_NAME} PRIVATE zspace)
endif()

if (USE_VRINK)
    target_link_libraries(${PROJECT_NAME} PRIVATE VRInkAPI::VRInkAPI)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_VRINK)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files"
    FILES ${${PROJECT_NAME}_headerList})
