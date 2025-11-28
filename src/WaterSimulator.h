#pragma once
#include "Chunk.h"
#include <queue>
#include <unordered_set>
#include <glm/glm.hpp>

struct ChunkManager;

constexpr uint8_t WATER_SOURCE = 9;
constexpr uint8_t WATER_FLOW_BASE = 10;
constexpr uint8_t WATER_LEVEL_MAX = 8;
constexpr uint8_t WATER_LEVEL_MIN = 1;

inline bool isWater(uint8_t blockId)
{
    return blockId == WATER_SOURCE || (blockId >= WATER_FLOW_BASE && blockId < WATER_FLOW_BASE + 8);
}

inline uint8_t getWaterLevel(uint8_t blockId)
{
    if (blockId == WATER_SOURCE) return WATER_LEVEL_MAX;
    if (blockId >= WATER_FLOW_BASE && blockId < WATER_FLOW_BASE + 8)
        return WATER_FLOW_BASE + 7 - blockId;
    return 0;
}

inline uint8_t waterBlockFromLevel(uint8_t level)
{
    if (level >= WATER_LEVEL_MAX) return WATER_SOURCE;
    if (level > 0) return WATER_FLOW_BASE + (7 - level);
    return 0;
}

struct WaterUpdate
{
    int x, y, z;
    int tickDelay;
};

struct WaterUpdateHash
{
    std::size_t operator()(const glm::ivec3& v) const noexcept
    {
        std::size_t h = std::hash<int>{}(v.x);
        h ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(v.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

class WaterSimulator
{
public:
    WaterSimulator();
    
    void setChunkManager(ChunkManager* cm) { chunkManager = cm; }
    
    void scheduleUpdate(int x, int y, int z, int tickDelay = 5);
    void tick();
    void onBlockChanged(int x, int y, int z, uint8_t oldBlock, uint8_t newBlock);
    
    int getTickRate() const { return tickRate; }
    void setTickRate(int rate) { tickRate = rate; }
    
private:
    ChunkManager* chunkManager = nullptr;
    std::queue<WaterUpdate> pendingUpdates;
    std::unordered_set<glm::ivec3, WaterUpdateHash> scheduledPositions;
    int tickRate = 5;
    int currentTick = 0;
    
    void processWaterAt(int x, int y, int z);
    bool canFlowInto(int x, int y, int z);
    void flowInto(int x, int y, int z, uint8_t level);
    int findPathToEdge(int x, int y, int z, int maxDist);
    void scheduleNeighborUpdates(int x, int y, int z);
};

