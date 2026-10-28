# =============================================================================
# Demos —— 示例项目的公共默认设置
#
# 这里的普通变量会被所有子目录继承，等价于原来的 Demos/DemoCommon.props。
# 单个 Demo 只需要提供源文件、着色器列表和差异化的库/定义。
# 所有示例项目统一在文件末尾启停。
# =============================================================================

# 额外第三方库（不带 .lib 后缀；Libs 下存在 <名>-d.lib 时 Debug 自动使用它）
set(GEAR_DEMO_LIBS "")

# 额外的头文件搜索路径（Engine/External、Engine/Inc 已由 gear_add_demo 自动添加）
set(GEAR_DEMO_INCLUDE_DIRS "")

# 额外的预处理器定义（_CONSOLE、_DEBUG/NDEBUG 已自动添加）
set(GEAR_DEMO_DEFINITIONS "")

# 着色器专用：额外的包含目录与预处理器定义
set(GEAR_DEMO_SHADER_INCLUDE_DIRS "")
set(GEAR_DEMO_SHADER_DEFINITIONS "")

# --- 示例项目 ---
add_subdirectory(BlackHole)
add_subdirectory(CrytekSponza)
add_subdirectory(DigitalRain)
add_subdirectory(FFTOcean)
add_subdirectory(FluidSimulation)
add_subdirectory(LoggerTest)
add_subdirectory(MandelBrotSet)
add_subdirectory(MandelBulb)
add_subdirectory(PBRRendering)
add_subdirectory(SampleProject)
add_subdirectory(SimpleParticleEffect)
add_subdirectory(SmoothLife)
add_subdirectory(SteeringBehavior)
add_subdirectory(TODOAPP)
