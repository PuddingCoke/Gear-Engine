#pragma once

#ifndef _GEAR_CORE_VIDEOENCODER_D3D12ENCODER_H_
#define _GEAR_CORE_VIDEOENCODER_D3D12ENCODER_H_

#include"Encoder.h"

#include<D3D12Headers/d3d12video.h>

namespace Gear::Core::VideoEncoder
{
	class D3D12Encoder
	{
	public:

	private:

		static constexpr D3D12_VIDEO_ENCODER_CODEC codec = D3D12_VIDEO_ENCODER_CODEC_H264;

		static constexpr D3D12_VIDEO_ENCODER_PROFILE_H264 profile = D3D12_VIDEO_ENCODER_PROFILE_H264_HIGH;

	};
}

#endif // !_GEAR_CORE_VIDEOENCODER_D3D12ENCODER_H_
