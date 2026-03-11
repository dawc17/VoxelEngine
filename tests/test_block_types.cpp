#include <gtest/gtest.h>
#include "utils/BlockTypes.h"
#include "rendering/ToolModelGenerator.h"

// All tests that call isBlock* or getBlock* must call initBlockTypes() first.
// We use a test fixture so the registry is initialised exactly once per suite.

class BlockTypesTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        initBlockTypes();
    }
};

// ---------------------------------------------------------------------------
// Air (ID 0) — special-cased in every inline helper
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, AirIsNotSolid)       { EXPECT_FALSE(isBlockSolid(0)); }
TEST_F(BlockTypesTest, AirIsTransparent)    { EXPECT_TRUE(isBlockTransparent(0)); }
TEST_F(BlockTypesTest, AirIsNotLiquid)      { EXPECT_FALSE(isBlockLiquid(0)); }

// ---------------------------------------------------------------------------
// Solid opaque blocks
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, DirtSolid)           { EXPECT_TRUE(isBlockSolid(1)); }
TEST_F(BlockTypesTest, DirtNotTransparent)  { EXPECT_FALSE(isBlockTransparent(1)); }
TEST_F(BlockTypesTest, DirtNotLiquid)       { EXPECT_FALSE(isBlockLiquid(1)); }

TEST_F(BlockTypesTest, GrassSolid)          { EXPECT_TRUE(isBlockSolid(2)); }
TEST_F(BlockTypesTest, GrassNotTransparent) { EXPECT_FALSE(isBlockTransparent(2)); }

TEST_F(BlockTypesTest, StoneSolid)          { EXPECT_TRUE(isBlockSolid(3)); }
TEST_F(BlockTypesTest, StoneNotTransparent) { EXPECT_FALSE(isBlockTransparent(3)); }

TEST_F(BlockTypesTest, CobblestoneSolid)    { EXPECT_TRUE(isBlockSolid(22)); }

// ---------------------------------------------------------------------------
// Transparent solid blocks (leaves, glass)
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, OakLeavesSolid)        { EXPECT_TRUE(isBlockSolid(6)); }
TEST_F(BlockTypesTest, OakLeavesTransparent)  { EXPECT_TRUE(isBlockTransparent(6)); }
TEST_F(BlockTypesTest, OakLeavesNotLiquid)    { EXPECT_FALSE(isBlockLiquid(6)); }

TEST_F(BlockTypesTest, GlassSolid)            { EXPECT_TRUE(isBlockSolid(7)); }
TEST_F(BlockTypesTest, GlassTransparent)      { EXPECT_TRUE(isBlockTransparent(7)); }

// ---------------------------------------------------------------------------
// Water (source = ID 9, flow levels = IDs 10–17)
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, WaterSourceNotSolid)   { EXPECT_FALSE(isBlockSolid(9)); }
TEST_F(BlockTypesTest, WaterSourceTransparent){ EXPECT_TRUE(isBlockTransparent(9)); }
TEST_F(BlockTypesTest, WaterSourceIsLiquid)   { EXPECT_TRUE(isBlockLiquid(9)); }

TEST_F(BlockTypesTest, WaterFlowLevelsAreLiquid)
{
    for (int id = 10; id <= 17; ++id)
    {
        EXPECT_FALSE(isBlockSolid(id))    << "ID " << id << " should not be solid";
        EXPECT_TRUE(isBlockTransparent(id)) << "ID " << id << " should be transparent";
        EXPECT_TRUE(isBlockLiquid(id))    << "ID " << id << " should be liquid";
    }
}

// ---------------------------------------------------------------------------
// Uninitialised block IDs (> 22 and not water) default to non-solid / transparent
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, UnusedIdsDefaultToAirLike)
{
    EXPECT_FALSE(isBlockSolid(100));
    EXPECT_TRUE(isBlockTransparent(100));
    EXPECT_FALSE(isBlockLiquid(100));
}

// ---------------------------------------------------------------------------
// getBlockHardness
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, HardnessDirt)        { EXPECT_FLOAT_EQ(getBlockHardness(1),  1.0f); }
TEST_F(BlockTypesTest, HardnessGrass)       { EXPECT_FLOAT_EQ(getBlockHardness(2),  1.2f); }
TEST_F(BlockTypesTest, HardnessStone)       { EXPECT_FLOAT_EQ(getBlockHardness(3),  1.5f); }
TEST_F(BlockTypesTest, HardnessSand)        { EXPECT_FLOAT_EQ(getBlockHardness(4),  0.8f); }
TEST_F(BlockTypesTest, HardnessLeaves)      { EXPECT_FLOAT_EQ(getBlockHardness(6),  0.2f); }
TEST_F(BlockTypesTest, HardnessCobblestone) { EXPECT_FLOAT_EQ(getBlockHardness(22), 2.0f); }
TEST_F(BlockTypesTest, HardnessDefaultFallback)
{
    // IDs not listed in the switch default to 1.0f
    EXPECT_FLOAT_EQ(getBlockHardness(0),   1.0f);
    EXPECT_FLOAT_EQ(getBlockHardness(100), 1.0f);
}

// ---------------------------------------------------------------------------
// getBlockPreferredTool
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, PreferredToolStone)       { EXPECT_EQ(getBlockPreferredTool(3),  ToolType::Pickaxe); }
TEST_F(BlockTypesTest, PreferredToolCobblestone) { EXPECT_EQ(getBlockPreferredTool(22), ToolType::Pickaxe); }
TEST_F(BlockTypesTest, PreferredToolDirt)        { EXPECT_EQ(getBlockPreferredTool(1),  ToolType::Shovel); }
TEST_F(BlockTypesTest, PreferredToolGrass)       { EXPECT_EQ(getBlockPreferredTool(2),  ToolType::Shovel); }
TEST_F(BlockTypesTest, PreferredToolOakPlanks)   { EXPECT_EQ(getBlockPreferredTool(8),  ToolType::None); }
TEST_F(BlockTypesTest, PreferredToolAir)         { EXPECT_EQ(getBlockPreferredTool(0),  ToolType::None); }
TEST_F(BlockTypesTest, PreferredToolWater)        { EXPECT_EQ(getBlockPreferredTool(9),  ToolType::None); }

// ---------------------------------------------------------------------------
// getToolSpeedMultiplier
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, PickaxeOnStoneSpeedMultipliers)
{
    // Stone (ID 3) prefers pickaxe
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_WOOD_PICKAXE,    3), 1.0f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_STONE_PICKAXE,   3), 1.5f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_IRON_PICKAXE,    3), 1.9f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_GOLD_PICKAXE,    3), 2.2f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_DIAMOND_PICKAXE, 3), 3.0f);
}

TEST_F(BlockTypesTest, WrongToolOnStonePenalty)
{
    // Using a non-pickaxe on stone should yield the wrong-tool penalty (0.3)
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_WOOD_AXE,    3), 0.3f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_WOOD_SHOVEL, 3), 0.3f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(0,                3), 0.3f);
}

TEST_F(BlockTypesTest, ShovelOnDirtSpeedMultipliers)
{
    // Dirt (ID 1) prefers shovel
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_DIAMOND_SHOVEL, 1), 3.0f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_WOOD_SHOVEL,    1), 1.0f);
}

TEST_F(BlockTypesTest, NoPreferredToolAlwaysOne)
{
    // Air (ID 0) has no preferred tool → multiplier is always 1.0
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_DIAMOND_PICKAXE, 0), 1.0f);
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(0, 0), 1.0f);

    // Oak planks (ID 8) — preferred tool is None
    EXPECT_FLOAT_EQ(getToolSpeedMultiplier(TOOL_DIAMOND_AXE, 8), 1.0f);
}

// ---------------------------------------------------------------------------
// initBlockTypes — verify g_defaultBlockTypes is a snapshot of initial state
// ---------------------------------------------------------------------------

TEST_F(BlockTypesTest, DefaultBlockTypesMatchInitial)
{
    // g_defaultBlockTypes is captured at the end of initBlockTypes()
    for (int id = 0; id < 256; ++id)
    {
        EXPECT_EQ(g_blockTypes[id].solid,         g_defaultBlockTypes[id].solid)        << "ID " << id;
        EXPECT_EQ(g_blockTypes[id].transparent,   g_defaultBlockTypes[id].transparent)  << "ID " << id;
        EXPECT_EQ(g_blockTypes[id].isLiquid,      g_defaultBlockTypes[id].isLiquid)     << "ID " << id;
    }
}
