#pragma once
#include "Chunk.h"
#include "Biome.h"
#include <cstdint>

uint32_t getWorldSeed();
void setWorldSeed(uint32_t seed);

// outTerrainHeights (optional): if non-null, filled with the computed terrain
// height for each XZ column in row-major order [z * CHUNK_SIZE + x].
// Pass the same array to applyCavesToBlocks to avoid recomputing heights.
void generateTerrain(BlockID* blocks, int cx, int cy, int cz, int* outTerrainHeights = nullptr);

BiomeID getBiomeAt(int worldX, int worldZ);

int getTerrainHeightAt(int worldX, int worldZ);

void getTerrainHeightsForChunk(int cx, int cz, int* outHeights);

