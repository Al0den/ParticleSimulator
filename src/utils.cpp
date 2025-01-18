#include "../include/utils.hpp"

sf::Texture createCircleTexture(unsigned int diameter)
{
    // Create an image with a transparent background
    sf::Image image({diameter, diameter}, sf::Color::Transparent);

    // Center and radius
    float radius = diameter / 2.f;
    float radiusSq = radius * radius;

    // Fill pixels that lie within the circle
    for (unsigned int y = 0; y < diameter; ++y)
    {
        for (unsigned int x = 0; x < diameter; ++x)
        {
            float dx = static_cast<float>(x) - radius;
            float dy = static_cast<float>(y) - radius;
            float distSq = dx * dx + dy * dy;

            if (distSq <= radiusSq)
            {
                image.setPixel({x, y}, sf::Color::White);
            }
        }
    }

    sf::Texture texture;
    if (!texture.loadFromImage(image))
    {
        throw std::runtime_error("Failed to create circle texture");
    }

    texture.setSmooth(true);

    return texture;
}
