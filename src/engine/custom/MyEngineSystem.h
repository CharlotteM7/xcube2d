#ifndef __MY_ENGINE_H__
#define __MY_ENGINE_H__

#include "../EngineCommon.h"
#include "../GraphicsEngine.h"
#include "../ResourceManager.h"
#include "../AbstractGame.h"
#include <unordered_map>


struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        return h1 ^ h2;
    }
};

class Chunk {
public:
    static const int CHUNK_SIZE = 38;
    float terrain[CHUNK_SIZE][CHUNK_SIZE];
   
    Chunk();
    void generateTerrain(int chunkX, int chunkY, MyEngineSystem& engineSystem);
};


class MyEngineSystem  {
    friend class XCube2Engine;
 
private:
    std::shared_ptr<GraphicsEngine> gfx;
    int p[512];
    SDL_Texture* blueTexture;
    SDL_Texture* greenTexture;
    SDL_Texture* yellowTexture;
    SDL_Texture* brownTexture;
    Mix_Chunk* waterSplashSound;
    static const int TERRAIN_GRID_SIZE = 10;
    static const int TERRAIN_WIDTH = 100;
    static const int TERRAIN_HEIGHT = 100;
    static constexpr float TERRAIN_HEIGHT_SCALE = 2.0f;
    float terrain[TERRAIN_HEIGHT][TERRAIN_WIDTH];
    bool playerOnWater = false;
    Uint32 waterSoundDelay = 2500; // milliseconds
    Uint32 lastWaterSoundPlayed = 0;

public:
    MyEngineSystem(std::shared_ptr<GraphicsEngine> gfxPtr);
    MyEngineSystem();
    ~MyEngineSystem();
    
    int playerX, playerY;
    static const int viewDistance = 2;
    static const int gridSize = TERRAIN_GRID_SIZE;
    static constexpr float LAKE_THRESHOLD = 0.2f;

    // Perlin noise functions
    float perlin(float x, float y);
    static float fade(float t);
    void perlin_init();
    float lerp(float a, float b, float t);
    float grad(int hash, float x, float y);
    void renderTerrain();
    void generateTerrain();
    float getTerrainHeightAt(int x, int y) const;


    std::unordered_map<std::pair<int, int>, Chunk, pair_hash> chunks;

    Chunk& getChunk(int chunkX, int chunkY);
    void generateChunk(int chunkX, int chunkY);
    void updateVisibleChunks(int playerX, int playerY, int viewDistance);

   
};



#endif
