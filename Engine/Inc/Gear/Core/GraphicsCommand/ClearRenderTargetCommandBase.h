#ifndef _GEAR_CORE_CLEARRENDERTARGETCOMMANDBASE_H_
#define _GEAR_CORE_CLEARRENDERTARGETCOMMANDBASE_H_

#include "GraphicsCommand.h"

namespace Gear::Core
{
	template<bool Instant, bool ClearDefRenderTarget>
	class ClearRenderTargetCommandBase :
		public GraphicsCommand,
		public CommandPool<ClearRenderTargetCommandBase<Instant, ClearDefRenderTarget>>
	{
	public:

		template<bool D = ClearDefRenderTarget>
			requires(!D)
		void set(const Resource::RenderTargetDesc& descParam, const float clearValueParam[4])
		{
			desc = descParam;

			memcpy(clearValue, clearValueParam, sizeof(float) * _countof(clearValue));
		}

		template<bool D = ClearDefRenderTarget>
			requires(D)
		void set(const float clearValueParam[4])
		{
			memcpy(clearValue, clearValueParam, sizeof(float) * _countof(clearValue));
		}

		void execute(GraphicsCommandExecuter& executer) override
		{
			if constexpr (ClearDefRenderTarget)
			{
				executer.clearDefRenderTarget(clearValue);
			}
			else
			{
				if constexpr (Instant)
				{
					executer.clearRenderTargetInstant(desc, clearValue);
				}
				else
				{
					executer.clearRenderTarget(desc, clearValue);
				}
			}
		}

	private:

		Resource::RenderTargetDesc desc;

		float clearValue[4];
	};

	using ClearRenderTargetCommand = ClearRenderTargetCommandBase<false, false>;

	using ClearRenderTargetInstantCommand = ClearRenderTargetCommandBase<true, false>;

	using ClearDefRenderTargetCommand = ClearRenderTargetCommandBase<true, true>;

}

#endif
