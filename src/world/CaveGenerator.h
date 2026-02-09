#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "Chunk.h"

constexpr uint32_t DEFAULT_WORLD_SEED = 1488u;

struct CaveConfig
{
  float cheeseFrequency    = 0.015f;  // Lower = bigger caves
  float cheeseThreshold    = -0.3f;   // Higher = more caves (negative means carve where noise is low)
  int   cheeseOctaves      = 2;
  float cheeseGain         = 0.5f;
  
  float spaghettiFrequency = 0.04f;   // Tunnel wiggliness
  float spaghettiThreshold = 0.08f;   // Tunnel width
  float spaghettiYStretch  = 0.6f;    // Horizontal bias
  
  float minCaveHeight      = 5.0f;    // Absolute minimum Y for caves
  float surfaceMargin      = 6.0f;    // Stay this many blocks below terrain surface
  
  float vegFrequency       = 0.08f;
  float vegThreshold       = 0.25f;
};

float caveDensity(int wx, int wy, int wz, int terrainHeight, uint32_t seed, const CaveConfig& cfg = CaveConfig{});

bool caveVegetationMask(int wx, int wy, int wz, uint32_t seed, const CaveConfig& cfg = CaveConfig{});

void applyCavesToBlocks(BlockID* blocks, const glm::ivec3& chunkPos, uint32_t worldSeed, 
                        const int* terrainHeights, const CaveConfig& cfg = CaveConfig{}, 
                        bool* outVegetationMask = nullptr);

void applyCavesToChunk(Chunk& c, uint32_t worldSeed, const int* terrainHeights, 
                       const CaveConfig& cfg = CaveConfig{}, bool* outVegetationMask = nullptr);
