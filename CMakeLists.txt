cmake_minimum_required(VERSION 3.15)
project(assignment2
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SFML/CMakeLists.txt")
  message("${CMAKE_CURRENT_SOURCE_DIR}/SFML/CMakeLists.txt")
  message(FATAL_ERROR "Please initialize submodules using:\n  git submodule update --init --recursive")
endif()

set(SFML_BUILD_AUDIO FALSE)
set(SFML_BUILD_NETWORK FALSE)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(dependencies/SFML)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/imgui.cpp")
  message("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/imgui.cpp")
  message(FATAL_ERROR "Please initialize submodules using:\n  git submodule update --init --recursive")
endif()

# imgui does not contain cmakelist but imgui-sfml does and relies on these varaiables
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui")
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

# Create SMFL aliases that ImGui-SFML expects = because we build from subdirectory
add_library(SFML::Graphics ALIAS sfml-graphics)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui-sfml/CMakeLists.txt")
  message("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui-sfml/CMakeLists.txt")
  message(FATAL_ERROR "Please initialize submodules using:\n  git submodule update --init --recursive")
endif()

add_subdirectory(dependencies/imgui-sfml)

# Copy DLLs needed for runtime on Windows
if(WIN32)
  if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET assignment2 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:ImGui-SFML::ImGui-SFML>
          $<TARGET_FILE:sfml-graphics>
          $<TARGET_FILE:sfml-window>
          $<TARGET_FILE:sfml-system>
          $<TARGET_FILE_DIR:assignment2>)
  endif()
endif()

