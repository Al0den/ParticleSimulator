#pragma once

#include "../include/particle.hpp"
#include "../include/config.hpp"
#include "../include/vulkan.hpp"

#include "../utils/thread_pool.hpp"

class Simulation {
public:
    float dampening = 0.6;

    int grid_size = 10;

    Simulation(VulkanCompute& vulkanHandler, int width, int height);
    ~Simulation() = default;

    void run(int num_iterations, float dt, int frameNum);
    
    void updateParticles(float dt);
    
    void boxConstraint();
    void circleConstraint();

    void handleCollisionsGeneral();
    void handleGridCollisions(int x, int y);
    void handleCollisions();

    void init_grid();
    void update_grid();

    std::vector<Particle> particles;

    ThreadPool threader{std::thread::hardware_concurrency() * 4};
    
    std::vector<int> cellOffsets;
    std::vector<int> cellIndices;

    int width, height, grid_width, grid_height;

    const inline int grid_index(int x, int y) { return x + grid_width * y; }
    const inline int num_cells() { return grid_width * grid_height; }

    void setWindowSize(int width, int height);

private:
    VulkanCompute &vulkanHandler;
};
