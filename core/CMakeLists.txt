cmake_minimum_required(VERSION 3.15)
project(SDGS_Core VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)
find_package(libzip REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(Threads REQUIRED)
set(SDGS_SOURCES
    src/main.cpp
    src/core/file_loader.cpp
    src/buffer/buffer_manager.cpp
    src/land/land_module.cpp
    src/life/life_module.cpp
    src/mind/mind_module.cpp
    src/network/network_module.cpp
    src/ui/ui_module.cpp
    src/script/script_manager.cpp
    src/script/lang/LISP/lisp_handler.cpp
    src/script/lang/FORTRAN/fortran_handler.cpp
    src/script/lang/COBOL/cobol_handler.cpp
    src/script/lang/BASIC/basic_handler.cpp
    src/script/lang/APL/apl_handler.cpp
    src/script/lang/SNOBOL/snobol_handler.cpp
    src/script/lang/SIMULA/simula_handler.cpp
    src/script/lang/PASCAL/pascal_handler.cpp
    src/script/lang/C/c_handler.cpp
    src/script/lang/FORTH/forth_handler.cpp
    src/script/lang/RUST/rust_handler.cpp
    src/script/lang/ERLANG/erlang_handler.cpp
    src/script/lang/LUA/lua_handler.cpp
    src/script/lang/FENNEL/fennel_handler.cpp
    src/script/lang/MOONSCRIPT/moonscript_handler.cpp
    src/script/lang/LUAU/luau_handler.cpp
    src/script/lang/METALUA/metalua_handler.cpp
    src/script/lang/GSL/gsl_handler.cpp
    src/script/lang/LLVM-LUA/llvm_lua_handler.cpp
    src/script/lang/CLIPS/clips_handler.cpp
    src/script/lang/LLVM-IR/llvm_ir_handler.cpp
    src/python/python_bridge.cpp
    src/script/llvm_compiler.cpp
)
add_executable(sdgs_core ${SDGS_SOURCES})
target_include_directories(sdgs_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/buffer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/land
    ${CMAKE_CURRENT_SOURCE_DIR}/src/life
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mind
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/script
    ${CMAKE_CURRENT_SOURCE_DIR}/src/python
)
target_link_libraries(sdgs_core
    ${LLVM_LIBRARIES}
    Boost::boost
    nlohmann_json::nlohmann_json
    libzip::zip
    pybind11::embed
    Threads::Threads
)
if(WIN32)
    target_link_libraries(sdgs_core ws2_32 crypt32)
endif()
if(MSVC)
    target_compile_options(sdgs_core PRIVATE /W4 /WX-)
else()
    target_compile_options(sdgs_core PRIVATE -Wall -Wextra -Wpedantic)
endif()