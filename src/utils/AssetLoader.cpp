#include "AssetLoader.h"

#include <array>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace
{
    std::filesystem::path executableDir()
    {
#ifdef _WIN32
        std::array<char, MAX_PATH> path{};
        DWORD len = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (len == 0 || len >= path.size())
            return {};
        return std::filesystem::path(path.data()).parent_path();
#else
        std::array<char, 4096> path{};
        ssize_t len = readlink("/proc/self/exe", path.data(), path.size() - 1);
        if (len <= 0)
            return {};
        path[static_cast<size_t>(len)] = '\0';
        return std::filesystem::path(path.data()).parent_path();
#endif
    }

    std::filesystem::path searchFrom(const std::filesystem::path& base,
                                     const std::string& relativePath)
    {
        if (base.empty())
            return {};

        std::filesystem::path dir = base;
        for (int depth = 0; depth < 6; ++depth)
        {
            std::filesystem::path candidate = dir / relativePath;
            if (std::filesystem::exists(candidate))
                return candidate;

            std::filesystem::path parent = dir.parent_path();
            if (parent == dir)
                break;
            dir = parent;
        }

        return {};
    }
}

namespace AssetLoader
{
    std::filesystem::path findAsset(const std::string& relativePath)
    {
        std::filesystem::path fromCwd = searchFrom(std::filesystem::current_path(), relativePath);
        if (!fromCwd.empty())
            return fromCwd;

        return searchFrom(executableDir(), relativePath);
    }

    std::filesystem::path findTexture(const std::string& relativePath)
    {
        return findAsset("assets/textures/" + relativePath);
    }
}
