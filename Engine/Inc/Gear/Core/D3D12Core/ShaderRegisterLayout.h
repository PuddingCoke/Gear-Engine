//由 ParseShaderRegisterLayout.ps1 自动生成，请勿编辑

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
                constexpr struct ShaderGlobalParameterIndices
                {
                    uint32_t engineGlobalCBufferParameterIndex = 0;
                    uint32_t userGlobalCBufferParameterIndex = 1;
                }globalParameterIndices;

                struct ShaderLocalParameterIndices
                {
                    uint32_t perInvokeConstantsParameterIndex = 0;
                    uint32_t perInvokeCBufferParameterIndex = 0;
                };

                constexpr uint32_t engineGlobalCBufferRegister = 0;
                constexpr uint32_t userGlobalCBufferRegister = 1;
                constexpr uint32_t perInvokeConstantsRegister = 2;
                constexpr uint32_t perInvokeCBufferRegister = 3;

            }
        }
    }
}
