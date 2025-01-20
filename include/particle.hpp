#pragma once

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <random>

struct Particle {
    glm::vec2 position;
    glm::vec2 position_last;
    glm::vec2 acceleration;

    float mass;
    float radius;
};

Particle create_particle(glm::vec2 position, glm::vec2 position_last, glm::vec2 acceleration, float mass, float radius);

void update_particle(Particle &p, float dt);
void accelerate_particle(Particle &p, glm::vec2 a);
void set_particle_velocity(Particle &p, glm::vec2 v, float dt);
void add_particle_velocity(Particle &p, glm::vec2 v, float dt);

glm::vec2 get_particle_velocity(Particle &p);
