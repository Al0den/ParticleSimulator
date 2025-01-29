#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

#include "../include/particle.hpp"
#include "../include/config.hpp"
#include "../utils/thread_pool.hpp"

class Renderer {
public:
    // Remove copy and move constructors
    Renderer(const Renderer&) = delete;

    Renderer();
    ~Renderer();

    SDL_Window* getWindow() {
        return window;
    }

    void drawFrame(std::vector<Particle> particles);

    ThreadPool threader{std::thread::hardware_concurrency() * 4};

    const uint get_width() { return window_width; }
    const uint get_height() { return window_height; }

    SDL_Window *get_window() { return window; }
    SDL_Renderer *get_renderer() { return sdl_renderer; }

    float fps;

    TTF_Font* font;
    SDL_Texture* fpsTexture;
    SDL_Rect fpsRect;

private:
    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    uint window_width = DEFAULT_WIDTH;
    uint window_height = DEFAULT_HEIGHT;
    SDL_Texture* circleTexture;

    void updateFpsText(float fps);

    void createCircleTexture();

    void drawCircle(int centerX, int centerY, int radius);
};

