#include <Gear/Core/Device.h>

#include <Gear/Core/GraphicsDevice.h>

#include <Gear/Core/VideoDevice.h>

namespace Gear::Core::Device
{
	struct DeviceImpl
	{
		ComPtr<ID3D12Device9> device;

		ComPtr<ID3D12VideoDevice3> videoDevice;

	} impl;

	namespace Internal
	{
		void initialize(IUnknown* const adapter)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&impl.device))))
			{
				LOGSUCCESS() << "创建" << COLORIZEENUM(D3D_FEATURE_LEVEL_12_0) << "特性等级的"
							 << COLORIZESTRUCT(ID3D12Device);
			}
			else
			{
				THROWLOG(LOGERROR() << "找到的性能最强适配器不支持" << COLORIZEENUM(D3D_FEATURE_LEVEL_12_0));
			}

			CHECKERROR(impl.device->QueryInterface(IID_PPV_ARGS(&impl.videoDevice)));
		}

		void release()
		{
			impl.device = nullptr;

			impl.videoDevice = nullptr;
		}
	}

	void checkFeatureSupport()
	{
		LOGENGINE() << "以下是特性支持信息";

		CD3DX12FeatureSupport features;

		features.Init(impl.device.Get());

		{
			const D3D12_RESOURCE_BINDING_TIER resourceBindingTier = features.ResourceBindingTier();

			switch (resourceBindingTier)
			{
			default:
			case D3D12_RESOURCE_BINDING_TIER_1:
				LOGENGINE() << "资源绑定等级" << COLORIZEENUM(D3D12_RESOURCE_BINDING_TIER_1);
				break;
			case D3D12_RESOURCE_BINDING_TIER_2:
				LOGENGINE() << "资源绑定等级" << COLORIZEENUM(D3D12_RESOURCE_BINDING_TIER_2);
				break;
			case D3D12_RESOURCE_BINDING_TIER_3:
				LOGENGINE() << "资源绑定等级" << COLORIZEENUM(D3D12_RESOURCE_BINDING_TIER_3);
				break;
			}
		}

		{
			const D3D_SHADER_MODEL shaderModel = features.HighestShaderModel();

			switch (shaderModel)
			{
			default:
			case D3D_SHADER_MODEL_5_1:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_5_1);
				break;
			case D3D_SHADER_MODEL_6_0:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_0);
				break;
			case D3D_SHADER_MODEL_6_1:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_1);
				break;
			case D3D_SHADER_MODEL_6_2:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_2);
				break;
			case D3D_SHADER_MODEL_6_3:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_3);
				break;
			case D3D_SHADER_MODEL_6_4:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_4);
				break;
			case D3D_SHADER_MODEL_6_5:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_5);
				break;
			case D3D_SHADER_MODEL_6_6:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_6);
				break;
			case D3D_SHADER_MODEL_6_7:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_7);
				break;
			case D3D_SHADER_MODEL_6_8:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_8);
				break;
			case D3D_SHADER_MODEL_6_9:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_9);
				break;
			case D3D_SHADER_MODEL_6_10:
				LOGENGINE() << "最高支持的着色模型" << COLORIZEENUM(D3D_SHADER_MODEL_6_10);
				break;
			}

			if (shaderModel < D3D_SHADER_MODEL_6_6)
			{
				THROWLOG(LOGERROR() << "你的适配器不支持" << COLORIZEENUM(D3D_SHADER_MODEL_6_6));
			}
		}

		{
			const bool typedUAVLoad = features.TypedUAVLoadAdditionalFormats();

			LOGENGINE() << "有类型UAV读取" << typedUAVLoad;

			if (!typedUAVLoad)
			{
				THROWLOG(LOGERROR() << "你的适配器不支持有类型UAV读取");
			}
		}

		{
			const D3D12_RAYTRACING_TIER rayTracingTier = features.RaytracingTier();

			switch (rayTracingTier)
			{
			default:
			case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
				LOGENGINE() << "光线追踪等级" << COLORIZEENUM(D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
				break;
			case D3D12_RAYTRACING_TIER_1_0:
				LOGENGINE() << "光线追踪等级" << COLORIZEENUM(D3D12_RAYTRACING_TIER_1_0);
				break;
			case D3D12_RAYTRACING_TIER_1_1:
				LOGENGINE() << "光线追踪等级" << COLORIZEENUM(D3D12_RAYTRACING_TIER_1_1);
				break;
			case D3D12_RAYTRACING_TIER_1_2:
				LOGENGINE() << "光线追踪等级" << COLORIZEENUM(D3D12_RAYTRACING_TIER_1_2);
				break;
			}
		}
	}
}

namespace Gear::Core::GraphicsDevice
{
	ID3D12Device9* get()
	{
		return Device::impl.device.Get();
	}
}

namespace Gear::Core::VideoDevice
{
	ID3D12VideoDevice3* get()
	{
		return Device::impl.videoDevice.Get();
	}
}
