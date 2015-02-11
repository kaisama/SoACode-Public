///
/// SoaState.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// The main game state for SoA
///

#pragma once

#ifndef SoAState_h__
#define SoAState_h__

#include "SpaceSystem.h"
#include "GameSystem.h"

#include "PlanetLoader.h" //Why is this needed here??

#include <Vorb/io/IOManager.h>
#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>

class DebugRenderer;
class MeshManager;
class PlanetLoader;
DECL_VG(class GLProgramManager);
DECL_VIO(class IOManager);

class SoaState {
public:
    ~SoaState();

    std::unique_ptr<SpaceSystem> spaceSystem;
    std::unique_ptr<GameSystem> gameSystem;

    vcore::EntityID startingPlanet = 0;
    vcore::EntityID playerEntity = 0;

    std::unique_ptr<vg::GLProgramManager> glProgramManager;
    std::unique_ptr<DebugRenderer> debugRenderer;
    std::unique_ptr<MeshManager> meshManager;

    std::unique_ptr<vio::IOManager> systemIoManager;
    std::unique_ptr<PlanetLoader> planetLoader;

    vio::IOManager saveFileIom;
    bool isNewGame = true;
    f64v3 startSpacePos = f64v3(0.0f);
    int startFace = 0;
    f64 time = 0.0;
};

#endif // SoAState_h__
