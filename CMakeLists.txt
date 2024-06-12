cmake_minimum_required(VERSION 3.5)

project(CumulusMQ VERSION 1.0)
set (CMAKE_CXX_STANDARD 17)


include_directories(include)
# Include all source files in the src/ directory and its subdirectories
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

# Create the executable target
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

