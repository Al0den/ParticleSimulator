#include "../include/renderer.hpp"
#include "../include/simulation.hpp"

int main() {
    Renderer renderer;
    Simulation simulation(renderer);

    simulation.run();

    printf("Clean exit");

    return 0;

}
