 
cmake_minimum_required( VERSION 2.8 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( TrafficController )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL "Enable A")
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL "Enable B")
set(SPDLOG_FMT_EXTERNAL ON)

set(SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/geom.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm_mesh.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/graphics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/parser.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/render.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm_graph.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/agent.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/engine.cpp
)
#set(HEADERS
#   ${HEADERS}
#   ${CMAKE_CURRENT_SOURCE_DIR}/file1.hpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/file2.hpp
#   PARENT_SCOPE
#)

add_executable(Controller ${SOURCE})

target_include_directories(Controller PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/include
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glfw/include
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/coro
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glad/include
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/nanovg/src
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/nanovg_metal/include
   ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/pybind11/include
   ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
   ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(nanogui)
target_link_libraries(Controller PRIVATE nanogui)

add_subdirectory(fmt)
target_link_libraries(Controller PRIVATE fmt::fmt)

add_subdirectory(spdlog)
target_link_libraries(Controller PRIVATE spdlog::spdlog)
