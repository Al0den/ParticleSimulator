#include "../include/renderer.hpp"
#include "../include/simulation.hpp"
#include "../include/vulkan.hpp"

#include <chrono>

int main() {
    Renderer renderer;
    VulkanCompute vulkanHandler;
    Simulation simulation(vulkanHandler, renderer.get_width(), renderer.get_height());

    auto prevTime = std::chrono::high_resolution_clock::now();

    SDL_Window *window = renderer.get_window();
    SDL_Renderer *sdl_renderer = renderer.get_renderer();

    float liveFps;
    
    int frameNum = 0;

    int mult = 10;
    int fps = 60;
    float dt = (float)fps / mult;

    bool running = true;
    SDL_Event e;
    
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);
        SDL_RenderClear(sdl_renderer);

        frameNum++;
        
        simulation.run(mult, dt, frameNum);

        renderer.drawFrame(simulation.particles);

        SDL_RenderPresent(sdl_renderer);

        int spawnX = 100;
        int spawnY = 10;

        int num_spawners = fmin(100, frameNum / fps * 10 + 1);

        if(frameNum < 1000 && frameNum % 2 == 0) {
            for (int i=0; i<num_spawners; i++) {
                    
                float vx = 0.1;
                float vy = 0.1;
                
                simulation.particles.push_back(Particle{
                    glm::vec3(spawnX + 2 * i * (2 + 1), spawnY, 0),
                    glm::vec3(spawnX + 2 * i * (2 + 1) - vx, spawnY - vy, 0),
                    glm::vec3{},
                    1, 
                    2
                });
            }
        }

        int timeSpentMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - prevTime).count();

        std::this_thread::sleep_for(std::chrono::milliseconds(std::max(0, (int)(1000/fps) - timeSpentMS)));
        liveFps = 1.0 / std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - prevTime).count();
        prevTime = std::chrono::high_resolution_clock::now();
    }

    return 0;

}
