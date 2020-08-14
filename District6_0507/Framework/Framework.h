#pragma once
#include "Pch.h"

#define WIN_START_X 0
#define WIN_START_Y 0
#define WIN_WIDTH 1280
#define WIN_HEIGHT 720
#define CENTER_X WIN_WIDTH*0.5f
#define CENTER_Y WIN_HEIGHT*0.5f

#ifdef NDEBUG
	#define V(hr) hr
#else
	#define V(hr) assert(SUCCEEDED(hr))
#endif

#define DEVICE Device::Get()->GetDevice()
#define DC Device::Get()->GetContext()

#define DELTA Timer::Get()->Delta()

#define KEY_DOWN(k) Keyboard::Get()->Down(k)
#define KEY_PRESS(k) Keyboard::Get()->Press(k)
#define KEY_UP(k) Keyboard::Get()->Up(k)
#define SHADER SelfShader::Get()
#define STATE CommonStates::Get()
#define SOUND SoundManager::Get()


#define MOUSEPOS Keyboard::Get()->GetMouse()

#define LERP(s, e, t) (s + (e - s)*t)

#define SAFE_DELETE(p) {if(p){delete (p); (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p) {if(p){delete[] (p); (p)=NULL;}}
#define SAFE_RELEASE(p) {if(p) {(p)->Release(); (p)=NULL;}}


#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif

#ifdef _DEBUG

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#define DEBUG_LOG(fmt, ...)\
fprintf(stderr, "[%s:%d]\n%[s()]\n" fmt "\n\n",\
    strrchr("\\" __FILE__, '\\') + 1, __LINE__, __func__, __VA_ARGS__)

#define DEBUG_LOG_I(fmt, ...)\
fprintf(stderr, "[%s:%d]\n[%s:%p]\n[%s()]\n" fmt "\n\n",\
    strrchr("\\" __FILE__, '\\') + 1, __LINE__, typeid(*this).name(), this, __func__, __VA_ARGS__)

#else
#define DEBUG_LOG(fmt, ...)
#define DEBUG_LOG_I(fmt, ...)
#endif


typedef XMFLOAT4 Float4;
typedef XMFLOAT3 Float3;
typedef XMFLOAT2 Float2;
typedef XMVECTOR Vector4;
typedef XMMATRIX Matrix;
typedef XMFLOAT4X4 Float4x4;
typedef XMCOLOR Color;

typedef function<void()> Event;

const XMVECTORF32 kForward = { 0, 0, 1 };
const XMVECTORF32 kUp = { 0, 1, 0 };
const XMVECTORF32 kRight = { 1, 0, 0 };

#include "Device/Device.h"

#include "Math/Vector3.h"
#include "Math/Transform.h"
#include "Math/Math.h"

#include "Utility/Path.h"
#include "Utility/Binary.h"
#include "Utility/Keyboard.h"
#include "Utility/Timer.h"
#include "Utility/Xml.h"
#include "Utility/SoundManager.h"
#include "Utility/Loop.h"

#include "Render/Shader.h"
#include "Render/ComputShader.h"
#include "Render/Buffer.h"
#include "Render/ConstBuffer.h"
#include "Render/GlobalBuffer.h"
#include "Render/VertexLayouts.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/CsResource.h"
#include "Render/CsBuffer.h"
#include "Render/DepthStencil.h"
#include "Render/RenderTarget.h"
#include "Render/CommonStates.h"
#include "Render/SelfShader.h"

#include "State/RasterizerState.h"
#include "State/BlendState.h"
#include "State/DepthStencilState.h"
#include "State/SamplerState.h"

#include "Converter/Types.h"
#include "Converter/ModelReader.h"
#include "Converter/AssimpConverter.h"

#define MAX_MODEL_BONE 500
#define MAX_MODEL_KEY 600
#define MAX_MODEL_INSTANCE 500

#include "Collision/Collider.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"

#include "Model/ModelMeshPart.h"
#include "Model/ModelMesh.h"
#include "Model/ModelClip.h"
#include "Model/Model.h"
#include "Model/ModelRender.h"
#include "Model/ModelAnimator.h"
#include "Model/AnimationModel.h"

using namespace GameMath;
//using namespace Path;