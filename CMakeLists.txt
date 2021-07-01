cmake_minimum_required(VERSION 3.13)
project(python_clang)

find_package(PythonInterp 3.8 REQUIRED)

# This goes after, since it uses PythonInterp as hint
find_package(PythonLibs 3.8 REQUIRED)

set(SRCS python_clang_module.c)

add_library(python_clang SHARED ${SRCS})

set_target_properties(
    python_clang
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME "python_clang"
        LINKER_LANGUAGE C
    )

target_include_directories(python_clang PUBLIC ${PYTHON_INCLUDE_DIRS})
target_link_libraries(python_clang PUBLIC ${PYTHON_LIBRARIES})


