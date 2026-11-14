#ifndef _GEAR_CORE_DRAWCOMMANDBASE_H_
#define _GEAR_CORE_DRAWCOMMANDBASE_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool DrawIndexed>
	class DrawCommandBase : public GraphicsCommand, public CommandPool<DrawCommandBase<DrawIndexed>>
	{
	public:

		template<bool D = DrawIndexed>
			requires(!D)
		void set(const uint64_t pixelShaderResourceDescPtrOffset,
		         const uint32_t numPixelShaderResourceDesc,
				 const uint64_t nonPixelShaderResourceDescPtrOffset,
		         const uint32_t numNonPixelShaderResourceDesc,
		         const uint32_t vertexCountPerInstance,
		         const uint32_t instanceCount,
		         const uint32_t startVertexLocation,
		         const uint32_t startInstanceLocation)
		{
			this->pixelShaderResourceDescPtrOffset = pixelShaderResourceDescPtrOffset;

			this->numPixelShaderResourceDesc = numPixelShaderResourceDesc;

			this->nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffset;

			this->numNonPixelShaderResourceDesc = numNonPixelShaderResourceDesc;

			this->drawData.vertexCountPerInstance = vertexCountPerInstance;

			this->drawData.instanceCount = instanceCount;

			this->drawData.startVertexLocation = startVertexLocation;

			this->drawData.startInstanceLocation = startInstanceLocation;
		}

		template<bool D = DrawIndexed>
			requires(D)
		void set(const uint64_t pixelShaderResourceDescPtrOffset,
		         const uint32_t numPixelShaderResourceDesc,
				 const uint64_t nonPixelShaderResourceDescPtrOffset,
		         const uint32_t numNonPixelShaderResourceDesc,
		         const uint32_t indexCountPerInstance,
		         const uint32_t instanceCount,
		         const uint32_t startIndexLocation,
		         const int32_t baseVertexLocation,
		         const uint32_t startInstanceLocation)
		{
			this->pixelShaderResourceDescPtrOffset = pixelShaderResourceDescPtrOffset;

			this->numPixelShaderResourceDesc = numPixelShaderResourceDesc;

			this->nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffset;

			this->numNonPixelShaderResourceDesc = numNonPixelShaderResourceDesc;

			this->drawIndexedData.indexCountPerInstance = indexCountPerInstance;

			this->drawIndexedData.instanceCount = instanceCount;

			this->drawIndexedData.startIndexLocation = startIndexLocation;

			this->drawIndexedData.baseVertexLocation = baseVertexLocation;

			this->drawIndexedData.startInstanceLocation = startInstanceLocation;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			executer.setPixelShaderResources(pixelShaderResourceDescPtrOffset, numPixelShaderResourceDesc);

			executer.setNonPixelShaderResources(nonPixelShaderResourceDescPtrOffset,
			                                    numNonPixelShaderResourceDesc);

			if constexpr (DrawIndexed)
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

	private:

		uint64_t pixelShaderResourceDescPtrOffset;

		uint32_t numPixelShaderResourceDesc;

		uint64_t nonPixelShaderResourceDescPtrOffset;

		uint32_t numNonPixelShaderResourceDesc;

		union
		{
			struct DrawData
			{
				uint32_t vertexCountPerInstance;

				uint32_t instanceCount;

				uint32_t startVertexLocation;

				uint32_t startInstanceLocation;

			} drawData;

			struct DrawIndexedData
			{
				uint32_t indexCountPerInstance;

				uint32_t instanceCount;

				uint32_t startIndexLocation;

				int32_t baseVertexLocation;

				uint32_t startInstanceLocation;

			} drawIndexedData;
		};
	};

	using DrawCommand = DrawCommandBase<false>;

	using DrawIndexedCommand = DrawCommandBase<true>;

}

#endif // _GEAR_CORE_DRAWCOMMANDBASE_H_
