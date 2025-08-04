cmake_minimum_required(VERSION 3.14)

# if (MSVC)
#   set(CMAKE_VS_GLOBALS "EnableASAN=true")
# endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJ_NAME SlotMapTest)
project(${PROJ_NAME} 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Slot map data structure implementation and tests"
)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type for performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(TEST_SOURCES 
  SlotMapTest01.cpp
  SlotMapTest02.cpp
  SlotMapTest03.cpp
  SlotMapTest04.cpp
)

add_executable(${PROJ_NAME} ${TEST_SOURCES})

# Set Visual Studio startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SlotMapTest)


if(MSVC)
  target_compile_options(${PROJ_NAME} PRIVATE /W4 /WX /permissive- /EHsc)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  
  # Enable iterator debugging in debug builds
  target_compile_definitions(${PROJ_NAME} PRIVATE 
      $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=2>
  )
else()
  target_compile_options(${PROJ_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  
  # Enable debug flags for debug builds
  target_compile_options(${PROJ_NAME} PRIVATE 
      $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
  )
endif()

# add slot_map
add_subdirectory("${PROJECT_SOURCE_DIR}/slot_map")
target_link_libraries(${PROJ_NAME} slot_map)

# add gtest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")
target_link_libraries(${PROJ_NAME} gtest_main)

# Coverage support - this is crucial for codecov.io
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE AND NOT MSVC)
    target_compile_options(${PROJ_NAME} PRIVATE --coverage -O0)
    target_link_options(${PROJ_NAME} PRIVATE --coverage)
endif()

# Optional: Add address sanitizer support
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN AND NOT MSVC)
    target_compile_options(${PROJ_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${PROJ_NAME} PRIVATE -fsanitize=address)
endif()

# Optional: Add thread sanitizer support
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
if(ENABLE_TSAN AND NOT MSVC)
    target_compile_options(${PROJ_NAME} PRIVATE -fsanitize=thread)
    target_link_options(${PROJ_NAME} PRIVATE -fsanitize=thread)
endif()

# Optional: Add memory sanitizer support  
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)
if(ENABLE_MSAN AND NOT MSVC)
    target_compile_options(${PROJ_NAME} PRIVATE -fsanitize=memory -fno-omit-frame-pointer)
    target_link_options(${PROJ_NAME} PRIVATE -fsanitize=memory)
endif()

# Print configuration info
message(STATUS "SlotMapTest Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Address Sanitizer: ${ENABLE_ASAN}")
message(STATUS "  Thread Sanitizer: ${ENABLE_TSAN}")
message(STATUS "  Memory Sanitizer: ${ENABLE_MSAN}")
message(STATUS "  Coverage: ${ENABLE_COVERAGE}")


