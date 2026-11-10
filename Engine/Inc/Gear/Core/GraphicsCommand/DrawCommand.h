#ifndef _GEAR_CORE_DRAWCOMMAND_H_
#define _GEAR_CORE_DRAWCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class DrawCommand : public GraphicsCommand
	{
	public:

		DrawCommand() = delete;

		DrawCommand(const uint64_t pixelShaderResourceDescPtrOffset,
		            const uint64_t nonPixelShaderResourceDescPtrOffset,
		            const uint32_t numPixelShaderResourceDesc,
		            const uint32_t numNonPixelShaderResourceDesc,
		            const uint32_t vertexCountPerInstance,
		            const uint32_t instanceCount,
		            const uint32_t startVertexLocation,
		            const uint32_t startInstanceLocation);

        DrawCommand(const uint64_t pixelShaderResourceDescPtrOffset,
                    const uint64_t nonPixelShaderResourceDescPtrOffset,
                    const uint32_t numPixelShaderResourceDesc,
                    const uint32_t numNonPixelShaderResourceDesc,
                    const uint32_t indexCountPerInstance,
                    const uint32_t instanceCount,
                    const uint32_t startIndexLocation,
                    const int32_t baseVertexLocation,
                    const uint32_t startInstanceLocation);

		void set(const uint64_t pixelShaderResourceDescPtrOffset,
		         const uint64_t nonPixelShaderResourceDescPtrOffset,
		         const uint32_t numPixelShaderResourceDesc,
		         const uint32_t numNonPixelShaderResourceDesc,
		         const uint32_t vertexCountPerInstance,
		         const uint32_t instanceCount,
		         const uint32_t startVertexLocation,
		         const uint32_t startInstanceLocation);

        void set(const uint64_t pixelShaderResourceDescPtrOffset,
                 const uint64_t nonPixelShaderResourceDescPtrOffset,
                 const uint32_t numPixelShaderResourceDesc,
                 const uint32_t numNonPixelShaderResourceDesc,
                 const uint32_t indexCountPerInstance,
                 const uint32_t instanceCount,
                 const uint32_t startIndexLocation,
                 const int32_t baseVertexLocation,
                 const uint32_t startInstanceLocation);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		uint64_t pixelShaderResourceDescPtrOffset;

		uint64_t nonPixelShaderResourceDescPtrOffset;

		uint32_t numPixelShaderResourceDesc;

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

        bool drawIndexed;

	};
}

#endif // _GEAR_CORE_DRAWCOMMAND_H_
