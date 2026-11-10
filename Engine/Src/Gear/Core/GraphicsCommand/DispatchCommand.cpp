#include <Gear/Core/GraphicsCommand/DispatchCommand.h>

namespace Gear::Core
{
	DispatchCommand::DispatchCommand(const uint64_t nonPixelShaderResourceDescPtrOffset,
	                                 const uint32_t numNonPixelShaderResourceDesc,
	                                 const uint32_t threadGroupCountX,
	                                 const uint32_t threadGroupCountY,
	                                 const uint32_t threadGroupCountZ,
	                                 const bool dispatchGrp)
	{
		set(nonPixelShaderResourceDescPtrOffset,
		    numNonPixelShaderResourceDesc,
		    threadGroupCountX,
		    threadGroupCountY,
		    threadGroupCountZ,
		    dispatchGrp);
	}

	void DispatchCommand::set(const uint64_t nonPixelShaderResourceDescPtrOffset,
	                          const uint32_t numNonPixelShaderResourceDesc,
	                          const uint32_t threadGroupCountX,
	                          const uint32_t threadGroupCountY,
	                          const uint32_t threadGroupCountZ,
	                          const bool dispatchGrp)
	{
		this->nonPixelShaderResourceDescPtrOffset = nonPixelShaderResourceDescPtrOffset;

		this->numNonPixelShaderResourceDesc = numNonPixelShaderResourceDesc;

		this->threadGroupCountX = threadGroupCountX;

		this->threadGroupCountY = threadGroupCountY;

		this->threadGroupCountZ = threadGroupCountZ;

		this->dispatchGrp = dispatchGrp;
	}

	void DispatchCommand::execute(GraphicsCommandExecuter& executer)
	{
		executer.setNonPixelShaderResources(nonPixelShaderResourceDescPtrOffset,
		                                    numNonPixelShaderResourceDesc);

		if (dispatchGrp)
		{
			executer.dispatchGrp(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
		}
		else
		{
			executer.dispatchDim(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
		}
	}
}
