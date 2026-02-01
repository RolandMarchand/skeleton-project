cmake_minimum_required(VERSION 3.12)

project(project VERSION 1.0.0
  DESCRIPTION "A project"
  LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_compile_options(
  $<$<C_COMPILER_ID:GNU,Clang>:-Wall>
  $<$<C_COMPILER_ID:GNU,Clang>:-Wextra>
  $<$<C_COMPILER_ID:GNU,Clang>:-Wpedantic>
  $<$<C_COMPILER_ID:GNU,Clang>:-Werror=vla>
  $<$<C_COMPILER_ID:MSVC>:/W4>
)

include(sanitizers)
enable_sanitizers()

add_subdirectory(src)
add_subdirectory(tests)
