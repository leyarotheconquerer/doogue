cmake_minimum_required(VERSION 3.2)
project(doogue)

# Set relevant flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

# Include the relevant SFML modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(SFML_ROOT "" CACHE PATH "Path to the root of the SFML installation")
find_package(SFML COMPONENTS window graphics system REQUIRED)
if(NOT SFML_FOUND)
    message(WARNING "Unable to find SFML")
endif()

# List source files
set(SOURCE_FILES
	src/main.cpp
)

# Specify build target
include_directories(${SFML_INCLUDE_DIR})
add_executable(gameoff ${SOURCE_FILES})
target_link_libraries(gameoff ${SFML_LIBRARIES})
