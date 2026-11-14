cmake_minimum_required(VERSION 3.20)
project(wonderland LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Prefer Xcode's macOS SDK on Apple Silicon when CLT ships a newer SDK
# that the installed linker cannot parse (arm64e.x1 TBD errors).
if(APPLE AND NOT DEFINED CMAKE_OSX_SYSROOT)
  execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE _wl_sdk
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(_wl_sdk)
    set(CMAKE_OSX_SYSROOT "${_wl_sdk}" CACHE PATH "macOS SDK" FORCE)
  endif()
endif()

# Link Homebrew raylib by path — avoid raylib's exported absolute
# Framework paths into CommandLineTools SDKs that break the link step.
find_path(RAYLIB_INCLUDE_DIR raylib.h
  HINTS /opt/homebrew/include /usr/local/include
  REQUIRED)
find_library(RAYLIB_LIBRARY NAMES raylib
  HINTS /opt/homebrew/lib /usr/local/lib
  REQUIRED)

add_executable(wonderland
  src/main.cpp
  src/app/Game.cpp
  src/sim/Boss.cpp
  src/sim/Level.cpp
  src/sim/Shop.cpp
  src/sim/Sim.cpp
  src/render/Draw.cpp
)

target_include_directories(wonderland PRIVATE src "${RAYLIB_INCLUDE_DIR}")
target_compile_definitions(wonderland PRIVATE
  PLATFORM_DESKTOP
  GRAPHICS_API_OPENGL_33
  WL_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)
target_link_libraries(wonderland PRIVATE "${RAYLIB_LIBRARY}")

if(APPLE)
  target_link_libraries(wonderland PRIVATE
    "-framework IOKit"
    "-framework Cocoa"
    "-framework OpenGL"
  )
endif()
