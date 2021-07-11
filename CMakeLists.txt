cmake_minimum_required(VERSION 3.13)
project(python_clang)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(PythonInterp 3.8 REQUIRED)
find_package(PythonLibs 3.8 REQUIRED)
find_package(Clang)
include(CTest)

# COMMON LIBRARY
add_library(clang_interface clang_interface.cpp)
# hacks for: clang: libs in bad order, llvm: not including all deps
target_link_libraries(clang_interface PUBLIC ${CLANG_LIBS} ${CLANG_LIBS} ${LLVM_LIBS} "-Wl,/usr/lib/llvm-10/lib/LLVMPolly.so")
target_include_directories(clang_interface PRIVATE ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_include_directories(clang_interface PUBLIC ${CMAKE_SOURCE_DIR})
set_property(TARGET clang_interface PROPERTY POSITION_INDEPENDENT_CODE ON)

# PYTHON INTERFACE SHARED LIBRARY
add_library(python_clang SHARED 
    python_clang_module.c 
    python_clang_compiler.c 
    python_clang_function.c
    )
set_target_properties(
    python_clang
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME "python_clang"
        LINKER_LANGUAGE C
    )
target_include_directories(python_clang PUBLIC ${PYTHON_INCLUDE_DIRS})
target_link_libraries(python_clang PUBLIC ${PYTHON_LIBRARIES} clang_interface)
add_test(
        NAME python_usage 
        COMMAND sh -c "python3.8 test.py local_shared"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})   
set_tests_properties(python_usage PROPERTIES ENVIRONMENT PYTHON_CLANG_SO=$<TARGET_FILE:python_clang>)

# TEST CLI
add_executable(clang_interface_cli clang_interface_cli.c)
target_link_libraries(clang_interface_cli PUBLIC clang_interface)
set_target_properties(clang_interface_cli PROPERTIES LINKER_LANGUAGE CXX)
add_test(NAME clang_interface_cli COMMAND clang_interface_cli)
