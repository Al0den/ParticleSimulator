#include "../include/renderer.hpp"
#include <SFML/Graphics.hpp>

Renderer::Renderer() : window(sf::VideoMode({WIDTH, HEIGHT}), "Particle Simulator") {}
