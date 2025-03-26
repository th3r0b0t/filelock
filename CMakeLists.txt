cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0042 NEW)

project (filelock)

add_definitions(-DNAPI_VERSION=9)
add_definitions(-DNODE_ADDON_API_ENABLE_TYPE_CHECK_ON_AS)

include_directories(${CMAKE_JS_INC})

file(GLOB SOURCE_FILES "./cpp-src/*")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
