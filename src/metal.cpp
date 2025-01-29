#include "../include/metal.hpp"
#include "../include/particle.hpp"
#include "../include/config.hpp"

#include <Foundation/NSString.hpp>
#include <Metal/MTLResource.hpp>
#include <cassert>

void MetalCompute::init_metal() {
    setDefaults();
    createDevice();
    createPipeline();
    createCommandQueue();
    createBuffers();
}

void MetalCompute::updateBuffers(std::vector<Particle> vec_particles, std::vector<int> vec_indices, std::vector<int> vec_offsets, Constants c) {
    num_particles = vec_particles.size();
    num_indices = vec_indices.size();
    num_offsets = vec_offsets.size();

    if (num_particles > particles_buf_max) {
        particles_buf_max = num_particles * 2;
        int deltas_buf_max = particles_buf_max * 3;

        particles->release();
        deltas->release();

        particles = device->newBuffer(sizeof(Particle) * particles_buf_max, MTL::ResourceStorageModeShared);
        deltas = device->newBuffer(sizeof(glm::vec3) * deltas_buf_max, MTL::ResourceStorageModeShared);
    }
    if (num_indices > indices_buf_max) {
        indices_buf_max = num_indices * 2;
        indices->release();
        indices = device->newBuffer(sizeof(int) * indices_buf_max, MTL::ResourceStorageModeShared);
    }
    if (num_offsets > offsets_buf_max) {
        offsets_buf_max = num_offsets * 2;
        offsets->release();
        offsets = device->newBuffer(sizeof(int) * offsets_buf_max, MTL::ResourceStorageModeShared);
    }
    if(num_cellCounts < c.grid_width * c.grid_height) {
        cellCounts->release();
        cellCounts = device->newBuffer(sizeof(int) * c.grid_width * c.grid_height, MTL::ResourceStorageModeShared);
        num_cellCounts = c.grid_width * c.grid_height;
    }

    memcpy(particles->contents(), vec_particles.data(), sizeof(Particle) * num_particles);
    memcpy(indices->contents(), vec_indices.data(), sizeof(int) * num_indices);
    memcpy(offsets->contents(), vec_offsets.data(), sizeof(int) * num_offsets);
    memcpy(constants->contents(), &c, sizeof(Constants));
    memset(deltas->contents(), 0, sizeof(glm::vec3) * num_particles * 3);

    particles->didModifyRange(NS::Range{0, sizeof(Particle) * num_particles});
    indices->didModifyRange(NS::Range{0, sizeof(int) * num_indices});
    offsets->didModifyRange(NS::Range{0, sizeof(int) * num_offsets});
    constants->didModifyRange(NS::Range{0, sizeof(Constants)});
    deltas->didModifyRange(NS::Range({0, sizeof(float) * num_particles * 3 }));
}

void MetalCompute::loadFromBuffers(std::vector<Particle> &vec_particles) {
    assert(vec_particles.size() == num_particles);

    memcpy(vec_particles.data(), particles->contents(), sizeof(Particle) * num_particles);
}

void MetalCompute::setDefaults() {
    num_particles = 0;
    particles_buf_max = 100;

    num_indices = 0;
    indices_buf_max = 100;

    num_offsets = 0;
    offsets_buf_max = 100;
}

void MetalCompute::createDevice() {
    device = MTL::CreateSystemDefaultDevice();

    NS::Error *e = nullptr;

   
    commandQueue = device->newCommandQueue();

    if (!commandQueue) {
        throw std::runtime_error("Failed to create command queue");
    }
}

void MetalCompute::createPipeline() {
    NS::Error *e = nullptr;
    NS::String *shader_path = NS::String::string("shaders/shaders.metallib", NS::UTF8StringEncoding);
    MTL::Library *library = device->newLibrary(shader_path, &e);

    if (!library) {
        std::cout << e << std::endl;
        throw std::runtime_error("Failed to create library");
    }

    NS::String *functionStringDeltas = NS::String::string("calculate_collisions_deltas", NS::UTF8StringEncoding);
    MTL::Function *functionDeltas = library->newFunction(functionStringDeltas);

    NS::String *functionStringCollisions = NS::String::string("handle_collisions", NS::UTF8StringEncoding);
    MTL::Function *functionCollisions = library->newFunction(functionStringCollisions);

    NS::String *functionStringBoxCollisions = NS::String::string("boxConstraint", NS::UTF8StringEncoding);
    MTL::Function *functionBoxCollisions = library->newFunction(functionStringBoxCollisions);

    NS::String *functionStringUpdate = NS::String::string("update_particles", NS::UTF8StringEncoding);
    MTL::Function *functionUpdate = library->newFunction(functionStringUpdate);

    library->release();

    if (functionCollisions == nullptr || functionBoxCollisions == nullptr || functionUpdate == nullptr || functionDeltas == nullptr) {
        std::cout << e << std::endl;
        throw std::runtime_error("Failed to create function");
    }

    pipelineStateCollisions = device->newComputePipelineState(functionCollisions, &e);
    pipelineStateBoxCollisions = device->newComputePipelineState(functionBoxCollisions, &e);
    pipelineStateUpdate = device->newComputePipelineState(functionUpdate, &e);
    pipelineStateDelta = device->newComputePipelineState(functionDeltas, &e);

    functionCollisions->release();
    functionBoxCollisions->release();
    functionUpdate->release();
    functionDeltas->release();

    if (!pipelineStateCollisions || !pipelineStateBoxCollisions || !pipelineStateUpdate || !pipelineStateDelta) {
        throw std::runtime_error("Failed to create pipeline state");
    }
}

void MetalCompute::createCommandQueue() {
    NS::Error *e = nullptr;
    commandQueue = device->newCommandQueue();
    if (!commandQueue) {
        throw std::runtime_error("Failed to create command queue");
    }
}

void MetalCompute::createBuffers() {
    NS::Error *e = nullptr;

    indices = device->newBuffer(sizeof(int) * indices_buf_max, MTL::ResourceStorageModeShared);
    if (!indices) {
        throw std::runtime_error("Failed to create indices buffer");
    }

    offsets = device->newBuffer(sizeof(int) * offsets_buf_max, MTL::ResourceStorageModeShared);
    if (!offsets) {
        throw std::runtime_error("Failed to create offsets buffer");
    }

    particles = device->newBuffer(sizeof(Particle) * particles_buf_max, MTL::ResourceStorageModeShared);
    if (!particles) {
        throw std::runtime_error("Failed to create particles buffer");
    }

    constants = device->newBuffer(sizeof(Constants), MTL::ResourceStorageModeShared);
    if (!constants) {
        throw std::runtime_error("Failed to create constants buffer");
    }

    deltas = device->newBuffer(sizeof(glm::vec3) * particles_buf_max * 3, MTL::ResourceStorageModeShared);
    if(!deltas) {
        throw std::runtime_error("Failed to create deltas buffer");
    }

    cellCounts = device->newBuffer(sizeof(int), MTL::ResourceStorageModeShared);
    if(!cellCounts) {
        throw std::runtime_error("Failed to create cellCounts buffer");
    }

}

void MetalCompute::handle_box_constraints() {
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder *encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(pipelineStateBoxCollisions);
    encoder->setBuffer(particles, 0, 0);
    encoder->setBuffer(constants, 0, 3);

    MTL::Size gridSize = MTL::Size{(NS::UInteger)num_particles, 1, 1};

    NS::UInteger threadGroupSize = pipelineStateBoxCollisions->maxTotalThreadsPerThreadgroup();

    NS::UInteger threadGroupDim = (threadGroupSize > num_particles) ? num_particles : threadGroupSize;

    MTL::Size threadGroup = MTL::Size{threadGroupDim, 1, 1};

    encoder->dispatchThreads(gridSize, threadGroup);

    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

void MetalCompute::update_particles() {
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder *encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(pipelineStateUpdate);
    encoder->setBuffer(particles, 0, 0);
    encoder->setBuffer(constants, 0, 3);

    MTL::Size gridSize = MTL::Size{(NS::UInteger)num_particles, 1, 1};

    NS::UInteger threadGroupSize = pipelineStateUpdate->maxTotalThreadsPerThreadgroup();

    NS::UInteger threadGroupDim = (threadGroupSize > num_particles) ? num_particles : threadGroupSize;

    MTL::Size threadGroup = MTL::Size{threadGroupDim, 1, 1};

    encoder->dispatchThreads(gridSize, threadGroup);

    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

void MetalCompute::handle_collisions() {
    MTL::CommandBuffer *commandBuffer = commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder *encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(pipelineStateDelta);
    encoder->setBuffer(particles, 0, 0);
    encoder->setBuffer(indices, 0, 1);
    encoder->setBuffer(offsets, 0, 2);
    encoder->setBuffer(constants, 0, 3);
    encoder->setBuffer(deltas, 0, 4);

    MTL::Size gridSize = MTL::Size{(NS::UInteger)num_particles, 1, 1};

    NS::UInteger threadGroupSize = pipelineStateDelta->maxTotalThreadsPerThreadgroup();

    NS::UInteger threadGroupDim = (threadGroupSize > num_particles) ? num_particles : threadGroupSize;

    MTL::Size threadGroup = MTL::Size{threadGroupDim, 1, 1};

    encoder->dispatchThreads(gridSize, threadGroup);

    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    commandBuffer = commandQueue->commandBuffer();
    encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(pipelineStateCollisions);
    encoder->setBuffer(particles, 0, 0);
    encoder->setBuffer(indices, 0, 1);
    encoder->setBuffer(offsets, 0, 2);
    encoder->setBuffer(constants, 0, 3);
    encoder->setBuffer(deltas, 0, 4);

    gridSize = MTL::Size{(NS::UInteger)num_particles, 1, 1};

    threadGroupSize = pipelineStateCollisions->maxTotalThreadsPerThreadgroup();

    threadGroupDim = (threadGroupSize > num_particles) ? num_particles : threadGroupSize;

    threadGroup = MTL::Size{threadGroupDim, 1, 1};

    encoder->dispatchThreads(gridSize, threadGroup);

    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

}

MetalCompute::~MetalCompute() {
    device->release();
    pipelineStateBoxCollisions->release();
    pipelineStateCollisions->release();
    commandQueue->release();
    indices->release();
    offsets->release();
    particles->release();
    constants->release();
}
