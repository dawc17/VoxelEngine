#include "CaveGenerator.h"
#include <array>
#include <cmath>

// Lightweight Perlin 3D implementation with reseeding support.
class Perlin3D
{
public:
  Perlin3D() = default;
  explicit Perlin3D(uint32_t seed) { reseed(seed); }

  void reseed(uint32_t seed)
  {
    uint32_t x = seed;
    for (int i = 0; i < 256; ++i)
    {
      x ^= x << 13; x ^= x >> 17; x ^= x << 5;
      perm[i] = static_cast<uint8_t>(x & 255u);
    }
    // Fisher-Yates shuffle for better distribution
    for (int i = 255; i > 0; --i)
    {
      x ^= x << 13; x ^= x >> 17; x ^= x << 5;
      uint32_t j = x % static_cast<uint32_t>(i + 1);
      std::swap(perm[i], perm[j]);
    }
    for (int i = 0; i < 256; ++i)
      perm[256 + i] = perm[i];
  }

  float noise(float x, float y, float z) const
  {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A  = perm[X] + Y;
    int AA = perm[A] + Z;
    int AB = perm[A + 1] + Z;
    int B  = perm[X + 1] + Y;
    int BA = perm[B] + Z;
    int BB = perm[B + 1] + Z;

    float g000 = grad(perm[AA],     x,     y,     z    );
    float g100 = grad(perm[BA],     x-1.0f,y,     z    );
    float g010 = grad(perm[AB],     x,     y-1.0f,z    );
    float g110 = grad(perm[BB],     x-1.0f,y-1.0f,z    );
    float g001 = grad(perm[AA+1],   x,     y,     z-1.0f);
    float g101 = grad(perm[BA+1],   x-1.0f,y,     z-1.0f);
    float g011 = grad(perm[AB+1],   x,     y-1.0f,z-1.0f);
    float g111 = grad(perm[BB+1],   x-1.0f,y-1.0f,z-1.0f);

    float lerpX1 = lerp(u, g000, g100);
    float lerpX2 = lerp(u, g010, g110);
    float lerpX3 = lerp(u, g001, g101);
    float lerpX4 = lerp(u, g011, g111);

    float lerpY1 = lerp(v, lerpX1, lerpX2);
    float lerpY2 = lerp(v, lerpX3, lerpX4);

    return lerp(w, lerpY1, lerpY2); // ~[-1,1]
  }

  float fbm(float x, float y, float z, int octaves, float gain, float freq) const
  {
    float amp = 1.0f;
    float sum = 0.0f;
    for (int i = 0; i < octaves; ++i)
    {
      sum += amp * noise(x * freq, y * freq, z * freq);
      freq *= 2.0f;
      amp *= gain;
    }
    return sum;
  }

private:
  std::array<uint8_t, 512> perm{};

  static inline float fade(float t) { return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
  static inline float lerp(float t, float a, float b) { return a + t * (b - a); }
  static inline float grad(uint8_t h, float x, float y, float z)
  {
    int g = h & 15;
    float u = g < 8 ? x : y;
    float v = g < 4 ? y : (g == 12 || g == 14 ? x : z);
    return ((g & 1) ? -u : u) + ((g & 2) ? -v : v);
  }
};

struct CaveNoiseSet
{
  Perlin3D shape;
  Perlin3D detail;
  Perlin3D veg;
  uint32_t seedShape = 0;
  uint32_t seedDetail = 0;
  uint32_t seedVeg = 0;
};

static thread_local CaveNoiseSet g_noiseCache;

static const CaveNoiseSet& getNoise(uint32_t seed)
{
  uint32_t sShape = seed * 0x9E3779B1u + 1u;
  uint32_t sDetail = seed * 0x85EBCA77u + 3u;
  uint32_t sVeg = seed * 0xC2B2AE3Du + 7u;

  if (g_noiseCache.seedShape != sShape)
  {
    g_noiseCache.shape.reseed(sShape);
    g_noiseCache.seedShape = sShape;
  }
  if (g_noiseCache.seedDetail != sDetail)
  {
    g_noiseCache.detail.reseed(sDetail);
    g_noiseCache.seedDetail = sDetail;
  }
  if (g_noiseCache.seedVeg != sVeg)
  {
    g_noiseCache.veg.reseed(sVeg);
    g_noiseCache.seedVeg = sVeg;
  }

  return g_noiseCache;
}

float caveDensity(int wx, int wy, int wz, const glm::ivec3& chunkPos, uint32_t seed, const CaveConfig& cfg)
{
  (void)chunkPos; // included for signature clarity
  const CaveNoiseSet& noise = getNoise(seed);

  float sx = static_cast<float>(wx);
  float sy = static_cast<float>(wy);
  float sz = static_cast<float>(wz);

  float shape = noise.shape.fbm(sx, sy, sz, cfg.shapeOctaves, cfg.shapeGain, cfg.shapeFrequency);
  float detail = noise.detail.fbm(sx, sy, sz, cfg.detailOctaves, cfg.detailGain, cfg.detailFrequency);

  float density = shape - cfg.baseThreshold + detail * cfg.detailAmplitude;

  // Fade out caves near the top of the world to avoid floating entrances.
  if (sy > cfg.verticalFadeStart)
  {
    if (sy >= cfg.verticalFadeEnd)
    {
      density = 0.0f;
    }
    else
    {
      float t = (sy - cfg.verticalFadeStart) / (cfg.verticalFadeEnd - cfg.verticalFadeStart);
      density *= (1.0f - t);
    }
  }

  // Floor/ceiling jitter
  float jitter = noise.detail.noise(sx * cfg.detailFrequency * 0.5f, sy * 0.05f, sz * cfg.detailFrequency * 0.5f) * cfg.floorJitterAmp;
  density += jitter;

  return density;
}

bool caveVegetationMask(int wx, int wy, int wz, uint32_t seed, const CaveConfig& cfg)
{
  const CaveNoiseSet& noise = getNoise(seed);
  float n = noise.veg.noise(wx * cfg.vegFrequency, wy * cfg.vegFrequency, wz * cfg.vegFrequency);
  return n > cfg.vegThreshold;
}

void applyCavesToBlocks(BlockID* blocks, const glm::ivec3& chunkPos, uint32_t worldSeed, const CaveConfig& cfg, bool* outVegetationMask)
{
  const int baseX = chunkPos.x * CHUNK_SIZE;
  const int baseY = chunkPos.y * CHUNK_SIZE;
  const int baseZ = chunkPos.z * CHUNK_SIZE;

  for (int z = 0; z < CHUNK_SIZE; ++z)
  {
    for (int y = 0; y < CHUNK_SIZE; ++y)
    {
      for (int x = 0; x < CHUNK_SIZE; ++x)
      {
        int wx = baseX + x;
        int wy = baseY + y;
        int wz = baseZ + z;

        float d = caveDensity(wx, wy, wz, chunkPos, worldSeed, cfg);
        int i = blockIndex(x, y, z);

        if (d > 0.0f)
        {
          blocks[i] = 0; // carve to air
          if (outVegetationMask)
            outVegetationMask[i] = false;
        }
        else if (outVegetationMask)
        {
          bool isFloor = false;
          if (y + 1 < CHUNK_SIZE)
          {
            float above = caveDensity(wx, wy + 1, wz, chunkPos, worldSeed, cfg);
            if (above > 0.0f)
              isFloor = true;
          }
          outVegetationMask[i] = isFloor && caveVegetationMask(wx, wy, wz, worldSeed, cfg);
        }
      }
    }
  }
}
