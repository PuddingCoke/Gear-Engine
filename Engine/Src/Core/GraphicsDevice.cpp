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
				LOGSUCCESS(L"create d3d12 device with feature level", LogColor::brightMagenta, L"D3D_FEATURE_LEVEL_12_0", LogColor::defaultColor, L"succeeded");
			}
			else
			{
				LOGERROR(L"your graphics card should support D3D_FEATURE_LEVEL_12_0");
			}
		}

		void release()
		{
			impl.device = nullptr;
		}

		void checkFeatureSupport()
		{
			LOGENGINE(L"following are feature support data");

			CD3DX12FeatureSupport features;

			features.Init(impl.device.Get());

			{
				const D3D12_RESOURCE_BINDING_TIER resourceBindingTier = features.ResourceBindingTier();

				std::wstring bindingTierString;

				switch (resourceBindingTier)
				{
				default:
				case D3D12_RESOURCE_BINDING_TIER_1:
					bindingTierString = L"D3D12_RESOURCE_BINDING_TIER_1";
					break;
				case D3D12_RESOURCE_BINDING_TIER_2:
					bindingTierString = L"D3D12_RESOURCE_BINDING_TIER_2";
					break;
				case D3D12_RESOURCE_BINDING_TIER_3:
					bindingTierString = L"D3D12_RESOURCE_BINDING_TIER_3";
					break;
				}

				LOGENGINE(L"resource binding tier", LogColor::brightMagenta, bindingTierString);
			}

			{
				const D3D_SHADER_MODEL shaderModel = features.HighestShaderModel();

				std::wstring shaderModelString;

				switch (shaderModel)
				{
				default:
				case D3D_SHADER_MODEL_5_1:
					shaderModelString = L"D3D_SHADER_MODEL_5_1";
					break;
				case D3D_SHADER_MODEL_6_0:
					shaderModelString = L"D3D_SHADER_MODEL_6_0";
					break;
				case D3D_SHADER_MODEL_6_1:
					shaderModelString = L"D3D_SHADER_MODEL_6_1";
					break;
				case D3D_SHADER_MODEL_6_2:
					shaderModelString = L"D3D_SHADER_MODEL_6_2";
					break;
				case D3D_SHADER_MODEL_6_3:
					shaderModelString = L"D3D_SHADER_MODEL_6_3";
					break;
				case D3D_SHADER_MODEL_6_4:
					shaderModelString = L"D3D_SHADER_MODEL_6_4";
					break;
				case D3D_SHADER_MODEL_6_5:
					shaderModelString = L"D3D_SHADER_MODEL_6_5";
					break;
				case D3D_SHADER_MODEL_6_6:
					shaderModelString = L"D3D_SHADER_MODEL_6_6";
					break;
				case D3D_SHADER_MODEL_6_7:
					shaderModelString = L"D3D_SHADER_MODEL_6_7";
					break;
				case D3D_SHADER_MODEL_6_8:
					shaderModelString = L"D3D_SHADER_MODEL_6_8";
					break;
				}

				LOGENGINE(L"highest supported shader model", LogColor::brightMagenta, shaderModelString);

				if (shaderModel < D3D_SHADER_MODEL_6_6)
				{
					LOGERROR(L"your graphics card should at least support D3D_SHADER_MODEL_6_6");
				}
			}

			{
				const BOOL typedUAVLoad = features.TypedUAVLoadAdditionalFormats();

				LOGENGINE(L"typed uav load", (typedUAVLoad ? LogColor::brightGreen : LogColor::brightRed), (typedUAVLoad ? L"YES" : L"NO"));

				if (!typedUAVLoad)
				{
					LOGERROR(L"your graphics card should support typed uav load");
				}
			}

			{
				const D3D12_RAYTRACING_TIER rayTracingTier = features.RaytracingTier();

				std::wstring raytracingTierString;

				switch (rayTracingTier)
				{
				default:
				case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
					raytracingTierString = L"D3D12_RAYTRACING_TIER_NOT_SUPPORTED";
					break;
				case D3D12_RAYTRACING_TIER_1_0:
					raytracingTierString = L"D3D12_RAYTRACING_TIER_1_0";
					break;
				case D3D12_RAYTRACING_TIER_1_1:
					raytracingTierString = L"D3D12_RAYTRACING_TIER_1_1";
					break;
				}

				LOGENGINE(L"raytracing tier", LogColor::brightMagenta, raytracingTierString);
			}
		}
	}

	ID3D12Device9* get()
	{
		return Internal::impl.device.Get();
	}
}
