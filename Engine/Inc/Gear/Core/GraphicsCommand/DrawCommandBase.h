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
		void set(const uint64_t allShaderResourceDescPtrOffsetParam,
		         const uint32_t numAllShaderResourceDescParam,
		         const uint64_t pixelShaderResourceDescPtrOffsetParam,
		         const uint32_t numPixelShaderResourceDescParam,
		         const uint64_t nonPixelShaderResourceDescPtrOffsetParam,
		         const uint32_t numNonPixelShaderResourceDescParam,
		         const uint32_t vertexCountPerInstanceParam,
		         const uint32_t instanceCountParam,
		         const uint32_t startVertexLocationParam,
		         const uint32_t startInstanceLocationParam)
		{
			allShaderResourceDescPtrOffset = allShaderResourceDescPtrOffsetParam;

			numAllShaderResourceDesc = numAllShaderResourceDescParam;

			pixelShaderResourceDescPtrOffset = pixelShaderResourceDescPtrOffsetParam;

			numPixelShaderResourceDesc = numPixelShaderResourceDescParam;

			nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffsetParam;

			numNonPixelShaderResourceDesc = numNonPixelShaderResourceDescParam;

			drawData.vertexCountPerInstance = vertexCountPerInstanceParam;

			drawData.instanceCount = instanceCountParam;

			drawData.startVertexLocation = startVertexLocationParam;

			drawData.startInstanceLocation = startInstanceLocationParam;
		}

		template<bool D = DrawIndexed>
			requires(D)
		void set(const uint64_t allShaderResourceDescPtrOffsetParam,
		         const uint32_t numAllShaderResourceDescParam,
		         const uint64_t pixelShaderResourceDescPtrOffsetParam,
		         const uint32_t numPixelShaderResourceDescParam,
		         const uint64_t nonPixelShaderResourceDescPtrOffsetParam,
		         const uint32_t numNonPixelShaderResourceDescParam,
		         const uint32_t indexCountPerInstanceParam,
		         const uint32_t instanceCountParam,
		         const uint32_t startIndexLocationParam,
		         const int32_t baseVertexLocationParam,
		         const uint32_t startInstanceLocationParam)
		{
			allShaderResourceDescPtrOffset = allShaderResourceDescPtrOffsetParam;

			numAllShaderResourceDesc = numAllShaderResourceDescParam;

			pixelShaderResourceDescPtrOffset = pixelShaderResourceDescPtrOffsetParam;

			numPixelShaderResourceDesc = numPixelShaderResourceDescParam;

			nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffsetParam;

			numNonPixelShaderResourceDesc = numNonPixelShaderResourceDescParam;

			drawIndexedData.indexCountPerInstance = indexCountPerInstanceParam;

			drawIndexedData.instanceCount = instanceCountParam;

			drawIndexedData.startIndexLocation = startIndexLocationParam;

			drawIndexedData.baseVertexLocation = baseVertexLocationParam;

			drawIndexedData.startInstanceLocation = startInstanceLocationParam;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			executer.setAllShaderResources(allShaderResourceDescPtrOffset, numAllShaderResourceDesc);

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

		uint64_t allShaderResourceDescPtrOffset;

		uint32_t numAllShaderResourceDesc;

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
