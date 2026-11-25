#ifndef _GEAR_CORE_DISPATCHCOMMANDBASE_H_
#define _GEAR_CORE_DISPATCHCOMMANDBASE_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool DispatchGrp>
	class DispatchCommandBase : public GraphicsCommand, public CommandPool<DispatchCommandBase<DispatchGrp>>
	{
	public:

		void set(const uint64_t allShaderResourceDescPtrOffsetParam,
		         const uint32_t numAllShaderResourceDescParam,
		         const uint64_t nonPixelShaderResourceDescPtrOffsetParam,
		         const uint32_t numNonPixelShaderResourceDescParam,
		         const uint32_t dispatchCountXParam,
		         const uint32_t dispatchCountYParam,
		         const uint32_t dispatchCountZParam)
		{
			allShaderResourceDescPtrOffset = allShaderResourceDescPtrOffsetParam;

			numAllShaderResourceDesc = numAllShaderResourceDescParam;

			nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffsetParam;

			numNonPixelShaderResourceDesc = numNonPixelShaderResourceDescParam;

			dispatchCountX = dispatchCountXParam;

			dispatchCountY = dispatchCountYParam;

			dispatchCountZ = dispatchCountZParam;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			executer.setAllShaderResources(allShaderResourceDescPtrOffset,
			                               numAllShaderResourceDesc,
			                               D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			executer.setNonPixelShaderResources(nonPixelShaderResourceDescPtrOffset,
			                                    numNonPixelShaderResourceDesc);

			if constexpr (DispatchGrp)
			{
				executer.dispatchGrp(dispatchCountX, dispatchCountY, dispatchCountZ);
			}
			else
			{
				executer.dispatchDim(dispatchCountX, dispatchCountY, dispatchCountZ);
			}
		}

	private:

		uint64_t allShaderResourceDescPtrOffset;

		uint32_t numAllShaderResourceDesc;

		uint64_t nonPixelShaderResourceDescPtrOffset;

		uint32_t numNonPixelShaderResourceDesc;

		uint32_t dispatchCountX;

		uint32_t dispatchCountY;

		uint32_t dispatchCountZ;
	};

	using DispatchDimCommand = DispatchCommandBase<false>;

	using DispatchGrpCommand = DispatchCommandBase<true>;

}

#endif // _GEAR_CORE_DISPATCHCOMMANDBASE_H_
