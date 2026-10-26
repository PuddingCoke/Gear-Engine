# =============================================================================
# SampleProject —— 示例项目模板（对应 SampleProject.vcxproj）
#
# 源文件（main.cpp / MyGame.h / MyRenderTask.h）由 gear_add_demo 自动收集；
# 其余设置（IncludePath、库、延迟加载、Assets/EngineAssets/Dlls 拷贝、
# Build/SampleProject/<Config> 输出）全部走 Demos/CMakeLists.txt 的公共默认。
# =============================================================================
gear_add_demo(SampleProject
    SHADERS
        "PixelShader.hlsl|Pixel"
)
