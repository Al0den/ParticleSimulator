#include "../include/particle.hpp"

Particle create_particle(glm::vec3 position, glm::vec3 position_last, glm::vec3 acceleration, float mass, float radius) {
    Particle p;
    p.position = position;
    p.position_last = position_last;
    p.acceleration = acceleration;
    p.mass = mass;

    return p;
}

void update_particle(Particle &p, float dt) {
    glm::vec3 displacement = p.position - p.position_last;
    p.position_last = p.position;
    p.position = p.position + displacement + p.acceleration * (dt * dt);
    p.acceleration = {};
}

void accelerate_particle(Particle &p, glm::vec3 a) {
    p.acceleration += a;
}

void set_particle_velocity(Particle &p, glm::vec3 v, float dt) {
    p.position_last = p.position - v * dt;
}

void add_particle_velocity(Particle &p, glm::vec3 v, float dt) {
    p.position_last -= v * dt;
}

glm::vec3 get_particle_velocity(Particle &p) {
    return p.position - p.position_last;
}
