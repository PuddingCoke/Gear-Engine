#include<Gear/Resource/SwapBuffer.h>

namespace Gear::Resource
{
	SwapBuffer::SwapBuffer(const std::function<BufferViewPtr(void)>& readBufferFunc, const std::function<BufferViewPtr(void)>& writeBufferFunc) :
		SwappableBase(readBufferFunc, writeBufferFunc)
	{
	}

	VertexBufferDesc SwapBuffer::getVertexBuffer() const
	{
		return read()->getVertexBuffer();
	}

	IndexBufferDesc SwapBuffer::getIndexBuffer() const
	{
		return read()->getIndexBuffer();
	}

	ShaderResourceDesc SwapBuffer::getSRVIndex() const
	{
		return read()->getSRVIndex();
	}

	ShaderResourceDesc SwapBuffer::getUAVIndex() const
	{
		return write()->getUAVIndex();
	}

	UAVClearDesc SwapBuffer::getUAVClearDesc() const
	{
		return write()->getUAVClearDesc();
	}

	SwapBuffer::operator VertexBufferDesc() const
	{
		return getVertexBuffer();
	}

	SwapBuffer::operator IndexBufferDesc() const
	{
		return getIndexBuffer();
	}
}
