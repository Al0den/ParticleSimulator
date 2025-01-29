#include "../include/renderer.hpp"
#include "../include/config.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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

    if (TTF_Init() == -1) {
    throw std::runtime_error("Failed to initialize SDL_ttf");
    }

    font = TTF_OpenFont("/Users/alois/Library/Fonts/Monaco Nerd Font Complete Mono.ttf", 24);
    if (!font) {
        throw std::runtime_error("Failed to load font");
    }

    fpsTexture = nullptr;
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
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);

    // Reserve space to avoid reallocations
    std::vector<SDL_Rect> rects;
    rects.reserve(particles.size());

    for (const auto &p : particles) {
        rects.emplace_back(SDL_Rect{
            static_cast<int>(p.position.x - p.radius),
            static_cast<int>(p.position.y - p.radius),
            static_cast<int>(p.radius * 2),
            static_cast<int>(p.radius * 2)
        });
    }

    SDL_RenderFillRects(sdl_renderer, rects.data(), static_cast<int>(rects.size()));
}


