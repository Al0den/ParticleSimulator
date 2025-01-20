#include "../include/renderer.hpp"
#include "../include/utils.hpp"
#include "../include/config.hpp"

#include <SFML/Graphics.hpp>

Renderer::Renderer() : window(sf::VideoMode({WIDTH, HEIGHT}), "Particle Simulator") {}


void Renderer::drawFrame(std::vector<Particle> particles) {
    static sf::Texture circleTexture = createCircleTexture(64); // Creates a 64x64 circle texture

    sf::VertexArray circles(sf::PrimitiveType::Triangles, particles.size() * 6);

    threader.Parallel(particles.size(), [&](int start, int end) {
        for (int i=start; i<end; i++) {
            const auto& p = particles[i];
            float x = p.position.x;
            float y = p.position.y;
            float r = p.radius;

            sf::Color color(255, 0, 0);

            unsigned int index = i * 6;

            circles[index + 0].position = sf::Vector2f(x - r, y - r); // Top-left
            circles[index + 1].position = sf::Vector2f(x + r, y - r); // Top-right
            circles[index + 2].position = sf::Vector2f(x + r, y + r); // Bottom-right
            circles[index + 3].position = sf::Vector2f(x - r, y - r); // Top-left
            circles[index + 4].position = sf::Vector2f(x + r, y + r); // Bottom-right
            circles[index + 5].position = sf::Vector2f(x - r, y + r); // Bottom-left

            float texSize = static_cast<float>(circleTexture.getSize().x); // Assuming square texture
            circles[index + 0].texCoords = sf::Vector2f(0.f,      0.f);
            circles[index + 1].texCoords = sf::Vector2f(texSize,  0.f);
            circles[index + 2].texCoords = sf::Vector2f(texSize,  texSize);
            circles[index + 3].texCoords = sf::Vector2f(0.f,      0.f);
            circles[index + 4].texCoords = sf::Vector2f(texSize,  texSize);
            circles[index + 5].texCoords = sf::Vector2f(0.f,      texSize);

            for (unsigned int j = 0; j < 6; ++j) {
                circles[index + j].color = color;
            }
        }
    });

    sf::RenderStates states;
    states.texture = &circleTexture;

    window.draw(circles, states);
}


