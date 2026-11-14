#ifndef _GEAR_CORE_DISPATCHCOMMANDBASE_H_
#define _GEAR_CORE_DISPATCHCOMMANDBASE_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool DispatchGrp>
	class DispatchCommandBase : public GraphicsCommand, public CommandPool<DispatchCommandBase<DispatchGrp>>
	{
	public:

		void set(const uint64_t nonPixelShaderResourceDescPtrOffset,
		         const uint32_t numNonPixelShaderResourceDesc,
		         const uint32_t dispatchCountX,
		         const uint32_t dispatchCountY,
		         const uint32_t dispatchCountZ)
		{
			this->nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffset;

			this->numNonPixelShaderResourceDesc = numNonPixelShaderResourceDesc;

			this->dispatchCountX = dispatchCountX;

			this->dispatchCountY = dispatchCountY;

			this->dispatchCountZ = dispatchCountZ;
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
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
