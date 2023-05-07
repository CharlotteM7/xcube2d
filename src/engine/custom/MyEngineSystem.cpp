#include "MyEngineSystem.h"
#include "../GraphicsEngine.h"
#include <algorithm>


MyEngineSystem::MyEngineSystem() {}


MyEngineSystem::MyEngineSystem(std::shared_ptr<GraphicsEngine> gfxPtr) : gfx(gfxPtr) {

    playerX = 5; 
    playerY = 5; 

    blueTexture = ResourceManager::loadTexture("res/textures/water.jpg", transparentColor);
    greenTexture = ResourceManager::loadTexture("res/textures/grassGreen.png", transparentColor);
    yellowTexture = ResourceManager::loadTexture("res/textures/grassYellow.png", transparentColor);
    brownTexture = ResourceManager::loadTexture("res/textures/grassBrown.png", transparentColor);
  
    waterSplashSound = Mix_LoadWAV("res/sounds/watersplash.wav");
    if (waterSplashSound == nullptr) {
        debug("Failed to load sound");
    }

}


MyEngineSystem::~MyEngineSystem() {
    if (waterSplashSound != nullptr) {
        Mix_FreeChunk(waterSplashSound);
    }
}

//Compute the 6th degree polynomial to return smoothed value t
float MyEngineSystem::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

//Linear interpolation
float MyEngineSystem::lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

//Gradient vector direction
float MyEngineSystem::grad(int hash, float x, float y) {
    int h = hash & 3;
    float u = h == 0 ? x : h == 1 ? -x : y;
    float v = h == 2 ? -x : h == 3 ? -y : x;
    return u + v;
}


float MyEngineSystem::perlin(float x, float y) {

    //Determine the integer grid cell coordinates xi and yi and the fractional parts xf and yf
    int xi = (int)x & 255;
    int yi = (int)y & 255;
    float xf = x - (int)x;
    float yf = y - (int)y;

    //Compute the fade values u and v for smooth interpolation.
    float u = fade(xf);
    float v = fade(yf);

    //Compute the hashed indices for each corner of the grid cell.
    int a = p[xi] + yi;
    int aa = p[a];
    int ab = p[a + 1];
    int b = p[xi + 1] + yi;
    int ba = p[b];
    int bb = p[b + 1];

    //Interpolate the gradient values at each corner using the lerp and grad functions.
    float x1, x2, y1;
    x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
    x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);
    y1 = lerp(x1, x2, v);

    //Return the final noise value
    return y1;
}

// Fill table and shuffle values
void MyEngineSystem::perlin_init() {
    for (int i = 0; i < 256; i++) {
        p[i] = i;
    }

    std::random_shuffle(p, p + 256);

    for (int i = 0; i < 256; i++) {
        p[i + 256] = p[i];
    }
}

//Generate grid and apply perlin 
void MyEngineSystem::generateTerrain() {
    // Generate random grid
    for (int i = 0; i < TERRAIN_HEIGHT; i++) {
        for (int j = 0; j < TERRAIN_WIDTH; j++) {
            terrain[i][j] = (float)rand() / RAND_MAX;
        }
    }

    // Apply Perlin noise to the grid
    perlin_init();
    int numOctaves = 6;
    float persistence = 0.3f;
    for (int octave = 0; octave < numOctaves; octave++) {
        float frequency = pow(2, octave);
        float amplitude = pow(persistence, octave);
        for (int i = 0; i < TERRAIN_HEIGHT; i++) {
            for (int j = 0; j < TERRAIN_WIDTH; j++) {
                terrain[i][j] += perlin(i * 0.1f * frequency, j * 0.1f * frequency) * amplitude;
            }
        }
    }

    // Normalise the modified values to a range of 0-1
    float min = terrain[0][0];
    float max = terrain[0][0];
    for (int i = 0; i < TERRAIN_HEIGHT; i++) {
        for (int j = 0; j < TERRAIN_WIDTH; j++) {
            if (terrain[i][j] < min) {
                min = terrain[i][j];
            }
            if (terrain[i][j] > max) {
                max = terrain[i][j];
            }
        }
    }
    for (int i = 0; i < TERRAIN_HEIGHT; i++) {
        for (int j = 0; j < TERRAIN_WIDTH; j++) {
            terrain[i][j] = (terrain[i][j] - min) / (max - min) * TERRAIN_HEIGHT_SCALE;
        }
    }
    debug("terrain generated");
}


void MyEngineSystem::renderTerrain() {
    if (gfx == nullptr) {
        debug("gfx is null");
        return;
    }

    /*  for (int i = 0; i < TERRAIN_HEIGHT; i++) {
          for (int j = 0; j < TERRAIN_WIDTH; j++) {
              printf("%0.2f ", terrain[i][j]);
          }
          printf("\n");
      }*/



    int offsetX = playerX % Chunk::CHUNK_SIZE;
    int offsetY = playerY % Chunk::CHUNK_SIZE;
    int playerChunkX = playerX / Chunk::CHUNK_SIZE;
    int playerChunkY = playerY / Chunk::CHUNK_SIZE;

    bool currentPlayerOnWater = false;

    for (int dx = -viewDistance; dx <= viewDistance; dx++) {
        for (int dy = -viewDistance; dy <= viewDistance; dy++) {
            int chunkX = playerChunkX + dx;
            int chunkY = playerChunkY + dy;
            Chunk& chunk = getChunk(chunkX, chunkY);

            for (int i = 0; i < Chunk::CHUNK_SIZE; i++) {
                for (int j = 0; j < Chunk::CHUNK_SIZE; j++) {
                    float height = chunk.terrain[i][j];
                    SDL_Texture* currentTexture;

                    if (height < LAKE_THRESHOLD) {
                        currentTexture = greenTexture;
                    }
                    else if (height < LAKE_THRESHOLD + 0.05f) {
                        currentTexture = yellowTexture;
                    }
                    else if (height < LAKE_THRESHOLD + 0.1f) {
                        currentTexture = brownTexture;
                    }
                    else {
                        currentTexture = blueTexture;
                    }

                    int worldX = chunkX * Chunk::CHUNK_SIZE + j;
                    int worldY = chunkY * Chunk::CHUNK_SIZE + i;

                    if (playerX == worldX && playerY == worldY && currentTexture == blueTexture) {
                        currentPlayerOnWater = true;
                        
                    }


                    int screenX = (j + dx * Chunk::CHUNK_SIZE - offsetX) * gridSize;
                    int screenY = (i + dy * Chunk::CHUNK_SIZE - offsetY) * gridSize;
                    SDL_Rect dst = { screenX, screenY, gridSize, gridSize };
                    gfx->drawTexture(currentTexture, &dst);
                }
            }
        }
    }

    if (currentPlayerOnWater && !playerOnWater) {

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastWaterSoundPlayed > waterSoundDelay) {
            if (waterSplashSound != nullptr) {
                Mix_PlayChannel(-1, waterSplashSound, 0);
            }
            lastWaterSoundPlayed = currentTime;
        }
    }

    playerOnWater = currentPlayerOnWater;
    

}



float MyEngineSystem::getTerrainHeightAt(int x, int y) const {
    int chunkX = x / Chunk::CHUNK_SIZE;
    int chunkY = y / Chunk::CHUNK_SIZE;
    int offsetX = x % Chunk::CHUNK_SIZE;
    int offsetY = y % Chunk::CHUNK_SIZE;
    auto key = std::make_pair(chunkX, chunkY);

    if (chunks.find(key) != chunks.end()) {
        return chunks.at(key).terrain[offsetY][offsetX];
    }
    else {
        return 0.0f;
    }
}


Chunk::Chunk() {}

//Generate terrain for a chunk
void Chunk::generateTerrain(int chunkX, int chunkY, MyEngineSystem& engineSystem) {
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            int worldX = chunkX * CHUNK_SIZE + i;
            int worldY = chunkY * CHUNK_SIZE + j;
            terrain[i][j] = engineSystem.perlin(worldX * 0.05f, worldY * 0.05f) * 1.0f;
        }
    }
}


Chunk& MyEngineSystem::getChunk(int chunkX, int chunkY) {
    auto key = std::make_pair(chunkX, chunkY);
    return chunks[key];
}

void MyEngineSystem::generateChunk(int chunkX, int chunkY) {
    
    auto key = std::make_pair(chunkX, chunkY);
    if (chunks.find(key) == chunks.end()) {
        Chunk newChunk;
        newChunk.generateTerrain(chunkX, chunkY, *this);
        chunks[key] = newChunk;
    }
}

//Update chunks based on player position 
void MyEngineSystem::updateVisibleChunks(int playerX, int playerY, int viewDistance) {
    int playerChunkX = playerX / Chunk::CHUNK_SIZE;
    int playerChunkY = playerY / Chunk::CHUNK_SIZE;

    //Generate chunks around the player
    for (int dx = -viewDistance; dx <= viewDistance; dx++) {
        for (int dy = -viewDistance; dy <= viewDistance; dy++) {
            int chunkX = playerChunkX + dx;
            int chunkY = playerChunkY + dy;
            generateChunk(chunkX, chunkY);
        }
    }

    // Remove chunks that are too far away
    int removalDistance = viewDistance + 3; 
    for (auto it = chunks.begin(); it != chunks.end();) {
        int distanceX = std::abs(it->first.first - playerChunkX);
        int distanceY = std::abs(it->first.second - playerChunkY);
        if (distanceX > removalDistance || distanceY > removalDistance) {
            it = chunks.erase(it);
          
        }
        else {
            ++it;
        }
    }
}

