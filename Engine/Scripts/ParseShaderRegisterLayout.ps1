$ErrorActionPreference = "Stop"

$hlsliPath = "$PSScriptRoot\..\Shaders\Common.hlsli"
$hlsli = Get-Content -Encoding UTF8 $hlsliPath -Raw

function Get-RegisterNumber($macroName) {
    if ($hlsli -match "#define\s+$macroName\s+b(\d+)") {
        return [int]$Matches[1]
    }
    throw "Cannot find definition of '$macroName' in Common.hlsli"
}

$engGlobal  = Get-RegisterNumber "ENGINEGLOBALCBUFFER"
$usrGlobal  = Get-RegisterNumber "USERGLOBALCBUFFER"
$drawConsts = Get-RegisterNumber "DRAWCALLCONSTANTS"
$drawCB     = Get-RegisterNumber "DRAWCALLCBUFFER"

$header = @"
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
                constexpr uint32_t EngineGlobalCBRegister              = $engGlobal;
                constexpr uint32_t UserGlobalCBRegister                = $usrGlobal;
                constexpr uint32_t DrawCallConstantsRegister           = $drawConsts;
                constexpr uint32_t DrawCallConstantBufferRegister      = $drawCB;
            }
        }
    }
}
"@

$outPath = "$PSScriptRoot\..\Inc\Gear\Core\D3D12Core\ShaderRegisterLayout.h"
$header | Set-Content -Encoding UTF8 $outPath

Write-Host "从Common.hlsli生成ShaderRegisterLayout.h"
