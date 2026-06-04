#pragma once

#include <filesystem>
#include <string>

namespace AssetLoader
{
    std::filesystem::path findAsset(const std::string& relativePath);
    std::filesystem::path findTexture(const std::string& relativePath);
}
