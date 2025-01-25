#pragma once

#include "../include/particle.hpp"

#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 position_last;
    glm::vec3 acceleration;

    float mass;
    float radius;
};

Particle create_particle(glm::vec3 position, glm::vec3 position_last, glm::vec2 acceleration, float mass, float radius);

void update_particle(Particle &p, float dt);
void accelerate_particle(Particle &p, glm::vec3 a);
void set_particle_velocity(Particle &p, glm::vec3 v, float dt);
void add_particle_velocity(Particle &p, glm::vec3 v, float dt);

glm::vec3 get_particle_velocity(Particle &p);
