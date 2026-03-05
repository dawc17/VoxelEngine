#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct ToolVertex {
    glm::vec3 pos;
    glm::vec3 color;
    float faceShade;
};

struct ToolModel {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    uint32_t indexCount = 0;
};

constexpr uint8_t TOOL_ID_START = 128;

// pickaxes
constexpr uint8_t TOOL_DIAMOND_PICKAXE = 128;
constexpr uint8_t TOOL_WOOD_PICKAXE = 129;
constexpr uint8_t TOOL_STONE_PICKAXE = 130;
constexpr uint8_t TOOL_GOLD_PICKAXE = 131;
constexpr uint8_t TOOL_IRON_PICKAXE = 132;

// axes
constexpr uint8_t TOOL_DIAMOND_AXE = 133;
constexpr uint8_t TOOL_WOOD_AXE = 134;
constexpr uint8_t TOOL_STONE_AXE = 135;
constexpr uint8_t TOOL_GOLD_AXE = 136;
constexpr uint8_t TOOL_IRON_AXE = 137;

// shovels
constexpr uint8_t TOOL_DIAMOND_SHOVEL = 138;
constexpr uint8_t TOOL_WOOD_SHOVEL = 139;
constexpr uint8_t TOOL_STONE_SHOVEL = 140;
constexpr uint8_t TOOL_GOLD_SHOVEL = 141;
constexpr uint8_t TOOL_IRON_SHOVEL = 142;

// swords
constexpr uint8_t TOOL_DIAMOND_SWORD = 143;
constexpr uint8_t TOOL_WOOD_SWORD = 144;
constexpr uint8_t TOOL_STONE_SWORD = 145;
constexpr uint8_t TOOL_GOLD_SWORD = 146;
constexpr uint8_t TOOL_IRON_SWORD = 147;

inline bool isToolItem(uint8_t id) { return id >= TOOL_ID_START; }

class ToolModelGenerator {
    public:
    static ToolModel generateFromSprite(const unsigned char* pngData, unsigned int pngSize);
    static void destroyModel(ToolModel& model);
};

extern std::unordered_map<uint8_t, ToolModel> g_toolModels;
extern std::unordered_map<uint8_t, GLuint> g_toolIcons;

void loadToolModels();
void unloadToolModels();
