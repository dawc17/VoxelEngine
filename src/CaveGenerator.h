#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "Chunk.h"

// Default world seed used for cave generation; adjust to change world layout.
constexpr uint32_t DEFAULT_WORLD_SEED = 1488u;

struct CaveConfig
{
  float shapeFrequency = 0.008f;   // Low freq base blobs
  int   shapeOctaves   = 3;
  float shapeGain      = 0.5f;
  float detailFrequency = 0.04f;   // High freq variation
  int   detailOctaves   = 2;
  float detailGain      = 0.55f;
  float detailAmplitude = 0.35f;   // Strength of detail noise contribution
  float baseThreshold   = 0.18f;   // Lower = more caves
  float verticalFadeStart = 50.0f; // Above this, caves fade out
  float verticalFadeEnd   = 70.0f; // At/above this, caves stop
  float floorJitterAmp    = 0.5f;  // Small vertical wobble
  float vegFrequency      = 0.08f; // Optional vegetation mask noise
  float vegThreshold      = 0.25f;
};

// Returns cave density at world position; >0 means carve to air.
float caveDensity(int wx, int wy, int wz, const glm::ivec3& chunkPos, uint32_t seed, const CaveConfig& cfg = CaveConfig{});

// Optional vegetation mask helper for future use.
bool caveVegetationMask(int wx, int wy, int wz, uint32_t seed, const CaveConfig& cfg = CaveConfig{});

// Carve caves into a raw block buffer.
void applyCavesToBlocks(BlockID* blocks, const glm::ivec3& chunkPos, uint32_t worldSeed, const CaveConfig& cfg = CaveConfig{}, bool* outVegetationMask = nullptr);

// Convenience wrapper for chunk instances.
inline void applyCavesToChunk(Chunk& c, uint32_t worldSeed, const CaveConfig& cfg = CaveConfig{}, bool* outVegetationMask = nullptr)
{
  applyCavesToBlocks(c.blocks, c.position, worldSeed, cfg, outVegetationMask);
}
