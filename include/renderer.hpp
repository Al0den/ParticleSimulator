#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "../include/particle.hpp"
#include "../utils/thread_pool.hpp"

class Renderer {
public:
    // Remove copy and move constructors
    Renderer(const Renderer&) = delete;

    Renderer();

    sf::RenderWindow& getWindow() {
        return window;
    }

    void drawFrame(std::vector<Particle> particles);

    ThreadPool threader{std::thread::hardware_concurrency() * 4};

private:
    sf::RenderWindow window;
};

