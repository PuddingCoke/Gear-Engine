#include<Gear/Core/GraphicsDevice.h>

#include<Gear/Core/Internal/GraphicsDeviceInternal.h>

namespace Gear::Core::GraphicsDevice
{
	namespace Internal
	{
		struct GraphicsDeviceImpl
		{
			ComPtr<ID3D12Device9> device;
		}impl;

		void initialize(IUnknown* const adapter)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&impl.device))))
			{
				LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(D3D_FEATURE_LEVEL_12_0), LogColor::defaultColor, L"特性等级的", LogColor::brightMagenta, TOWSTRING(ID3D12Device));
			}
			else
			{
				LOGERROR(L"找到的性能最强适配器不支持", TOWSTRING(D3D_FEATURE_LEVEL_12_0));
			}
		}

		void release()
		{
			impl.device = nullptr;
		}

		void checkFeatureSupport()
		{
			LOGENGINE(L"以下是特性支持信息");

			CD3DX12FeatureSupport features;

			features.Init(impl.device.Get());

			{
				const D3D12_RESOURCE_BINDING_TIER resourceBindingTier = features.ResourceBindingTier();

				std::wstring bindingTierString;

				switch (resourceBindingTier)
				{
				default:
				case D3D12_RESOURCE_BINDING_TIER_1:
					bindingTierString = TOWSTRING(D3D12_RESOURCE_BINDING_TIER_1);
					break;
				case D3D12_RESOURCE_BINDING_TIER_2:
					bindingTierString = TOWSTRING(D3D12_RESOURCE_BINDING_TIER_2);
					break;
				case D3D12_RESOURCE_BINDING_TIER_3:
					bindingTierString = TOWSTRING(D3D12_RESOURCE_BINDING_TIER_3);
					break;
				}

				LOGENGINE(L"资源绑定等级", LogColor::brightMagenta, bindingTierString);
			}

			{
				const D3D_SHADER_MODEL shaderModel = features.HighestShaderModel();

				std::wstring shaderModelString;

				switch (shaderModel)
				{
				default:
				case D3D_SHADER_MODEL_5_1:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_5_1);
					break;
				case D3D_SHADER_MODEL_6_0:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_0);
					break;
				case D3D_SHADER_MODEL_6_1:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_1);
					break;
				case D3D_SHADER_MODEL_6_2:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_2);
					break;
				case D3D_SHADER_MODEL_6_3:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_3);
					break;
				case D3D_SHADER_MODEL_6_4:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_4);
					break;
				case D3D_SHADER_MODEL_6_5:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_5);
					break;
				case D3D_SHADER_MODEL_6_6:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_6);
					break;
				case D3D_SHADER_MODEL_6_7:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_7);
					break;
				case D3D_SHADER_MODEL_6_8:
					shaderModelString = TOWSTRING(D3D_SHADER_MODEL_6_8);
					break;
				}

				LOGENGINE(L"最高支持的着色模型", LogColor::brightMagenta, shaderModelString);

				if (shaderModel < D3D_SHADER_MODEL_6_6)
				{
					LOGERROR(L"你的适配器不支持", TOWSTRING(D3D_SHADER_MODEL_6_6));
				}
			}

			{
				const bool typedUAVLoad = features.TypedUAVLoadAdditionalFormats();

				LOGENGINE(L"有类型UAV读取", typedUAVLoad);

				if (!typedUAVLoad)
				{
					LOGERROR(L"你的适配器不支持有类型UAV读取");
				}
			}

			{
				const D3D12_RAYTRACING_TIER rayTracingTier = features.RaytracingTier();

				std::wstring raytracingTierString;

				switch (rayTracingTier)
				{
				default:
				case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
					raytracingTierString = TOWSTRING(D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
					break;
				case D3D12_RAYTRACING_TIER_1_0:
					raytracingTierString = TOWSTRING(D3D12_RAYTRACING_TIER_1_0);
					break;
				case D3D12_RAYTRACING_TIER_1_1:
					raytracingTierString = TOWSTRING(D3D12_RAYTRACING_TIER_1_1);
					break;
				}

				LOGENGINE(L"光线追踪等级", LogColor::brightMagenta, raytracingTierString);
			}
		}
	}

	ID3D12Device9* get()
	{
		return Internal::impl.device.Get();
	}
}
