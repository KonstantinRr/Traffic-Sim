 
cmake_minimum_required( VERSION 2.8 FATAL_ERROR )
set(CMAKE_VERBOSE_MAKEFILE ON)

project(TrafficController)
set(CMAKE_PROJECT_VERSION 1.0.0)
set(CMAKE_PROJECT_VERSION_MAJOR 1)
set(CMAKE_PROJECT_VERSION_MINOR 0)
set(CMAKE_PROJECT_VERSION_PATCH 0)

include(CheckCXXSourceRuns)

macro(CHECK_CXX_COMPILER_AND_LINKER_FLAGS_BASE _RESULT _CXX_FLAGS _LINKER_FLAGS)
  set(CMAKE_REQUIRED_FLAGS ${_CXX_FLAGS})
  set(CMAKE_REQUIRED_LIBRARIES ${_LINKER_FLAGS})
  set(CMAKE_REQUIRED_QUIET TRUE)
  check_cxx_source_runs("#include <iostream>\nint main(int argc, char **argv) { std::cout << \"test\"; return 0; }" ${_RESULT})
  set(CMAKE_REQUIRED_FLAGS "")
  set(CMAKE_REQUIRED_LIBRARIES "")
endmacro()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NANOGUI_BUILD_EXAMPLES ON CACHE BOOL "Build examples")
set(NANOGUI_BUILD_PYTHON OFF CACHE BOOL "Build Python")
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL "Build Shared")
set(SPDLOG_FMT_EXTERNAL ON)

# All source files in this project
set(SOURCE
   "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp                  "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/mapcanvas.cpp             "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/geom.cpp          "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm.cpp           "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm_mesh.cpp      "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/parser.cpp        "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/render.cpp        "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm_graph.cpp     "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/agent.cpp         "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/engine.cpp        "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/camera.cpp         "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/com.cpp            "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/entity.cpp         "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/glmodel.cpp        "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/graphics.cpp       "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/resource.cpp       "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/shader.cpp         "
   "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/tiny_obj_loader.cpp"
)
# All header files in this project
set(HEADERS
   ${CMAKE_CURRENT_SOURCE_DIR}/src/mapcanvas.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/listener.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/agent.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/engine.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/geom.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm_graph.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/osm_mesh.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/parser.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/traffic/render.hpp
)

# Prefer libc++ in conjunction with Clang
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fcxx-exceptions")
if (NOT MSVC)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_FLAGS MATCHES "-stdlib=libc\\+\\+")
    CHECK_CXX_COMPILER_AND_LINKER_FLAGS_BASE(HAS_LIBCPP "-stdlib=libc++" "-stdlib=libc++")
    if (HAS_LIBCPP)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -D_LIBCPP_VERSION")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
      message(STATUS "NanoGUI: using libc++.")
    else()
    CHECK_CXX_COMPILER_AND_LINKER_FLAGS_BASE(HAS_LIBCPP_AND_CPPABI "-stdlib=libc++" "-stdlib=libc++ -lc++abi")
      if (HAS_LIBCPP_AND_CPPABI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -D_LIBCPP_VERSION")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
        message(STATUS "NanoGUI: using libc++ and libc++abi.")
      else()
        message(FATAL_ERROR "When Clang is used to compile NanoGUI, libc++ must be available -- GCC's libstdc++ is not supported! (please insteall the libc++ development headers, provided e.g. by the packages 'libc++-dev' and 'libc++abi-dev' on Debian/Ubuntu).")
      endif()
    endif()
  endif()
endif()

add_executable(Controller ${SOURCE} ${HEADERS})

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

# Links the required subdirectories
add_subdirectory(nanogui)
target_link_libraries(Controller PRIVATE nanogui)

add_subdirectory(fmt)
target_link_libraries(Controller PRIVATE fmt::fmt)

#add_subdirectory(spdlog)
#target_link_libraries(Controller PRIVATE spdlog::spdlog)