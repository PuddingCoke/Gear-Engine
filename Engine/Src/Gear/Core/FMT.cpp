#include <Gear/Core/FMT.h>

#include <Gear/Core/GraphicsDevice.h>

namespace Gear::Core::FMT
{
	PlaneCountType planeCountTable[formatTableLength] = {};

	FormatIndexType rtvFormatIndexTable[formatTableLength] = {};

	FormatIndexType dsvFormatIndexTable[formatTableLength] = {};

	uint32_t rtvFormatBits = 0u;

	uint32_t dsvFormatBits = 0u;

	namespace Internal
	{
		void initialize()
		{
			CD3DX12FeatureSupport featureSupport;

			featureSupport.Init(GraphicsDevice::get());

			//获取格式的平面个数
			for (FormatIndexType i = 0; i < formatTableLength; i++)
			{
				const DXGI_FORMAT format = static_cast<DXGI_FORMAT>(i);

				featureSupport.FormatInfo(format, planeCountTable[i]);
			}

			//获取RTV索引
			{
				FormatIndexType rtvFormatIndex = 0u;

				for (FormatIndexType i = 0; i < formatTableLength; i++)
				{
					const DXGI_FORMAT format = static_cast<DXGI_FORMAT>(i);

					D3D12_FORMAT_SUPPORT1 support1;

					D3D12_FORMAT_SUPPORT2 support2;

					featureSupport.FormatSupport(format, support1, support2);

					rtvFormatIndexTable[i] =
						(support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) ? rtvFormatIndex++ : 0u;
				}

				unsigned long index = 0;

				BitScanReverse(&index, rtvFormatIndex);

				rtvFormatBits = index + 1u;
			}

			//获取DSV索引
			{
				FormatIndexType dsvFormatIndex = 0u;

				for (FormatIndexType i = 0; i < formatTableLength; i++)
				{
					const DXGI_FORMAT format = static_cast<DXGI_FORMAT>(i);

					D3D12_FORMAT_SUPPORT1 support1;

					D3D12_FORMAT_SUPPORT2 support2;

					featureSupport.FormatSupport(format, support1, support2);

					dsvFormatIndexTable[i] =
						(support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) ? dsvFormatIndex++ : 0u;
				}

				unsigned long index = 0;

				BitScanReverse(&index, dsvFormatIndex);

				dsvFormatBits = index + 1u;
			}
		}
	}

	PlaneCountType getPlaneCount(const DXGI_FORMAT format)
	{
		return planeCountTable[static_cast<uint32_t>(format)];
	}

	FormatIndexType getRTVFormatIndex(const DXGI_FORMAT format)
	{
		return rtvFormatIndexTable[static_cast<uint32_t>(format)];
	}

	FormatIndexType getDSVFormatIndex(const DXGI_FORMAT format)
	{
		return dsvFormatIndexTable[static_cast<uint32_t>(format)];
	}

	uint32_t getRTVFormatBits()
	{
		return rtvFormatBits;
	}

	uint32_t getDSVFormatBits()
	{
		return dsvFormatBits;
	}
}
