#pragma once

#include <SFML/Graphics.hpp>
#include "config.hpp"

class Renderer {
public:
    // Remove copy and move constructors
    Renderer(const Renderer&) = delete;

    Renderer();

    sf::RenderWindow& getWindow() {
        return window;
    }

private:
    sf::RenderWindow window;
};

