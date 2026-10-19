cmake_minimum_required(VERSION 3.5)
project(pong CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/web/src/wasm")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(pong main.cpp)

target_link_libraries(pong PRIVATE SDL3::SDL3)
target_link_libraries(pong PRIVATE glm::glm)

if (EMSCRIPTEN)
    set_target_properties(pong PROPERTIES SUFFIX ".js")
    
    target_compile_options(pong PRIVATE "--use-port=emdawnwebgpu")
    target_link_options(pong PRIVATE
            "-sWASM=1"
            "-sMODULARIZE=1"
            "-sEXPORT_ES6=1"
            "-sEXPORT_NAME=createPong"
            "--emit-tsd=${CMAKE_CURRENT_SOURCE_DIR}/web/src/wasm/pong.d.ts"
            "--bind"
            "--use-port=emdawnwebgpu"
    )
else()
    message(FATAL_ERROR "Pong only supports WebAssembly/Web builds.")
endif ()