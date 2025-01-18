#pragma once

#include <SFML/Graphics.hpp>
#include <random>

class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;

    float mass = 1;
    float radius;
    int id = -1;

    int grid_x, grid_y;

    Particle() = default;
    Particle(sf::Vector2f position) {
        this->position = position;
        this->position_last = position;

    }
    Particle(sf::Vector2f position, float radius) {
        this->position = position;
        this->position_last = position;
        this->radius = radius;
    }
    Particle(sf::Vector2f position, sf::Vector2f velocity, float radius) {
        this->position = position;
        this->position_last = position - velocity;
        this->radius = radius;
    }

    void update(float dt) {
        sf::Vector2f displacement = position - position_last;
        position_last = position;
        position = position + displacement + acceleration * (dt * dt);
        acceleration = {};
    }

    void accelerate(sf::Vector2f a) {
        acceleration += a;
    }

    void setVelocity(sf::Vector2f v, float dt) {
        position_last = position - v * dt;
    }

    void addVelocity(sf::Vector2f v, float dt) {
        position_last -= v * dt;
    }

    sf::Vector2f getVelocity() {
        return position - position_last;
    }
};
