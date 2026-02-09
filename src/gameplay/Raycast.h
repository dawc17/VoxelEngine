#pragma once
#include <glm/glm.hpp>
#include <optional>

struct ChunkManager;

struct RaycastHit
{
  glm::ivec3 blockPos;
  glm::ivec3 normal;
  float distance;
};

std::optional<RaycastHit> raycastVoxel(
    const glm::vec3& origin,
    const glm::vec3& direction,
    float maxDistance,
    ChunkManager& chunkManager);

uint8_t getBlockAtWorld(int wx, int wy, int wz, ChunkManager& chunkManager);

void setBlockAtWorld(int wx, int wy, int wz, uint8_t blockId, ChunkManager& chunkManager);
