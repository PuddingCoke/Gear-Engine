
//由 CommonShaderParse.exe 自动生成，请勿修改

#pragma once

#include<DirectXMath.h>

#include<cstdint>

namespace Gear
{
	namespace Core
	{
		namespace D3D12Core
		{
			struct PerframeResource
			{
				float deltaTime;
				float timeElapsed;
				uint32_t uintSeed;
				float floatSeed;
				DirectX::XMMATRIX proj;
				DirectX::XMMATRIX view;
				DirectX::XMFLOAT4 eyePos;
				DirectX::XMMATRIX prevViewProj;
				DirectX::XMMATRIX viewProj;
				DirectX::XMMATRIX normalMatrix;
				DirectX::XMFLOAT2 screenSize;
				DirectX::XMFLOAT2 screenTexelSize;
				DirectX::XMUINT4 padding1[9];
			};
		}
	}
}