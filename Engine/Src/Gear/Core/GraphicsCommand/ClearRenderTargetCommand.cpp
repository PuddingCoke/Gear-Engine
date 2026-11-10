#include <Gear/Core/GraphicsCommand/ClearRenderTargetCommand.h>

namespace Gear::Core
{
	ClearRenderTargetCommand::ClearRenderTargetCommand(const Resource::RenderTargetDesc& desc,
	                                                   const float clearValue[4],
	                                                   const bool instant)
	{
		set(desc, clearValue, instant);
	}

	void ClearRenderTargetCommand::set(const Resource::RenderTargetDesc& desc,
	                                   const float clearValue[4],
	                                   const bool instant)
	{
		this->desc = desc;

		memcpy(this->clearValue, clearValue, sizeof(float) * _countof(this->clearValue));

		this->instant = instant;
	}

	void ClearRenderTargetCommand::execute(GraphicsCommandExecuter& executer)
	{
		if (instant)
		{
			executer.clearRenderTargetInstant(desc, clearValue);
		}
		else
		{
			if(desc.texture)
			{
				executer.clearRenderTarget(desc, clearValue);
			}
			else
			{
				executer.clearDefRenderTarget(clearValue);
			}
		}
	}
}
