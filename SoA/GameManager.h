#pragma once
#include <deque>

#include <Vorb/TextureCache.h>
#include <Vorb/Vorb.h>

#include "WorldStructs.h"
#include "GLProgramManager.h"

class ChunkSlot;
class Player;
class VoxelEditor;
class Chunk;
class ChunkManager;

//This is where the main game components are contained
// TODO(Ben): Dependency injection.
class GameManager
{
public:

    static void initializeSystems();
    static void registerTexturesForLoad();
    static void getTextureHandles();

    static void initializeSound();
    static void saveState();
    static void savePlayerState();
    static void drawMarkers();

    static void addMarker(glm::dvec3 pos, nString name, glm::vec3 color);
    static void clickDragRay(ChunkManager* chunkManager, Player* player, bool isBreakRay);
    static void scanWSO(ChunkManager* chunkManager, Player* player);
    static void onQuit();
    static void endSession();

    static class VoxelEditor* voxelEditor;
    static class SoundEngine* soundEngine;
    static class MessageManager* messageManager;
    static class TerrainGenerator* terrainGenerator;

    static bool gameInitialized;
    static float fogStart, fogEnd;
    static Uint32 maxLodTicks;
    static class WSOAtlas* wsoAtlas;
    static class WSOScanner* wsoScanner;
    static class DebugRenderer* debugRenderer;
    static vg::GLProgramManager* glProgramManager;
    static class TexturePackLoader* texturePackLoader; ///< Handles the loading of texture packs
    static class vg::TextureCache* textureCache;

private:
    static bool _systemsInitialized;

};

