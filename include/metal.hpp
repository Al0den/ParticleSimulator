#pragma once

#include "../include/particle.hpp"
#include "../include/renderer.hpp"
#include "../include/config.hpp"

#include <Metal/Metal.hpp>

#include <stdexcept>
#include <vector>
#include <fstream>
#include <iostream>

struct MetalCompute {
public:
    MetalCompute() { init_metal(); }
    ~MetalCompute();

    void updateBuffers(std::vector<Particle> particles, std::vector<int> indices, std::vector<int> offsets, std::vector<float> constants);
    void loadFromBuffers(std::vector<Particle> &particles);

    void handle_collisions(int grid_size);
    void handle_box_constraints();
    void update_particles();
private:        
    void init_metal();
    
    void setDefaults();
    void createDevice();
    void createPipeline();
    void createCommandQueue();
    void createBuffers();

    MTL::Device *device;
    MTL::ComputePipelineState *pipelineStateDelta;
    MTL::ComputePipelineState *pipelineStateCollisions;
    MTL::ComputePipelineState *pipelineStateBoxCollisions;
    MTL::ComputePipelineState *pipelineStateUpdate;
    MTL::CommandQueue *commandQueue;

    MTL::Buffer *indices;
    MTL::Buffer *offsets;
    MTL::Buffer *particles;
    MTL::Buffer *constants;
    MTL::Buffer *deltas;

    int num_particles, particles_buf_max;
    int num_indices, indices_buf_max;
    int num_offsets, offsets_buf_max;

    int constants_size = 7;
};
