#include <time.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "../include/simulation.hpp"
#include "../include/utils.hpp"

float radius = 2;
int spawnDelay = 2;

Simulation::Simulation(Renderer& renderer) : renderer(renderer) {
    particles.push_back(Particle{
        glm::vec3(400, 400, 0),   // position
        glm::vec3(400, 400, 0),   // position_last
        glm::vec3{},       // acceleration
        1.0f,                  // mass
        radius                   // radius
    });
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

        int spawnX = 100;
        int spawnY = 10;

        int num_spawners = fmin(80, frameNum / fps * 10 + 1);
        //int num_spawners = 1;

        if(frameNum % spawnDelay == 0) {
            for (int i=0; i<num_spawners; i++) {
                    
                float vx = 0.1;
                float vy = 0.1;
                
                particles.push_back(Particle{
                    glm::vec3(spawnX + 2 * i * (radius + 1), spawnY, 0),
                    glm::vec3(spawnX + 2 * i * (radius + 1) - vx, spawnY - vy, 0),
                    glm::vec3{},
                    1, 
                    radius
                });
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
        accelerate_particle(p, {0, 0.000098, 0});
        update_particle(p, dt);
    }
}

void Simulation::drawFrame() {
    static sf::Texture circleTexture = createCircleTexture(64); // Creates a 64x64 circle texture

    sf::VertexArray circles(sf::PrimitiveType::Triangles, particles.size() * 6);

    threader.Parallel(particles.size(), [&](int start, int end) {
        for (int i=start; i<end; i++) {
            const auto& p = particles[i];
            float x = p.position.x;
            float y = p.position.y;
            float r = p.radius;

            sf::Color color(255, 0, 0);

            unsigned int index = i * 6;

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

    static std::vector<sf::Vector2i> toCheckOffsets = {
        {0, 0},  // the cell itself
        {1, 0},  // cell to the right
        {0, 1},  // cell below
        {1, 1},  // diagonal cell
        {-1, 1}  // diagonal to the left, etc.
    };

    int cellIndex = GRID_INDEX(x, y);
    int start = cellOffsets[cellIndex];
    int end   = cellOffsets[cellIndex + 1];

    for (int i = start; i < end; i++) {
        int p1Index = cellIndices[i];
        Particle& p1 = particles[p1Index];

        for (auto& offset : toCheckOffsets) {
            int nx = x + offset.x;
            int ny = y + offset.y;

            if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) {
                continue;
            }

            int neighborIndex = GRID_INDEX(nx, ny);
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
    int center_x = WIDTH / 2;
    int center_y = HEIGHT / 2;
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
        else if (p.position.x > WIDTH - p.radius) {
            p.position.x = WIDTH - p.radius;
            vel.x = -dampening * vel.x;
        }

        if (p.position.y < p.radius) {
            p.position.y = p.radius;
            vel.y = -dampening * vel.y;  // Flip and dampen y-velocity
        }
        else if (p.position.y > HEIGHT - p.radius) {
            p.position.y = HEIGHT - p.radius;
            vel.y = -dampening * vel.y;
        }

        set_particle_velocity(p, vel, 1.0f);
    }
}

void Simulation::init_grid() {
    cellOffsets.resize(GRID_WIDTH * GRID_HEIGHT + 1, 0);
    cellIndices.clear();
   
    update_grid();
}

void Simulation::update_grid() {
    std::vector<int> cellCounts(NUM_CELLS, 0);

    for (int i=0; i < (int)particles.size(); i++) {
        Particle& p = particles[i];

        int gx = (int)(p.position.x / grid_size);
        int gy = (int)(p.position.y / grid_size);

        int cellIndex = gx + gy * GRID_WIDTH;
        cellCounts[cellIndex]++;
    }

    cellOffsets[0] = 0;
    for (int i = 1; i <= NUM_CELLS; i++) {
        cellOffsets[i] = cellOffsets[i - 1] + cellCounts[i - 1];
    }

    cellIndices.resize(cellOffsets[NUM_CELLS]);

    std::fill(cellCounts.begin(), cellCounts.end(), 0);

    for (int i = 0; i < (int)particles.size(); i++) {
        Particle& p = particles[i];
        int gx = (int)(p.position.x / grid_size);
        int gy = (int)(p.position.y / grid_size);

        int cellIndex = gx + gy * GRID_WIDTH;

        int offset = cellOffsets[cellIndex];
        int writePos = offset + cellCounts[cellIndex];

        cellIndices[writePos] = i;
        cellCounts[cellIndex]++;
    }
}
