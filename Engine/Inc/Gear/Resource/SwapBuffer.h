#pragma once

#ifndef _GEAR_RESOURCE_SWAPBUFFER_H_
#define _GEAR_RESOURCE_SWAPBUFFER_H_

#include"BufferView.h"

#include"SwappableBase.h"

namespace Gear::Resource
{
	CREATESAFETYPE(SwapBuffer);

	class SwapBuffer :public SwappableBase<BufferView>
	{
	public:

		SwapBuffer() = delete;

		SwapBuffer(const std::function<BufferViewPtr(void)>& readBufferFunc, const std::function<BufferViewPtr(void)>& writeBufferFunc);

		VertexBufferDesc getVertexBuffer() const;

		IndexBufferDesc getIndexBuffer() const;

		ShaderResourceDesc getSRVIndex() const;

		ShaderResourceDesc getUAVIndex() const;

		UAVClearDesc getUAVClearDesc() const;

	};

}

#endif // !_GEAR_RESOURCE_SWAPBUFFER_H_
