# =============================================================================
# SteeringBehavior —— 示例项目（对应 SteeringBehavior.vcxproj）
# =============================================================================
gear_add_demo(SteeringBehavior
    SHADERS
        "StepCS.hlsl|Compute"
        "VehicleVS.hlsl|Vertex"
        "VehicleGS.hlsl|Geometry"
        "VehiclePS.hlsl|Pixel"
)
