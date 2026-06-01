$ErrorActionPreference = "Stop"

$hlsliPath = "$PSScriptRoot\..\Shaders\Common.hlsli"
$hlsli = Get-Content -Encoding UTF8 $hlsliPath -Raw

# ---------------------------------------------------------------
# 1. Extract all constant-buffer register slot macros
# ---------------------------------------------------------------
$macroPattern = '#define\s+(\w+)\s+b(\d+)'
$macroMatches = [regex]::Matches($hlsli, $macroPattern)

$macros = [ordered]@{}
foreach ($m in $macroMatches) {
    $macros[$m.Groups[1].Value] = [int]$m.Groups[2].Value
}

# ---------------------------------------------------------------
# 2. SNAKE_CASE -> camelCase (split on '_'; CBUFFER -> CBuffer)
# ---------------------------------------------------------------
function ConvertTo-CamelCase($macroName) {
    $parts = $macroName -split '_'
    $result = ""
    for ($i = 0; $i -lt $parts.Count; $i++) {
        $lower = $parts[$i].ToLower()
        if ($i -eq 0) {
            $result += $lower
        }
        elseif ($parts[$i] -eq "CBUFFER") {
            $result += "CBuffer"
        }
        else {
            $first = $lower.Substring(0, 1)
            $rest  = $lower.Substring(1)
            $result += $first.ToUpper([System.Globalization.CultureInfo]::InvariantCulture) + $rest
        }
    }
    return $result
}

# ---------------------------------------------------------------
# 3. Classify by GLOBAL presence
# ---------------------------------------------------------------
$globalMacros = [ordered]@{}
$localMacros  = [ordered]@{}

foreach ($name in $macros.Keys) {
    if ($name -match 'GLOBAL') {
        $globalMacros[$name] = $macros[$name]
    }
    else {
        $localMacros[$name] = $macros[$name]
    }
}

# ---------------------------------------------------------------
# 4. Generate output
# ---------------------------------------------------------------
$lines = [System.Collections.ArrayList]::new()

[void]$lines.Add('//由 ParseCommonHeaderLayout.ps1 自动生成，请勿编辑')
[void]$lines.Add('')
[void]$lines.Add('#pragma once')
[void]$lines.Add('')
[void]$lines.Add('#include <cstdint>')
[void]$lines.Add('')
[void]$lines.Add('namespace Gear')
[void]$lines.Add('{')
[void]$lines.Add('    namespace Core')
[void]$lines.Add('    {')
[void]$lines.Add('        namespace D3D12Core')
[void]$lines.Add('        {')
[void]$lines.Add('            namespace ShaderRegisterLayout')
[void]$lines.Add('            {')

# --- ShaderGlobalParameterIndices ---
[void]$lines.Add('                constexpr struct ShaderGlobalParameterIndices')
[void]$lines.Add('                {')
foreach ($name in $globalMacros.Keys) {
    $camel = ConvertTo-CamelCase $name
    $reg   = $globalMacros[$name]
    [void]$lines.Add("                    uint32_t ${camel}ParameterIndex = $reg;")
}
[void]$lines.Add('                }globalParameterIndices;')
[void]$lines.Add('')

# --- ShaderLocalParameterIndices ---
[void]$lines.Add('                struct ShaderLocalParameterIndices')
[void]$lines.Add('                {')
foreach ($name in $localMacros.Keys) {
    $camel = ConvertTo-CamelCase $name
    [void]$lines.Add("                    uint32_t ${camel}ParameterIndex = 0;")
}
[void]$lines.Add('                };')
[void]$lines.Add('')

# --- constexpr register-slot constants (all macros) ---
foreach ($name in $macros.Keys) {
    $camel = ConvertTo-CamelCase $name
    $reg   = $macros[$name]
    [void]$lines.Add("                constexpr uint32_t ${camel}Register = $reg;")
}
if ($macros.Count -gt 0) {
    [void]$lines.Add('')
}

[void]$lines.Add('            }')
[void]$lines.Add('        }')
[void]$lines.Add('    }')
[void]$lines.Add('}')

$outPath = "$PSScriptRoot\..\Inc\Gear\Core\D3D12Core\ShaderRegisterLayout.h"
$lines -join "`r`n" | Set-Content -Encoding UTF8 $outPath

Write-Host "生成 $outPath"
