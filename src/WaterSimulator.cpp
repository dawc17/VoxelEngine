#include "WaterSimulator.h"
#include "ChunkManager.h"
#include "Raycast.h"
#include "BlockTypes.h"
#include <algorithm>
#include <array>

WaterSimulator::WaterSimulator()
{
}

void WaterSimulator::scheduleUpdate(int x, int y, int z, int tickDelay)
{
    glm::ivec3 pos(x, y, z);
    if (scheduledPositions.find(pos) != scheduledPositions.end())
        return;
    
    scheduledPositions.insert(pos);
    pendingUpdates.push({x, y, z, currentTick + tickDelay});
}

void WaterSimulator::tick()
{
    currentTick++;
    
    std::vector<WaterUpdate> toProcess;
    
    while (!pendingUpdates.empty())
    {
        WaterUpdate& update = pendingUpdates.front();
        if (update.tickDelay <= currentTick)
        {
            toProcess.push_back(update);
            glm::ivec3 pos(update.x, update.y, update.z);
            scheduledPositions.erase(pos);
            pendingUpdates.pop();
        }
        else
        {
            break;
        }
    }
    
    for (const auto& update : toProcess)
    {
        processWaterAt(update.x, update.y, update.z);
    }
}

void WaterSimulator::onBlockChanged(int x, int y, int z, uint8_t oldBlock, uint8_t newBlock)
{
    if (isWater(newBlock))
    {
        scheduleUpdate(x, y, z, tickRate);
    }
    
    if (isWater(oldBlock) && !isWater(newBlock))
    {
        scheduleUpdate(x, y - 1, z, tickRate);
        scheduleUpdate(x + 1, y, z, tickRate);
        scheduleUpdate(x - 1, y, z, tickRate);
        scheduleUpdate(x, y, z + 1, tickRate);
        scheduleUpdate(x, y, z - 1, tickRate);
        scheduleUpdate(x, y + 1, z, tickRate);
    }
    
    if (!isWater(oldBlock) && !isWater(newBlock))
    {
        uint8_t above = getBlockAtWorld(x, y + 1, z, *chunkManager);
        uint8_t below = getBlockAtWorld(x, y - 1, z, *chunkManager);
        uint8_t nx = getBlockAtWorld(x + 1, y, z, *chunkManager);
        uint8_t px = getBlockAtWorld(x - 1, y, z, *chunkManager);
        uint8_t nz = getBlockAtWorld(x, y, z + 1, *chunkManager);
        uint8_t pz = getBlockAtWorld(x, y, z - 1, *chunkManager);
        
        if (isWater(above)) scheduleUpdate(x, y + 1, z, tickRate);
        if (isWater(below)) scheduleUpdate(x, y - 1, z, tickRate);
        if (isWater(nx)) scheduleUpdate(x + 1, y, z, tickRate);
        if (isWater(px)) scheduleUpdate(x - 1, y, z, tickRate);
        if (isWater(nz)) scheduleUpdate(x, y, z + 1, tickRate);
        if (isWater(pz)) scheduleUpdate(x, y, z - 1, tickRate);
        
        if (newBlock == 0)
        {
            if (isWater(above)) scheduleUpdate(x, y, z, tickRate);
            if (isWater(nx)) scheduleUpdate(x, y, z, tickRate);
            if (isWater(px)) scheduleUpdate(x, y, z, tickRate);
            if (isWater(nz)) scheduleUpdate(x, y, z, tickRate);
            if (isWater(pz)) scheduleUpdate(x, y, z, tickRate);
        }
    }
}

bool WaterSimulator::canFlowInto(int x, int y, int z)
{
    uint8_t block = getBlockAtWorld(x, y, z, *chunkManager);
    if (block == 0) return true;
    if (isWater(block)) return true;
    return false;
}

void WaterSimulator::flowInto(int x, int y, int z, uint8_t level)
{
    if (!chunkManager) return;
    
    uint8_t currentBlock = getBlockAtWorld(x, y, z, *chunkManager);
    uint8_t currentLevel = getWaterLevel(currentBlock);
    
    if (level > currentLevel)
    {
        uint8_t newBlock = waterBlockFromLevel(level);
        setBlockAtWorld(x, y, z, newBlock, *chunkManager);
        scheduleUpdate(x, y, z, tickRate);
    }
}

void WaterSimulator::processWaterAt(int x, int y, int z)
{
    if (!chunkManager) return;
    
    uint8_t block = getBlockAtWorld(x, y, z, *chunkManager);
    
    if (!isWater(block))
    {
        uint8_t above = getBlockAtWorld(x, y + 1, z, *chunkManager);
        if (isWater(above))
        {
            uint8_t newLevel = (above == WATER_SOURCE) ? WATER_LEVEL_MAX : getWaterLevel(above);
            setBlockAtWorld(x, y, z, waterBlockFromLevel(newLevel), *chunkManager);
            scheduleUpdate(x, y, z, tickRate);
        }
        else
        {
            const int dx[] = {1, -1, 0, 0};
            const int dz[] = {0, 0, 1, -1};
            uint8_t maxNeighborLevel = 0;
            
            for (int i = 0; i < 4; i++)
            {
                uint8_t neighbor = getBlockAtWorld(x + dx[i], y, z + dz[i], *chunkManager);
                if (isWater(neighbor))
                {
                    uint8_t neighborLevel = getWaterLevel(neighbor);
                    if (neighborLevel > maxNeighborLevel)
                        maxNeighborLevel = neighborLevel;
                }
            }
            
            if (maxNeighborLevel > 1)
            {
                setBlockAtWorld(x, y, z, waterBlockFromLevel(maxNeighborLevel - 1), *chunkManager);
                scheduleUpdate(x, y, z, tickRate);
            }
        }
        return;
    }
    
    uint8_t currentLevel = getWaterLevel(block);
    bool isSource = (block == WATER_SOURCE);
    
    uint8_t below = getBlockAtWorld(x, y - 1, z, *chunkManager);
    if (canFlowInto(x, y - 1, z) && !g_blockTypes[below].solid)
    {
        uint8_t belowLevel = getWaterLevel(below);
        if (belowLevel < WATER_LEVEL_MAX - 1)
        {
            setBlockAtWorld(x, y - 1, z, waterBlockFromLevel(WATER_LEVEL_MAX - 1), *chunkManager);
            scheduleUpdate(x, y - 1, z, tickRate);
        }
        
        if (!isSource)
        {
            return;
        }
    }
    
    if (!isSource)
    {
        uint8_t above = getBlockAtWorld(x, y + 1, z, *chunkManager);
        bool hasSourceAbove = isWater(above);
        
        const int dx[] = {1, -1, 0, 0};
        const int dz[] = {0, 0, 1, -1};
        uint8_t maxNeighborLevel = 0;
        int adjacentSourceCount = 0;
        
        for (int i = 0; i < 4; i++)
        {
            uint8_t neighbor = getBlockAtWorld(x + dx[i], y, z + dz[i], *chunkManager);
            if (isWater(neighbor))
            {
                uint8_t neighborLevel = getWaterLevel(neighbor);
                if (neighborLevel > maxNeighborLevel)
                    maxNeighborLevel = neighborLevel;
                if (neighbor == WATER_SOURCE)
                    adjacentSourceCount++;
            }
        }
        
        if (adjacentSourceCount >= 2)
        {
            setBlockAtWorld(x, y, z, WATER_SOURCE, *chunkManager);
            scheduleNeighborUpdates(x, y, z);
            return;
        }
        
        uint8_t expectedLevel = 0;
        if (hasSourceAbove)
        {
            expectedLevel = WATER_LEVEL_MAX;
        }
        else if (maxNeighborLevel > 1)
        {
            expectedLevel = maxNeighborLevel - 1;
        }
        
        if (expectedLevel != currentLevel)
        {
            if (expectedLevel == 0)
            {
                setBlockAtWorld(x, y, z, 0, *chunkManager);
            }
            else
            {
                setBlockAtWorld(x, y, z, waterBlockFromLevel(expectedLevel), *chunkManager);
            }
            scheduleNeighborUpdates(x, y, z);
            return;
        }
    }
    
    if (currentLevel > 1 || isSource)
    {
        uint8_t flowLevel = isSource ? (WATER_LEVEL_MAX - 1) : (currentLevel - 1);
        
        const int dx[] = {1, -1, 0, 0};
        const int dz[] = {0, 0, 1, -1};
        
        std::array<int, 4> distances;
        std::fill(distances.begin(), distances.end(), 100);
        
        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int nz = z + dz[i];
            
            uint8_t neighborBlock = getBlockAtWorld(nx, y, nz, *chunkManager);
            if (g_blockTypes[neighborBlock].solid && !isWater(neighborBlock))
                continue;
            
            uint8_t belowNeighbor = getBlockAtWorld(nx, y - 1, nz, *chunkManager);
            if (!g_blockTypes[belowNeighbor].solid || isWater(belowNeighbor))
            {
                distances[i] = 1;
                continue;
            }
            
            distances[i] = findPathToEdge(nx, y, nz, 4);
        }
        
        int minDist = *std::min_element(distances.begin(), distances.end());
        
        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int nz = z + dz[i];
            
            uint8_t neighborBlock = getBlockAtWorld(nx, y, nz, *chunkManager);
            
            if (g_blockTypes[neighborBlock].solid && !isWater(neighborBlock))
                continue;
            
            bool shouldFlow = (minDist >= 100) || (distances[i] <= minDist);
            
            if (shouldFlow)
            {
                uint8_t neighborLevel = getWaterLevel(neighborBlock);
                if (neighborLevel < flowLevel)
                {
                    flowInto(nx, y, nz, flowLevel);
                }
            }
        }
    }
}

int WaterSimulator::findPathToEdge(int x, int y, int z, int maxDist)
{
    if (maxDist <= 0) return 100;
    
    uint8_t below = getBlockAtWorld(x, y - 1, z, *chunkManager);
    if (!g_blockTypes[below].solid || isWater(below))
        return 0;
    
    const int dx[] = {1, -1, 0, 0};
    const int dz[] = {0, 0, 1, -1};
    int minDist = 100;
    
    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int nz = z + dz[i];
        
        uint8_t neighborBlock = getBlockAtWorld(nx, y, nz, *chunkManager);
        if (g_blockTypes[neighborBlock].solid && !isWater(neighborBlock))
            continue;
        
        int dist = findPathToEdge(nx, y, nz, maxDist - 1);
        if (dist + 1 < minDist)
            minDist = dist + 1;
    }
    
    return minDist;
}

void WaterSimulator::scheduleNeighborUpdates(int x, int y, int z)
{
    scheduleUpdate(x + 1, y, z, tickRate);
    scheduleUpdate(x - 1, y, z, tickRate);
    scheduleUpdate(x, y + 1, z, tickRate);
    scheduleUpdate(x, y - 1, z, tickRate);
    scheduleUpdate(x, y, z + 1, tickRate);
    scheduleUpdate(x, y, z - 1, tickRate);
}

