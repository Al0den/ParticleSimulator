#include <time.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "../include/simulation.hpp"
#include "../include/utils.hpp"

int radius = 2;
int spawnDelay = 1;

Simulation::Simulation(Renderer& renderer) : renderer(renderer) {
    particles.push_back(Particle({400, 400}, radius));
    init_grid();
    return;
}

int max(int a, int b) {
    if (a > b) return a;
    return b;
}

void Simulation::run() {
    auto prevTime = std::chrono::high_resolution_clock::now();

    sf::RenderWindow& window = renderer.getWindow();
    sf::Font font;

    if(!font.openFromFile("/Users/alois/Downloads/helvetica-255/Helvetica.ttf")) {
        printf("Failed to load font\n");
        return;
    }

    sf::Text fpsText(font, "FPS: ");
    fpsText.setFont(font);
    fpsText.setCharacterSize(18); // Set font size
    fpsText.setFillColor(sf::Color::Black); // Set text color
    fpsText.setPosition({10.f, 10.f}); // Top-left corner of the screen

    sf::Text numParticlesText(font, "Particles: " + std::to_string(particles.size()));
    numParticlesText.setFont(font);
    numParticlesText.setCharacterSize(18); // Set font size
    numParticlesText.setFillColor(sf::Color::Black); // Set text color
    numParticlesText.setPosition({10.f, 30.f}); // Top-left corner of the screen

    float liveFps;


    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if(event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                }
            }
        }

        window.clear(sf::Color::White);

        frameNum++;

        for (int i=0; i<mult; i++) {
            updateParticles();
            handleCollisions();
            boxConstraint();
            update_grid();
        }

        drawFrame();
        fpsText.setString("FPS: " + std::to_string((int)liveFps));
        numParticlesText.setString("Particles: " + std::to_string(particles.size()));
        window.draw(fpsText);
        window.draw(numParticlesText);

        window.display();

        int spawnX = 300;
        int spawnY = 100;

        int num_spawners = fmin(50, frameNum / fps * 5 + 1);
        //int num_spawners = 10;

        if(frameNum % spawnDelay == 0) {
            for (int i=0; i<num_spawners; i++) {
                
                float vx = sin(frameNum / 30.0) * 1.5;
                float vy = abs(cos(frameNum / 30.0) * 1.5); 

                particles.push_back(Particle({(float)spawnX, (float)spawnY + 2 * i * (radius+1)}, {vx, vy}, radius));
            }
        }

        int timeSpentMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - prevTime).count();

        std::this_thread::sleep_for(std::chrono::milliseconds(max(0, (int)(1000/this->fps) - timeSpentMS)));
        liveFps = 1.0 / std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - prevTime).count();
        prevTime = std::chrono::high_resolution_clock::now();
    }
}

void Simulation::updateParticles() {
    for (int i=0; i<(int)particles.size(); i++) {
        Particle& p = particles[i];
        p.accelerate({0, 0.000098});
        p.update(dt);
    }
}

void Simulation::drawFrame() {
    static sf::Texture circleTexture = createCircleTexture(64); // Creates a 64x64 circle texture
    int max_grid_index_x = WIDTH / grid_size;
    int max_grid_index_y = HEIGHT / grid_size;

    // Create a vertex array large enough to hold 6 vertices per particle (2 triangles per quad)
    sf::VertexArray circles(sf::PrimitiveType::Triangles, particles.size() * 6);

    threader.Parallel(particles.size(), [&](int start, int end) {
        for (int i=start; i<end; i++) {
            const auto& p = particles[i];
            float x = p.position.x;
            float y = p.position.y;
            float r = p.radius;
            // Compute fill color based on grid indexes (adjust as needed)
            sf::Color color(
                (255.f * (p.grid_x / static_cast<float>(max_grid_index_x))),
                (255.f * (p.grid_y / static_cast<float>(max_grid_index_y))),
                0
            );
            // Index in the vertex array
            unsigned int index = i * 6;
            // Define the six vertices (two triangles) for the quad
            circles[index + 0].position = sf::Vector2f(x - r, y - r); // Top-left
            circles[index + 1].position = sf::Vector2f(x + r, y - r); // Top-right
            circles[index + 2].position = sf::Vector2f(x + r, y + r); // Bottom-right
            circles[index + 3].position = sf::Vector2f(x - r, y - r); // Top-left
            circles[index + 4].position = sf::Vector2f(x + r, y + r); // Bottom-right
            circles[index + 5].position = sf::Vector2f(x - r, y + r); // Bottom-left

            float texSize = static_cast<float>(circleTexture.getSize().x); // Assuming square texture
            circles[index + 0].texCoords = sf::Vector2f(0.f,      0.f);
            circles[index + 1].texCoords = sf::Vector2f(texSize,  0.f);
            circles[index + 2].texCoords = sf::Vector2f(texSize,  texSize);
            circles[index + 3].texCoords = sf::Vector2f(0.f,      0.f);
            circles[index + 4].texCoords = sf::Vector2f(texSize,  texSize);
            circles[index + 5].texCoords = sf::Vector2f(0.f,      texSize);

            for (unsigned int j = 0; j < 6; ++j) {
                circles[index + j].color = color;
            }
        }
    });

    sf::RenderStates states;
    states.texture = &circleTexture;

    renderer.getWindow().draw(circles, states);
}

void Simulation::handleGridCollisions(int x, int y) {
    static float minDistSquared = pow((particles[0].radius * 2), 2);
    static std::vector<sf::Vector2i> toCheckOffsets = {{0, 0}, {1, 0}, {0, 1}, {1, 1}, {-1, 1}};

    for (auto &p1 : grid[GRID_INDEX(x, y)]) {
        for (auto &gridPos : toCheckOffsets) {
            int grid_x = x + gridPos.x;
            int grid_y = y + gridPos.y;
            if (grid_x < 0 || grid_x >= GRID_WIDTH || grid_y < 0 || grid_y >= GRID_HEIGHT) continue;

            for (auto &p2 : grid[GRID_INDEX(grid_x, grid_y)]) {
                Particle& particle1 = particles[p1];
                Particle& particle2 = particles[p2];
                
                sf::Vector2f v = particle1.position - particle2.position;
                float distSquared = v.x * v.x + v.y * v.y;

                if (distSquared < minDistSquared) {
                    float dist = sqrt(distSquared);
                    if (dist < 1e-4) dist = 1e-4;

                    sf::Vector2f n = v / dist;
                    n *= 0.25f * ((particle1.radius + particle2.radius) - dist);

                    particle1.position += n;
                    particle2.position -= n;
                }
            }
        }
    }
}

void Simulation::handleCollisions() {
    threader.Parallel(GRID_HEIGHT, [&](int start, int end) {
        for (int i=start; i<end; i += 2) {
            for (int j=0; j<GRID_WIDTH; j++) {
                handleGridCollisions(j, i);
            }
        }
    });
        
    threader.Parallel(GRID_HEIGHT, [&](int start, int end) {
        for (int i=start; i<end; i += 2) {
            for (int j=0; j<GRID_WIDTH; j++) {
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
            if (dist < 1e-6) dist = 1e-6;
            float min_dist = p1.radius + p2.radius;
            sf::Vector2f v = p1.position - p2.position;

            if (dist < p1.radius + p2.radius) {
                sf::Vector2f n = v / dist;
                float delta = 0.5f * (min_dist - dist);

                p1.position += n * 0.5f * delta;
                p2.position -= n * 0.5f * delta;
            }
        }
    }
}

void Simulation::circleConstraint() {
    int center_x = WIDTH / 2;
    int center_y = HEIGHT / 2;
    int radius = 350;

    sf::Vector2f boundary_center = {static_cast<float>(center_x), static_cast<float>(center_y)};

    threader.Parallel(particles.size(), [&](int start, int end) {
        for (int i=start; i<end; i++) {
            Particle& p = particles[i];
            float dist = std::sqrt(std::pow(p.position.x - center_x, 2) + std::pow(p.position.y - center_y, 2));
            sf::Vector2f r = boundary_center - p.position;
            if (dist > radius - p.radius) {
                sf::Vector2f n = r / dist; // Normalize
                sf::Vector2f perp = {-n.y, n.x};
                sf::Vector2f vel = p.getVelocity();
                p.position = boundary_center - n * (radius - p.radius);
                p.setVelocity(dampening * 2.0f * (vel.x * perp.x + vel.y * perp.y) * perp - vel, 1.0f);
            }
        }
    });
}

void Simulation::boxConstraint() {
    for (int i=0; i<particles.size(); i++) {
        Particle& p = particles[i];
        if (p.position.x < p.radius) {
            p.position.x = p.radius;
            sf::Vector2f vel = p.getVelocity();
            p.setVelocity({-dampening * vel.x, vel.y}, 1.0f);
        } else if (p.position.x > WIDTH - p.radius) {
            p.position.x = WIDTH - p.radius;
            sf::Vector2f vel = p.getVelocity();
            p.setVelocity({-dampening * vel.x, vel.y}, 1.0f);
        }
        if (p.position.y < p.radius) {
            p.position.y = p.radius;
            sf::Vector2f vel = p.getVelocity();
            p.setVelocity({vel.x, -dampening * vel.y}, 1.0f);
        } else if (p.position.y > HEIGHT - p.radius) {
            p.position.y = HEIGHT - p.radius;
            sf::Vector2f vel = p.getVelocity();
            p.setVelocity({vel.x, -dampening * vel.y}, 1.0f);
        }
    }
}

void Simulation::init_grid() {
    for (int i=0; i<WIDTH / grid_size; i++) {
        for(int j=0; j<HEIGHT / grid_size; j++) {
            grid.push_back(std::vector<int>());
        }
    }
    
    update_grid();
}

void Simulation::update_grid() {
    for(int i=0; i<WIDTH / grid_size; i++) {
        for(int j=0; j<HEIGHT / grid_size; j++) {
            grid[GRID_INDEX(i, j)].clear();
        }
    }
    
    for (int i=0; i<(int)particles.size(); i++) {
        Particle& p = particles[i];
        p.grid_x = p.position.x / grid_size;
        p.grid_y = p.position.y / grid_size;
        p.id = i;

        grid[GRID_INDEX(p.grid_x, p.grid_y)].push_back(i);
    }
}
