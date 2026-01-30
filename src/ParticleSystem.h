#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    float maxLifetime;
    float size;
    float tileIndex;
    glm::vec4 color;
};

class ParticleSystem {
    public:
        ParticleSystem();
        ~ParticleSystem();

        void init();
        void update(float dt);
        void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);

        void spawnBlockBreakParticles(const glm::vec3& blockCenter, int tileIndex, int count = 12);
        void spawnParticle(const glm::vec3& pos, const glm::vec3& vel, float lifetime, float size, float tileIndex, const glm::vec4& color = glm::vec4(1.0f));

        void clear();

    private:
        std::vector<Particle> particles;
        std::mt19937 rng;

        GLuint vao;
        GLuint quadVBO;
        GLuint instanceVBO;
        GLuint shaderProgram;

        static constexpr size_t MAX_PARTICLES = 10000;
        static constexpr float GRAVITY = -20.0f;

        void compileShaders();
        void setupBuffers();
};