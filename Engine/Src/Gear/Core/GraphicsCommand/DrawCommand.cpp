#include <Gear/Core/GraphicsCommand/DrawCommand.h>

namespace Gear::Core
{
	DrawCommand::DrawCommand(const uint64_t pixelShaderResourceDescPtrOffset,
	                         const uint64_t nonPixelShaderResourceDescPtrOffset,
	                         const uint32_t numPixelShaderResourceDesc,
	                         const uint32_t numNonPixelShaderResourceDesc,
	                         const uint32_t vertexCountPerInstance,
	                         const uint32_t instanceCount,
	                         const uint32_t startVertexLocation,
	                         const uint32_t startInstanceLocation)
	{
		set(pixelShaderResourceDescPtrOffset,
		    nonPixelShaderResourceDescPtrOffset,
		    numPixelShaderResourceDesc,
		    numNonPixelShaderResourceDesc,
		    vertexCountPerInstance,
		    instanceCount,
		    startVertexLocation,
		    startInstanceLocation);
	}

	DrawCommand::DrawCommand(const uint64_t pixelShaderResourceDescPtrOffset,
	                         const uint64_t nonPixelShaderResourceDescPtrOffset,
	                         const uint32_t numPixelShaderResourceDesc,
	                         const uint32_t numNonPixelShaderResourceDesc,
	                         const uint32_t indexCountPerInstance,
	                         const uint32_t instanceCount,
	                         const uint32_t startIndexLocation,
	                         const int32_t baseVertexLocation,
	                         const uint32_t startInstanceLocation)
	{
		set(pixelShaderResourceDescPtrOffset,
		    nonPixelShaderResourceDescPtrOffset,
		    numPixelShaderResourceDesc,
		    numNonPixelShaderResourceDesc,
		    indexCountPerInstance,
		    instanceCount,
		    startIndexLocation,
		    baseVertexLocation,
		    startInstanceLocation);
	}

	void DrawCommand::set(const uint64_t pixelShaderResourceDescPtrOffset,
	                      const uint64_t nonPixelShaderResourceDescPtrOffset,
	                      const uint32_t numPixelShaderResourceDesc,
	                      const uint32_t numNonPixelShaderResourceDesc,
	                      const uint32_t vertexCountPerInstance,
	                      const uint32_t instanceCount,
	                      const uint32_t startVertexLocation,
	                      const uint32_t startInstanceLocation)
	{
		this->pixelShaderResourceDescPtrOffset = pixelShaderResourceDescPtrOffset;

		this->nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffset;

		this->numPixelShaderResourceDesc = numPixelShaderResourceDesc;

		this->numNonPixelShaderResourceDesc = numNonPixelShaderResourceDesc;

		this->drawData.vertexCountPerInstance = vertexCountPerInstance;

		this->drawData.instanceCount = instanceCount;

		this->drawData.startVertexLocation = startVertexLocation;

		this->drawData.startInstanceLocation = startInstanceLocation;

		this->drawIndexed = false;
	}

	void DrawCommand::set(const uint64_t pixelShaderResourceDescPtrOffset,
	                      const uint64_t nonPixelShaderResourceDescPtrOffset,
	                      const uint32_t numPixelShaderResourceDesc,
	                      const uint32_t numNonPixelShaderResourceDesc,
	                      const uint32_t indexCountPerInstance,
	                      const uint32_t instanceCount,
	                      const uint32_t startIndexLocation,
	                      const int32_t baseVertexLocation,
	                      const uint32_t startInstanceLocation)
	{
		this->pixelShaderResourceDescPtrOffset = pixelShaderResourceDescPtrOffset;

		this->nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffset;

		this->numPixelShaderResourceDesc = numPixelShaderResourceDesc;

		this->numNonPixelShaderResourceDesc = numNonPixelShaderResourceDesc;

		this->drawIndexedData.indexCountPerInstance = indexCountPerInstance;

		this->drawIndexedData.instanceCount = instanceCount;

		this->drawIndexedData.startIndexLocation = startIndexLocation;

		this->drawIndexedData.baseVertexLocation = baseVertexLocation;

		this->drawIndexedData.startInstanceLocation = startInstanceLocation;

		this->drawIndexed = true;
	}

	void DrawCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setPixelShaderResources(pixelShaderResourceDescPtrOffset, numPixelShaderResourceDesc);

		executer.setNonPixelShaderResources(nonPixelShaderResourceDescPtrOffset,
		                                    numNonPixelShaderResourceDesc);

		if (drawIndexed)
		{
			executer.drawIndexed(drawIndexedData.indexCountPerInstance,
			                     drawIndexedData.instanceCount,
			                     drawIndexedData.startIndexLocation,
			                     drawIndexedData.baseVertexLocation,
			                     drawIndexedData.startInstanceLocation);
		}
		else
		{
			executer.draw(drawData.vertexCountPerInstance,
			              drawData.instanceCount,
			              drawData.startVertexLocation,
			              drawData.startInstanceLocation);
		}
	}
}
