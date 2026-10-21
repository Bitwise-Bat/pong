cmake_minimum_required(VERSION 3.5)
project(pong CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/web/src/wasm")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(DR_LIBS_INCLUDE_DIRS "dr_mp3.h" REQUIRED)

add_executable(pong main.cpp)

target_link_libraries(pong PRIVATE SDL3::SDL3)
target_link_libraries(pong PRIVATE glm::glm)
target_include_directories(pong PRIVATE ${DR_LIBS_INCLUDE_DIRS})

set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
file(GLOB_RECURSE ASSET_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/*")

if (EMSCRIPTEN)
    set_target_properties(pong PROPERTIES SUFFIX ".js")
    
    target_compile_options(pong PRIVATE "--use-port=emdawnwebgpu")
    target_link_options(pong PRIVATE
            "-sWASM=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sMODULARIZE=1"
            "-sEXPORT_ES6=1"
            "-sEXPORT_NAME=createPong"
            "--emit-tsd=${CMAKE_CURRENT_SOURCE_DIR}/web/src/wasm/pong.d.ts"
            "--bind"
            "--use-port=emdawnwebgpu"
            "--embed-file=${ASSETS_DIR}@/assets"
    )
    set_property(TARGET pong APPEND PROPERTY LINK_DEPENDS ${ASSET_FILES})
else()
    message(FATAL_ERROR "Pong only supports WebAssembly/Web builds.")
endif ()

target_include_directories(pong PRIVATE source)
target_sources(pong
        PRIVATE
        source/renderer/webgpu/renderer.cpp
            source/paddle.cpp
            source/audio.cpp
            source/input.cpp
            source/ball.cpp
            source/particles.cpp
            source/game.cpp
            source/web_bridge.cpp)
