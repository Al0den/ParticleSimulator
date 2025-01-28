#include <metal_stdlib>
struct Particle {
    float posx;
    float posy;
    float posz;

    float pos_lastx;
    float pos_lasty;
    float pos_lastz;

    float ax;
    float ay;
    float az;

    float mass;
    float radius;
};

using namespace metal;

kernel void handle_collisions(device Particle *particles [[ buffer(0) ]], 
                              device const int *indices [[ buffer(1) ]],
                              device const int *offsets [[ buffer(2) ]],
                              device const float *constants [[ buffer(3) ]],
                              device float *deltas [[ buffer(4) ]], 
                              uint id [[ thread_position_in_grid ]]) {
    
    particles[id].posx += deltas[id * 3];
    particles[id].posy += deltas[id * 3 + 1];
    particles[id].posz += deltas[id * 3 + 2];
}

kernel void calculate_collisions_deltas(device Particle *particles [[ buffer(0) ]], 
                              device const int *indices [[ buffer(1) ]],
                              device const int *offsets [[ buffer(2) ]],
                              device const float *constants [[ buffer(3) ]],
                              device float *deltas [[ buffer(4) ]], 
                              uint id [[ thread_position_in_grid ]]) {

    float num_particles = constants[0];
    float num_indices = constants[1];
    float num_offsets = constants[2];
    float grid_size = constants[3];
    float width = constants[4];
    float height = constants[5];

    Particle p1 = particles[id];

    int grid_x = (int)(p1.posx / grid_size);
    int grid_y = (int)(p1.posy / grid_size);

    int checkOffsets[9][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1},
        {-1, 1},
        {-1 ,0},
        {-1, -1},
        {0, -1},
        {1, -1}
    };
    
    for (int i=0; i<num_particles; i++) {
        Particle p2 = particles[i];

        float dist_sq = (p1.posx - p2.posx) * (p1.posx - p2.posx) + (p1.posy - p2.posy) * (p1.posy - p2.posy);
        float min_dist = p1.radius + p2.radius;

        if(dist_sq < min_dist * min_dist) {
            float dist = sqrt(dist_sq);

            if(dist < 0.001) {
                dist = 0.001;
            }

            float dx = (p1.posx - p2.posx) / dist;
            float dy = (p1.posy - p2.posy) / dist;

            float delta = 0.25 * (min_dist - dist);

            deltas[id * 3] += delta * dx;
            deltas[id * 3 + 1] += delta * dy;
        }
    }
}

kernel void update_particles(device Particle *particles [[ buffer(0) ]],
                          device const float *constants [[ buffer(3) ]],
                          uint id [[ thread_position_in_grid ]]) {
    Particle p = particles[id];

    float dt = constants[6];

    p.ay += 0.000098;

    float displacement_x = p.posx - p.pos_lastx;
    float displacement_y = p.posy - p.pos_lasty;
    float displacement_z = p.posz - p.pos_lastz;

    p.pos_lastx = p.posx;
    p.pos_lasty = p.posy;
    p.pos_lastz = p.posz;

    p.posx = p.posx + displacement_x + p.ax * dt * dt;
    p.posy = p.posy + displacement_y + p.ay * dt * dt;
    p.posz = p.posz + displacement_z + p.az * dt * dt;

    p.ax = 0;
    p.ay = 0;
    p.az = 0;

    particles[id] = p;
}

kernel void boxConstraint(device Particle *particles [[ buffer(0) ]],
                          device const float *constants [[ buffer(3) ]],
                          uint id [[ thread_position_in_grid ]]) {
    Particle p = particles[id];

    float velx = p.posx - p.pos_lastx;
    float vely = p.posy - p.pos_lasty;
    float velz = p.posz - p.pos_lastz;

    float width = constants[4];
    float height = constants[5];
    float dt = constants[6];

    float dampening = 0.6;

    if (p.posx < p.radius) {
        p.posx = p.radius;
        velx = -velx * dampening;
    } else if (p.posx > width - p.radius) {
        p.posx = width - p.radius;
        velx = -velx * dampening;
    }
    if(p.posy < p.radius) {
        p.posy = p.radius;
        vely = -vely * dampening;
    } else if (p.posy > height - p.radius) {
        p.posy = height - p.radius;
        vely = -vely * dampening;
    }

    p.pos_lastx = p.posx - velx;
    p.pos_lasty = p.posy - vely;

    particles[id] = p;
}
