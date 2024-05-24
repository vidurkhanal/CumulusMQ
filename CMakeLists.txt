cmake_minimum_required(VERSION 3.5)

project(Apop VERSION 1.0)
set (CMAKE_CXX_STANDARD 17)

# Include all source files in the src/ directory and its subdirectories
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

# Include all header files in the src/ directory and its subdirectories
file(GLOB_RECURSE HEADER_FILES "src/*.h")

# Create the executable target
add_executable(Apop ${SOURCE_FILES} ${HEADER_FILES})
