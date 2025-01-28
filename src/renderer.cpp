#include "../include/renderer.hpp"
#include "../include/config.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2/SDL_vulkan.h>

Renderer::Renderer() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Failed to initialize SDL");
    }

    if(SDL_Vulkan_LoadLibrary(nullptr) != 0) {
        throw std::runtime_error("Failed to load Vulkan library");
    }

    window = SDL_CreateWindow("Particle Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    if (!window) {
        throw std::runtime_error("Failed to create SDL window");
    }

    sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl_renderer) {
        throw std::runtime_error("Failed to create SDL renderer");
    }
}

Renderer::~Renderer() {
    if (circleTexture) {
        SDL_DestroyTexture(circleTexture);
    }
    if (sdl_renderer) {
        SDL_DestroyRenderer(sdl_renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void Renderer::drawFrame(std::vector<Particle> particles) {
    for (auto &p : particles) {
        SDL_Rect rect = {
            (int)(p.position.x - p.radius),
            (int)(p.position.y - p.radius),
            (int)(p.radius * 2),
            (int)(p.radius * 2)
        };
        SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(sdl_renderer, &rect);
    }
}


