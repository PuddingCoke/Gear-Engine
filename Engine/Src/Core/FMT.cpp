#include<Gear/Core/FMT.h>

uint32_t Gear::Core::FMT::getByteSize(const DXGI_FORMAT format)
{
    switch (format)
    {
    case FMT::RGBA32TL:
    case FMT::RGBA32F:
    case FMT::RGBA32UI:
    case FMT::RGBA32I:
        return 16;

    case FMT::RGB32TL:
    case FMT::RGB32F:
    case FMT::RGB32UI:
    case FMT::RGB32I:
        return 12;

    case FMT::RGBA16TL:
    case FMT::RGBA16F:
    case FMT::RGBA16UN:
    case FMT::RGBA16UI:
    case FMT::RGBA16SN:
    case FMT::RGBA16I:
    case FMT::RG32TL:
    case FMT::RG32F:
    case FMT::RG32UI:
    case FMT::RG32I:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
        return 8;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case FMT::RG11B10F:
    case FMT::RGBA8TL:
    case FMT::RGBA8UN:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case FMT::RGBA8UI:
    case FMT::RGBA8SN:
    case FMT::RGBA8I:
    case FMT::RG16TL:
    case FMT::RG16F:
    case FMT::RG16UN:
    case FMT::RG16UI:
    case FMT::RG16SN:
    case FMT::RG16I:
    case FMT::R32TL:
    case FMT::D32F:
    case FMT::R32F:
    case FMT::R32UI:
    case FMT::R32I:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case FMT::BGRA8UN:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_YUY2:
        return 4;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
    case DXGI_FORMAT_V408:
        return 3;

    case FMT::RG8TL:
    case FMT::RG8UN:
    case FMT::RG8UI:
    case FMT::RG8SN:
    case FMT::RG8I:
    case FMT::R16TL:
    case FMT::R16F:
    case FMT::D16UN:
    case FMT::R16UN:
    case FMT::R16UI:
    case FMT::R16SN:
    case FMT::R16I:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
    case DXGI_FORMAT_P208:
    case DXGI_FORMAT_V208:
        return 2;

    case FMT::R8TL:
    case FMT::R8UN:
    case FMT::R8UI:
    case FMT::R8SN:
    case FMT::R8I:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
        return 1;

    default:
        return 0;
    }
}
