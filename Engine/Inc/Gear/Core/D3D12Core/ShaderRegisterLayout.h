// 由 ParseShaderRegisterLayout.ps1 自动生成，请勿编辑

#pragma once

#include <cstdint>

namespace Gear
{
    namespace Core
    {
        namespace D3D12Core
        {
            namespace ShaderRegisterLayout
            {
                constexpr uint32_t EngineGlobalCBRegister              = 0;
                constexpr uint32_t UserGlobalCBRegister                = 1;
                constexpr uint32_t DrawCallConstantsRegister           = 2;
                constexpr uint32_t DrawCallConstantBufferRegister      = 3;
            }
        }
    }
}
