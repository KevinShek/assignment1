#include <iostream>
#include <memory>
#include <fstream>
#include <variant>

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#include "ReadConfig.h"
#include "ShapeStruct.h"
#include "UIStruct.h"
#include "ShapeProperties.h"

int main(int argc, char* argb[])
{
    // read the config file
    ReadConfig ConfigFile;
    UIStruct WindowSetting;
    std::string ConfigPath = "config.txt";
    // Initialise Positions of Shapes and Texts
    std::vector<ShapeStruct> ListofShapes = ConfigFile.ReadingConfig(ConfigPath, WindowSetting);
    std::cout << WindowSetting.FontPath;

    // Setup of Combo Box
    ShapeProperties ShapesProp;
    std::vector<const char*> items;
    for (auto& s : ListofShapes) {
        items.push_back(s.Name.c_str());
    }
    int item_index = 0;

    // create a new window
    sf::RenderWindow window(sf::VideoMode(WindowSetting.WindowWidth, WindowSetting.WindowHeight), "Bounce");
    window.setFramerateLimit(60); // limit frame rate to 60 fps

    // initialize IMGUI and create a clock used for its internal timing
    ImGui::SFML::Init(window);
    sf::Clock deltaClock;

    // scale the imgui ui and text size by 2
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;

    // main loop - conrinues for each frame while window is open
    while (window.isOpen())
    {
        // event handling
        sf::Event event;
        while (window.pollEvent(event))
        {
            // pass the event to imgui to be parsed
            ImGui::SFML::ProcessEvent(window, event);

            // this event triggers when the window is closed
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        // update imgui for this frame with the time that the last frame took
        ImGui::SFML::Update(window, deltaClock.restart());

        // draw the UI
        ImGui::Begin("Window title");
        //ImGui::Text("Window text!");
        ImGui::Combo("Shape", &item_index, items.data(), (int)items.size());
        //if (auto* circle = std::get_if<sf::CircleShape>(&ListofShapes[item_index].DrawableShape)) {
        //    ImGui::Checkbox("Draw Circle", &ListofShapes[item_index].DrawShape);
        //    ImGui::SameLine();
        //    ImGui::Checkbox("Draw text", &ListofShapes[item_index].DrawText);
        //    ImGui::SliderFloat("Radius", &ListofShapes[item_index].size1, 0.0f, 300.0f);
        //    ImGui::SliderInt("Sides", &ListofShapes[item_index].Segments, 3, 64);
        //    // set the circle properties, because they may have been updated with the ui
        //    circle->setPointCount(ListofShapes[item_index].Segments);
        //    circle->setRadius(ListofShapes[item_index].size1);
        //    ImGui::ColorEdit3("Color Circle", ListofShapes[item_index].Colour.data());
        //    if (ImGui::Button("Reset"))
        //    {
        //        circle->setPosition(ListofShapes[item_index].IntialPositionX, ListofShapes[item_index].IntialPositionX);
        //    }
        //    ImGui::End();
        //} 
        //else if (auto* rect = std::get_if<sf::RectangleShape>(&ListofShapes[item_index].DrawableShape)) {
        //    ImGui::Checkbox("Draw Rect", &ListofShapes[item_index].DrawShape);
        //    ImGui::SameLine();
        //    ImGui::Checkbox("Draw text", &ListofShapes[item_index].DrawText);
        //    ImGui::SliderFloat("Width", &ListofShapes[item_index].size1, 0.0f, 300.0f);
        //    ImGui::SliderFloat("Height", &ListofShapes[item_index].size2, 0.0f, 300.0f);
        //    // set the Rect properties, because they may have been updated with the ui
        //    rect->setSize(sf::Vector2f(ListofShapes[item_index].size1, ListofShapes[item_index].size2));
        //    ImGui::ColorEdit3("Color Circle", ListofShapes[item_index].Colour.data());
        //    if (ImGui::Button("Reset"))
        //    {
        //        rect->setPosition(ListofShapes[item_index].IntialPositionX, ListofShapes[item_index].IntialPositionX);

        //    }
        //    ImGui::End();
        //}
        //
        ImGui::Checkbox("Draw Shape", &ListofShapes[item_index].DrawShape);
        ImGui::SameLine();
        ImGui::Checkbox("Draw Text", &ListofShapes[item_index].DrawText);
        if (auto* circle = dynamic_cast<sf::CircleShape*>(ListofShapes[item_index].DrawableShape2.get())) {
            ImGui::SliderFloat("Radius", &ListofShapes[item_index].size1, 0.0f, 300.0f);
            ImGui::SliderInt("Sides", &ListofShapes[item_index].Segments, 3, 64);
            // set the circle properties, because they may have been updated with the ui
            circle->setPointCount(ListofShapes[item_index].Segments);
            circle->setRadius(ListofShapes[item_index].size1);
        }
        else if (auto* rect = dynamic_cast<sf::RectangleShape*>(ListofShapes[item_index].DrawableShape2.get())) {
            ImGui::SliderFloat("Width", &ListofShapes[item_index].size1, 0.0f, 300.0f);
            ImGui::SliderFloat("Height", &ListofShapes[item_index].size2, 0.0f, 300.0f);
            // set the Rect properties, because they may have been updated with the ui
            rect->setSize(sf::Vector2f(ListofShapes[item_index].size1, ListofShapes[item_index].size2));
        }
        ImGui::ColorEdit3("Color Circle", ListofShapes[item_index].Colour.data());
        if (ImGui::Button("Reset"))
        {
            items.clear();
            ListofShapes = ConfigFile.ReadingConfig(ConfigPath, WindowSetting);
            for (auto& s : ListofShapes) {
                items.push_back(s.Name.c_str());
            }
        }
        ImGui::End();
        
        // basic rendering function calls
        window.clear(); // clear the window of anything previously drawn
        for (auto& Shape : ListofShapes)
        {
            if (Shape.DrawShape) // draw the shape if the boolean is true
            {
                //std::visit([&](auto& ActualShape) {
                //    ActualShape.setFillColor(sf::Color(sf::Uint8(Shape.Colour[0] * 255), sf::Uint8(Shape.Colour[1] * 255), sf::Uint8(Shape.Colour[2] * 255)));
                //    //set new positions
                //    ActualShape.move(Shape.SpeedX, Shape.SpeedY);
                //    // grab from the bounding box after move
                //    sf::FloatRect bounds = ActualShape.getGlobalBounds();
                //    // Bounce off right and left window edges
                //    if (bounds.left + bounds.width > WindowSetting.WindowWidth || bounds.left < 0) {
                //        Shape.SpeedX = -Shape.SpeedX;
                //    }
                //    // Bounce off bottom and top window edges
                //    if (bounds.top + bounds.height > WindowSetting.WindowHeight || bounds.top < 0) {
                //        Shape.SpeedY = -Shape.SpeedY;
                //    }
                //    window.draw(ActualShape);
                //    if (Shape.DrawText) // draw the text if the boolean is true
                //    {
                //        // text would not be centered doing the method below so do two layers where the shape is one layer and the text is another layer
                //        //Shape.TextSMFL.setPosition(ActualShape.getPosition().x + (Shape.size1 / 2), ActualShape.getPosition().y + (Shape.size2 / 2));
                //        sf::FloatRect textBounds = Shape.TextSMFL.getLocalBounds();
                //        Shape.TextSMFL.setPosition(bounds.left + (bounds.width - textBounds.width) / 2, bounds.top + (bounds.height - textBounds.height) / 2 - 5);
                //        window.draw(Shape.TextSMFL);
                //    }
                //}, Shape.DrawableShape);
            
                Shape.DrawableShape2->setFillColor(sf::Color(sf::Uint8(Shape.Colour[0] * 255), sf::Uint8(Shape.Colour[1] * 255), sf::Uint8(Shape.Colour[2] * 255)));
                //set new positions
                Shape.DrawableShape2->move(Shape.SpeedX, Shape.SpeedY);
                // grab from the bounding box after move
                sf::FloatRect bounds = Shape.DrawableShape2->getGlobalBounds();
                // Bounce off right and left window edges
                if (bounds.left + bounds.width > WindowSetting.WindowWidth || bounds.left < 0) {
                    Shape.SpeedX = -Shape.SpeedX;
                }
                // Bounce off bottom and top window edges
                if (bounds.top + bounds.height > WindowSetting.WindowHeight || bounds.top < 0) {
                    Shape.SpeedY = -Shape.SpeedY;
                }
                window.draw(*Shape.DrawableShape2);
                if (Shape.DrawText) // draw the text if the boolean is true
                {
                    // text would not be centered doing the method below so do two layers where the shape is one layer and the text is another layer
                    //Shape.TextSMFL.setPosition(ActualShape.getPosition().x + (Shape.size1 / 2), ActualShape.getPosition().y + (Shape.size2 / 2));
                    sf::FloatRect textBounds = Shape.TextSMFL.getLocalBounds();
                    Shape.TextSMFL.setPosition(bounds.left + (bounds.width - textBounds.width) / 2, bounds.top + (bounds.height - textBounds.height) / 2 - 5);
                    window.draw(Shape.TextSMFL);
                }
            }
        }
        ImGui::SFML::Render(window); // draw the ui last so it's on top
        window.display();            // call the window display function
    }

    return 0;
}