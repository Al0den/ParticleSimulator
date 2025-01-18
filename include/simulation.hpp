#pragma once

#include "../include/renderer.hpp"
#include "../include/particle.hpp"
#include "../include/config.hpp"

#include "../utils/thread_pool.hpp"

#define GRID_WIDTH (WIDTH / grid_size)
#define GRID_HEIGHT (HEIGHT / grid_size)
#define GRID_INDEX(x, y) ((x) + (GRID_WIDTH * (y)))

class Simulation {
public:
    int mult = 10;
    int fps = 60;
    float dt = (float)fps / mult;

    float dampening = 0.98;

    Simulation(Renderer& renderer);

    void run();
    void drawFrame();
    
    void updateParticles();
    
    void boxConstraint();
    void circleConstraint();

    void handleCollisionsGeneral();
    void handleGridCollisions(int x, int y);
    void handleCollisions();

    void init_grid();
    void update_grid();

    std::vector<Particle> particles;

    ThreadPool threader{std::thread::hardware_concurrency() * 4};
    
    std::vector<std::vector<int>> grid;

private:
    Renderer& renderer;

    int frameNum = 0;
};
