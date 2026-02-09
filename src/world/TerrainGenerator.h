#pragma once
#include "Chunk.h"
#include <cstdint>

uint32_t getWorldSeed();

void generateTerrain(BlockID* blocks, int cx, int cy, int cz);

int getTerrainHeightAt(int worldX, int worldZ);

void getTerrainHeightsForChunk(int cx, int cz, int* outHeights);

