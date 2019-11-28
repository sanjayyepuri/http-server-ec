cmake_minimum_required(VERSION 3.10)

project(http-server)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

file(GLOB SOURCES "src/*.cpp" "src/*.h")

add_executable(server ${SOURCES})

find_package(Boost 1.65 REQUIRED COMPONENTS filesystem system)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(server ${Boost_LIBRARIES})
elseif(NOT Boost_FOUND)
    error("Boost Not Found")
endif()
