#include <gtest/gtest.h>

// CoordUtils.h is header-only (inline functions). It includes Chunk.h for
// CHUNK_SIZE and glm::ivec3. Chunk.h pulls in glad/glad.h for GLuint typedefs
// only — no GL functions are called here.
#include "utils/CoordUtils.h"

// CHUNK_SIZE == 16 throughout these tests.

// ---------------------------------------------------------------------------
// worldToChunk
// ---------------------------------------------------------------------------

TEST(WorldToChunk, Origin)
{
    EXPECT_EQ(worldToChunk(0, 0, 0), glm::ivec3(0, 0, 0));
}

TEST(WorldToChunk, WithinFirstChunk)
{
    EXPECT_EQ(worldToChunk(15, 15, 15), glm::ivec3(0, 0, 0));
    EXPECT_EQ(worldToChunk(1, 7, 3),   glm::ivec3(0, 0, 0));
}

TEST(WorldToChunk, ExactChunkBoundary)
{
    EXPECT_EQ(worldToChunk(16, 16, 16), glm::ivec3(1, 1, 1));
    EXPECT_EQ(worldToChunk(32,  0,  0), glm::ivec3(2, 0, 0));
}

TEST(WorldToChunk, NegativeCoords)
{
    // -1 falls in chunk -1 (floor(-1/16) = -1)
    EXPECT_EQ(worldToChunk(-1, -1, -1),  glm::ivec3(-1, -1, -1));
    // -16 is the start of chunk -1
    EXPECT_EQ(worldToChunk(-16, -16, -16), glm::ivec3(-1, -1, -1));
    // -17 crosses into chunk -2
    EXPECT_EQ(worldToChunk(-17, 0, 0), glm::ivec3(-2, 0, 0));
}

TEST(WorldToChunk, MixedAxes)
{
    EXPECT_EQ(worldToChunk(16, -1, 0), glm::ivec3(1, -1, 0));
}

// ---------------------------------------------------------------------------
// worldToLocal
// ---------------------------------------------------------------------------

TEST(WorldToLocal, Origin)
{
    EXPECT_EQ(worldToLocal(0, 0, 0), glm::ivec3(0, 0, 0));
}

TEST(WorldToLocal, WithinFirstChunk)
{
    EXPECT_EQ(worldToLocal(5, 10, 3), glm::ivec3(5, 10, 3));
    EXPECT_EQ(worldToLocal(15, 0, 15), glm::ivec3(15, 0, 15));
}

TEST(WorldToLocal, ExactChunkBoundary)
{
    // 16 is the start of the next chunk, local coord wraps to 0
    EXPECT_EQ(worldToLocal(16, 0, 0), glm::ivec3(0, 0, 0));
    EXPECT_EQ(worldToLocal(32, 0, 0), glm::ivec3(0, 0, 0));
}

TEST(WorldToLocal, JustPastBoundary)
{
    EXPECT_EQ(worldToLocal(17, 0, 0), glm::ivec3(1, 0, 0));
}

TEST(WorldToLocal, NegativeCoords)
{
    // -1 → local 15  (((−1 % 16) + 16) % 16 = 15)
    EXPECT_EQ(worldToLocal(-1, 0, 0), glm::ivec3(15, 0, 0));
    // -16 → local 0
    EXPECT_EQ(worldToLocal(-16, 0, 0), glm::ivec3(0, 0, 0));
    // -17 → local 15
    EXPECT_EQ(worldToLocal(-17, 0, 0), glm::ivec3(15, 0, 0));
}

TEST(WorldToLocal, RoundTrip)
{
    // worldToLocal and worldToChunk together reconstruct the world coord
    for (int w = -48; w <= 48; ++w)
    {
        glm::ivec3 chunk = worldToChunk(w, 0, 0);
        glm::ivec3 local = worldToLocal(w, 0, 0);
        int reconstructed = chunk.x * CHUNK_SIZE + local.x;
        EXPECT_EQ(reconstructed, w) << "Failed for world coord " << w;
    }
}

// ---------------------------------------------------------------------------
// chunkToWorld
// ---------------------------------------------------------------------------

TEST(ChunkToWorld, Origin)
{
    EXPECT_EQ(chunkToWorld(0, 0, 0), glm::ivec3(0, 0, 0));
}

TEST(ChunkToWorld, PositiveChunks)
{
    EXPECT_EQ(chunkToWorld(1, 0, 0), glm::ivec3(16, 0,  0));
    EXPECT_EQ(chunkToWorld(2, 3, 1), glm::ivec3(32, 48, 16));
}

TEST(ChunkToWorld, NegativeChunks)
{
    EXPECT_EQ(chunkToWorld(-1, -1, -1), glm::ivec3(-16, -16, -16));
    EXPECT_EQ(chunkToWorld(-2,  0,  0), glm::ivec3(-32,   0,   0));
}

TEST(ChunkToWorld, RoundTrip)
{
    // chunkToWorld is the inverse of worldToChunk for chunk-aligned positions
    for (int c = -5; c <= 5; ++c)
    {
        glm::ivec3 world = chunkToWorld(c, 0, 0);
        glm::ivec3 back  = worldToChunk(world.x, world.y, world.z);
        EXPECT_EQ(back.x, c) << "Failed for chunk coord " << c;
    }
}
