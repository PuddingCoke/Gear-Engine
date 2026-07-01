#pragma once

#ifndef _GEAR_DEVESSENTIAL_H_
#define _GEAR_DEVESSENTIAL_H_

#include<Gear/Core/PipelineStateHelper.h>

#include<Gear/Core/PipelineStateBuilder.h>

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/D3D12Core/Shader.h>

#include<Gear/Effect/BloomEffect.h>

#include<Gear/Effect/FXAAEffect.h>

#include<Gear/Effect/SSREffect.h>

#include<Gear/Effect/HBAOPlusEffect.h>

#include<Gear/Effect/ToneMapEffect.h>

#include<Gear/Effect/GammaCorrectEffect.h>

#include<Gear/Camera/FPSCamera.h>

#include<Gear/Camera/OrbitCamera.h>

#include<Gear/Input/Mouse.h>

#include<Gear/Input/Keyboard.h>

#include<Gear/Utils/MainMonitor.h>

#include<Gear/Utils/Math.h>

#include<Gear/Utils/Random.h>

#include<Gear/Utils/File.h>

#include<Gear/Utils/String.h>

#include<Gear/Utils/Timer.h>

#include<Gear/Utils/Color.h>

#include<Gear/Utils/StaticVector.h>

using namespace Gear;

using namespace Gear::Camera;

using namespace Gear::Core;

using namespace Gear::Effect;

using namespace Gear::Resource;

using namespace Gear::Input;

using namespace Gear::Utils;

using namespace Gear::Core::D3D12Core;

using Gear::Core::D3D12Core::DXCCompiler::ShaderProfile;

#endif // !_GEAR_DEVESSENTIAL_H_
