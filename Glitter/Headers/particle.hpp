#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>


#include <common/shader_particle.hpp>
#include <common/texture_particle.hpp>
#include <common/controls.hpp>

// CPU representation of a particle
struct Particle {
    glm::vec3 pos, speed;
    unsigned char r, g, b, a; // Color
    float size, angle, weight;
    float life; // Remaining life of the particle. if <0 : dead and unused.
    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

    bool operator<(const Particle& that) const {
        // Sort in reverse order : far particles drawn first.
        return this->cameradistance > that.cameradistance;
    }
};
const int MaxParticles = 100000;
const GLfloat g_vertex_buffer_data[] = {
     -0.5f, -0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
     -0.5f,  0.5f, 0.0f,
      0.5f,  0.5f, 0.0f,
};
struct ParticleGenerator {
public:
    //ParticleGenerator* Particles;

    Particle ParticlesContainer[MaxParticles];
    int LastUsedParticle = 0;

    // The VBO containing the 4 vertices of the particles.
 // Thanks to instancing, they will be shared by all particles.

    GLuint billboard_vertex_buffer;
    GLuint particles_position_buffer;
    GLuint particles_color_buffer;
    GLuint VertexArrayID;
    GLuint programID = LoadShaders("../Glitter/Shaders/Particle.vertexshader", "../Glitter/Shaders/Particle.fragmentshader");

    // Vertex shader
    GLuint CameraRight_worldspace_ID;
    GLuint CameraUp_worldspace_ID;
    GLuint ViewProjMatrixID;

    // fragment shader
    GLuint TextureID;
    GLuint Texture;

    GLfloat* g_particule_position_size_data;
    GLubyte* g_particule_color_data;
    void init() {

        g_particule_position_size_data = new GLfloat[MaxParticles * 4];
        g_particule_color_data = new GLubyte[MaxParticles * 4];
        programID = LoadShaders("../Glitter/Shaders/Particle.vertexshader", "../Glitter/Shaders/Particle.fragmentshader");

        // Vertex shader
        CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
        CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
        ViewProjMatrixID = glGetUniformLocation(programID, "VP");

        // fragment shader
        TextureID = glGetUniformLocation(programID, "myTextureSampler");
        Texture = loadDDS("../Glitter/objects/particle.DDS");
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);


        // Create and compile our GLSL program from the shaders

        for (int i = 0; i < MaxParticles; i++) {
            ParticlesContainer[i].life = -1.0f;
            ParticlesContainer[i].cameradistance = -1.0f;
        }

        glGenBuffers(1, &billboard_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

        // The VBO containing the positions and sizes of the particles

        glGenBuffers(1, &particles_position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        // Initialize with empty (NULL) buffer : it will be updated later, each frame.
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

        // The VBO containing the colors of the particles

        glGenBuffers(1, &particles_color_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        // Initialize with empty (NULL) buffer : it will be updated later, each frame.
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
    // Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
    int FindUnusedParticle() {

        for (int i = LastUsedParticle; i < MaxParticles; i++) {
            if (ParticlesContainer[i].life < 0) {
                LastUsedParticle = i;
                return i;
            }
        }

        for (int i = 0; i < LastUsedParticle; i++) {
            if (ParticlesContainer[i].life < 0) {
                LastUsedParticle = i;
                return i;
            }
        }

        return 0; // All particles are taken, override the first one
    }

    void SortParticles() {
        std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
    }


};
#endif