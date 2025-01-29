#include <time.h>
#include <chrono>
#include <thread>
#include <cmath>

#include "/Users/alois/Desktop/projects/CustomUtils/print.hpp"
#include "../include/simulation.hpp"

#define USE_SHADER true

Simulation::Simulation(MetalCompute &metalHandler, int width, int height) : metalHandler(metalHandler) {
    setWindowSize(width, height);
    particles.push_back(Particle{
        glm::vec3(width/2, height/2, 0),   // position
        glm::vec3(width/2, height/2, 0),   // position_last
        glm::vec3{},       // acceleration
        1.0f,                  // mass
        2                   // radius
    });
    init_grid();
    return;
}

int max(int a, int b) {
    if (a > b) return a;
    return b;
}

void Simulation::run(int num_iterations, float dt, int frameNum) {
    Constants constants = {
        .num_particles = (int)particles.size(),
        .num_indices = (int)cellIndices.size(),
        .num_offsets = (int)cellOffsets.size(),
        .grid_size = grid_size,
        .width = width,
        .height = height,
        .dt = dt,
        .grid_width = grid_width,
        .grid_height = grid_height
    };
     
    for (int i=0; i<num_iterations; i++) {
        if(USE_SHADER) {
            metalHandler.updateBuffers(particles, cellIndices, cellOffsets, constants);
            metalHandler.update_particles();
            metalHandler.handle_collisions();
            metalHandler.handle_box_constraints();
            metalHandler.loadFromBuffers(particles);
        } else {
            updateParticles(dt);
            handleCollisions();
            boxConstraint();
        }

        update_grid();
    }
}

void Simulation::updateParticles(float dt) {
    for (int i=0; i<(int)particles.size(); i++) {
        Particle& p = particles[i];
        accelerate_particle(p, {0, 0.000098, 0});
        update_particle(p, dt);
    }
}

void Simulation::handleGridCollisions(int x, int y) {
    static float minDistSquared = pow((particles[0].radius * 2), 2);

    static std::vector<glm::vec2> toCheckOffsets = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1}, {-1, 1} 
    };

    int cellIndex = grid_index(x, y);
    int start = cellOffsets[cellIndex];
    int end   = cellOffsets[cellIndex + 1];

    for (int i = start; i < end; i++) {
        int p1Index = cellIndices[i];
        Particle& p1 = particles[p1Index];

        for (auto& offset : toCheckOffsets) {
            int nx = x + offset.x;
            int ny = y + offset.y;

            if (nx < 0 || nx >= grid_width || ny < 0 || ny >= grid_height) {
                continue;
            }

            int neighborIndex = grid_index(nx, ny);
            int nStart = cellOffsets[neighborIndex];
            int nEnd   = cellOffsets[neighborIndex + 1];

            for (int j = nStart; j < nEnd; j++) {
                int p2Index = cellIndices[j];

                if (p1Index == p2Index) continue;

                Particle& p2 = particles[p2Index];

                glm::vec3 v = p1.position - p2.position;
                float distSquared = v.x * v.x + v.y * v.y;

                if (distSquared < minDistSquared) {
                    float dist = std::sqrt(distSquared);
                    if (dist < 1e-8f) dist = 1e-8f;

                    glm::vec3 n = v / dist;

                    float overlap = 0.25f * ((p1.radius + p2.radius) - dist);

                    if (overlap > 0.0f) {
                        p1.position += n * overlap;
                        p2.position -= n * overlap;
                    }
                }
            }
        }
    }
}

void Simulation::handleCollisions() {
    threader.Parallel(grid_height, [&](int start, int end) {
        for (int i=start; i<end; i += 2) {
            for (int j=0; j<grid_width; j++) {
                handleGridCollisions(j, i);
            }
        }
    });
        
    threader.Parallel(grid_height, [&](int start, int end) {
        for (int i=start; i<end; i += 2) {
            for (int j=0; j<grid_width; j++) {
                handleGridCollisions(j, i+1);
            }
        }
    });
}

void Simulation::handleCollisionsGeneral() {
    for (int i=0; i<(int)particles.size(); i++) {
        for (int j=i+1; j<(int)particles.size(); j++) {
            if (i == j) continue;

            Particle& p1 = particles[i];
            Particle& p2 = particles[j];

            float dist = sqrt(pow(p1.position.x - p2.position.x, 2) + pow(p1.position.y - p2.position.y, 2));
            if (dist < 1e-5) dist = 1e-5;
            float min_dist = p1.radius + p2.radius;
            glm::vec3 v = p1.position - p2.position;

            if (dist < p1.radius + p2.radius) {
                glm::vec3 n = v / dist;
                float delta = 0.5f * (min_dist - dist);

                p1.position += n * 0.5f * delta;
                p2.position -= n * 0.5f * delta;
            }
        }
    }
}

void Simulation::circleConstraint() {
    int center_x = width / 2;
    int center_y = height / 2;
    int radius = 350;

    glm::vec3 boundary_center = {static_cast<float>(center_x), static_cast<float>(center_y), 0};

    threader.Parallel(particles.size(), [&](int start, int end) {
        for (int i=start; i<end; i++) {
            Particle& p = particles[i];
            float dist = std::sqrt(std::pow(p.position.x - center_x, 2) + std::pow(p.position.y - center_y, 2));
            glm::vec3 r = boundary_center - p.position;
            if (dist > radius - p.radius) {
                glm::vec3 n = r / dist; // Normalize
                glm::vec3 perp = {-n.y, n.x, 0};
                glm::vec3 vel = get_particle_velocity(p);
                p.position = boundary_center - n * (radius - p.radius);
                set_particle_velocity(p, dampening * 2.0f * (vel.x * perp.x + vel.y * perp.y) * perp - vel, 1.0f);
            }
        }
    });
}

void Simulation::boxConstraint() {
    for (int i = 0; i < (int)particles.size(); i++) {
        Particle& p = particles[i];

        glm::vec3 vel = p.position - p.position_last;

        if (p.position.x < p.radius) {
            p.position.x = p.radius;
            vel.x = -dampening * vel.x;  // Flip and dampen x-velocity
        }
        else if (p.position.x > width - p.radius) {
            p.position.x = width - p.radius;
            vel.x = -dampening * vel.x;
        }

        if (p.position.y < p.radius) {
            p.position.y = p.radius;
            vel.y = -dampening * vel.y;  // Flip and dampen y-velocity
        }
        else if (p.position.y > height - p.radius) {
            p.position.y = height - p.radius;
            vel.y = -dampening * vel.y;
        }

        set_particle_velocity(p, vel, 1.0f);
    }
}

void Simulation::init_grid() {
    cellOffsets.resize(grid_width * grid_height + 1, 0);
    cellIndices.clear();
   
    update_grid();
}

void Simulation::update_grid() {
    std::vector<int> cellCounts(num_cells(), 0);

    for (int i=0; i < (int)particles.size(); i++) {
        Particle& p = particles[i];

        int gx = (int)(p.position.x / grid_size);
        int gy = (int)(p.position.y / grid_size);

        int cellIndex = gx + gy * grid_width;
        cellCounts[cellIndex]++;
    }

    cellOffsets[0] = 0;
    for (int i = 1; i <= num_cells(); i++) {
        cellOffsets[i] = cellOffsets[i - 1] + cellCounts[i - 1];
    }

    cellIndices.resize(cellOffsets[num_cells()]);

    memset(cellCounts.data(), 0, sizeof(int) * num_cells());

    for (int i = 0; i < (int)particles.size(); i++) {
        Particle& p = particles[i];
        int gx = (int)(p.position.x / grid_size);
        int gy = (int)(p.position.y / grid_size);

        int cellIndex = gx + gy * grid_width;

        int offset = cellOffsets[cellIndex];
        int writePos = offset + cellCounts[cellIndex];

        cellIndices[writePos] = i;
        cellCounts[cellIndex]++;
    }
}


void Simulation::setWindowSize(int width, int height) {
    this->width = width;
    this->height = height;
    this->grid_width = width / grid_size;
    this->grid_height = height / grid_size;
}
