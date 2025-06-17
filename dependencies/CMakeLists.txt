cmake_minimum_required(VERSION 3.15)
include(FetchContent)

FetchContent_Declare(
  sfml
  URL https://github.com/SFML/SFML/archive/refs/tags/2.6.2.zip
)

add_subdirectory(sfml)

FetchContent_Declare(
  imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5.zip
)

FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  imgui-sfml
  URL https://github.com/SFML/imgui-sfml/archive/refs/tags/v2.6.1.zip
)
add_subdirectory(imgui-sfml)