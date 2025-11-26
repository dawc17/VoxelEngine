#include "ChunkManager.h"
#include "PerlinNoise.hpp"
#include <iostream>

// Global Perlin noise generator with a fixed seed for consistent terrain
static const siv::PerlinNoise::seed_type TERRAIN_SEED = 12345u;
static const siv::PerlinNoise perlin{TERRAIN_SEED};

// Terrain generation parameters
constexpr float NOISE_SCALE = 0.02f;      // Lower = larger features
constexpr int BASE_HEIGHT = 4;             // Minimum terrain height
constexpr int HEIGHT_VARIATION = 10;       // Max height added by noise (keeps max at 14, within chunk)
constexpr int DIRT_DEPTH = 3;              // Layers of dirt below grass

bool ChunkManager::hasChunk(int cx, int cy, int cz)
{
  ChunkCoord key{cx, cy, cz};
  return chunks.find(key) != chunks.end();
}

namespace
{
  void generateTerrain(Chunk &c)
  {
    // World position offset for this chunk
    int worldOffsetX = c.position.x * CHUNK_SIZE;
    int worldOffsetZ = c.position.z * CHUNK_SIZE;

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
      for (int z = 0; z < CHUNK_SIZE; z++)
      {
        // Calculate world coordinates
        float worldX = static_cast<float>(worldOffsetX + x);
        float worldZ = static_cast<float>(worldOffsetZ + z);

        // Sample Perlin noise (returns value in roughly -1 to 1 range)
        // Use octaves for more interesting terrain
        double noiseValue = perlin.octave2D_01(worldX * NOISE_SCALE, worldZ * NOISE_SCALE, 4);

        // Convert to height (0-1 range from octave2D_01)
        int terrainHeight = BASE_HEIGHT + static_cast<int>(noiseValue * HEIGHT_VARIATION);

        // Fill the column
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
          int i = blockIndex(x, y, z);

          if (y > terrainHeight)
          {
            c.blocks[i] = 0; // Air
          }
          else if (y == terrainHeight)
          {
            c.blocks[i] = 2; // Grass on top
          }
          else if (y > terrainHeight - DIRT_DEPTH)
          {
            c.blocks[i] = 1; // Dirt below grass
          }
          else
          {
            c.blocks[i] = 3; // Stone at the bottom
          }
        }
      }
    }
  }
}

Chunk *ChunkManager::getChunk(int cx, int cy, int cz)
{
  ChunkCoord key{cx, cy, cz};
  auto it = chunks.find(key);
  if (it == chunks.end())
    return nullptr;
  return it->second.get();
}

Chunk *ChunkManager::loadChunk(int cx, int cy, int cz)
{
  if (hasChunk(cx, cy, cz))
    return getChunk(cx, cy, cz);

  std::cout << "Loading chunk at (" << cx << ", " << cy << ", " << cz << ")" << std::endl;

  ChunkCoord key{cx, cy, cz};
  auto [it, inserted] = chunks.emplace(key, std::make_unique<Chunk>());
  (void)inserted;
  Chunk *c = it->second.get();
  c->position = {cx, cy, cz};

  glGenVertexArrays(1, &c->vao);
  glGenBuffers(1, &c->vbo);
  glGenBuffers(1, &c->ebo);

  generateTerrain(*c);

  return c;
}

void ChunkManager::unloadChunk(int cx, int cy, int cz)
{
  ChunkCoord key{cx, cy, cz};
  auto it = chunks.find(key);

  if (it != chunks.end())
  {
    chunks.erase(it);
  }
}
