#ifndef _GEAR_CORE_DISPATCHCOMMAND_H_
#define _GEAR_CORE_DISPATCHCOMMAND_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	class DispatchCommand : public GraphicsCommand
	{
	public:

		DispatchCommand() = delete;

		DispatchCommand(const uint64_t nonPixelShaderResourceDescPtrOffset,
		                const uint32_t numNonPixelShaderResourceDesc,
		                const uint32_t threadGroupCountX,
		                const uint32_t threadGroupCountY,
		                const uint32_t threadGroupCountZ,
		                const bool dispatchGrp);

		void set(const uint64_t nonPixelShaderResourceDescPtrOffset,
		         const uint32_t numNonPixelShaderResourceDesc,
		         const uint32_t threadGroupCountX,
		         const uint32_t threadGroupCountY,
		         const uint32_t threadGroupCountZ,
		         const bool dispatchGrp);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		uint64_t nonPixelShaderResourceDescPtrOffset;

		uint32_t numNonPixelShaderResourceDesc;

		uint32_t threadGroupCountX;

		uint32_t threadGroupCountY;

		uint32_t threadGroupCountZ;

		bool dispatchGrp;
	};
}

#endif // _GEAR_CORE_DISPATCHCOMMAND_H_
