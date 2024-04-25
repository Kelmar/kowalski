cmake_minimum_required(VERSION 3.20)

project (6502 C CXX)

find_package(wxWidgets COMPONENTS core base REQUIRED)

include_directories(
    include/
    include/crystal
)

include("${wxWidgets_USE_FILE}")

file (GLOB_RECURSE SRCS src/*.cpp)

list(APPEND SOURCES ${SRCS})

add_executable(6502 ${SRCS})

target_link_libraries(6502 ${wxWidgets_LIBRARIES})

