add_executable(assignment2 
main.cpp
Game.cpp
Game.h
Vec2.hpp
Entity.hpp
EntityManager.hpp
Components.hpp
config.txt
)
target_link_libraries(assignment2 PRIVATE ImGui-SFML::ImGui-SFML)