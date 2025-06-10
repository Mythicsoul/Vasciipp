cmake_minimum_required(VERSION 3.29)
project(vasciipp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenCV_DEBUG FALSE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Set compiler flags for different build types
# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(STATUS "Debug mode enabled")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0") # No optimization
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    message(STATUS "Release with Debug Info mode enabled")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 ") # Moderate optimization with debug symbols
elseif(CMAKE_BUILD_TYPE MATCHES "GProf")
    message(STATUS "GProf mode enabled")
    set(CMAKE_CXX_FLAGS_GPROF "-pg -O0") # gprof profiling
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "Release mode enabled")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

find_package(OpenCV REQUIRED)
# message(STATUS "OpenCV status:")
# message(STATUS "  version: ${OpenCV_VERSION}")
# message(STATUS "  libs: ${OpenCV_LIBS}")
# message(STATUS "  include path: ${OpenCV_INCLUDE_DIRS}")

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include)

set(FILES
  src/main.cpp
  src/ascii.cpp
  src/utils.cpp
)

add_executable(vasciipp ${FILES})

target_compile_options(vasciipp PRIVATE -Wall -Wextra -pedantic)
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
target_link_libraries(vasciipp ${OpenCV_LIBS}) 
