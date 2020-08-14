#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

#define VP Environment::Get()->GetVP()
#define CAMERA Environment::Get()->MainCamera()
#define D_LIGHT Environment::Get()->GetLight()
#define LIGHT Environment::Get()->GetLightInfoBuffer()
#define ENVIRONMENT Environment::Get()
#define SCENE SceneManager::GetInstance()

#include <windows.h>

#include "../Framework/Framework.h"

#include "Camera/Camera.h"
#include "Camera/FreeCam.h"
#include "Camera/FollowCam.h"
#include "Camera/FixityCam.h"
#include "Camera/FPSCam.h"

#include "Environment/Environment.h"

#include "Objects/Landscape/Terrain.h"
#include "Objects/Landscape/TerrainEditor.h"
#include "Objects/Landscape/CubeSky.h"

#include "Objects/UI/Render2D.h"

#include "Objects/Model/Kaya.h"
#include "Objects/Model/AniInstance.h"
#include "Objects/Model/ModelInstance.h"
#include "Objects/Model/FPSModel.h"
#include "Objects/Model/Enemy.h"
#include "Objects/Model/EnemyManager.h"

#include "Objects/Sphere.h"
#include "Objects/Shadow.h"
#include "Objects/Reflection.h"


#include "AStar/Node.h"
#include "AStar/Heap.h"
#include "AStar/AStar.h"

#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

#include "Scene/MapToolScene.h"
#include "Scene/PlayScene.h"
#include "Scene/TitleScene.h"
#include "Scene/EndScene.h"

extern bool isGuiRender;