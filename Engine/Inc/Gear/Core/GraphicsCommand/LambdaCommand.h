#ifndef _GEAR_CORE_LAMBDACOMMAND_H_
#define _GEAR_CORE_LAMBDACOMMAND_H_

#include "GraphicsCommand.h"

#include <functional>

#include <string>

namespace Gear::Core
{
	class LambdaCommand final : public GraphicsCommand, public CommandPool<LambdaCommand>
	{
	public:

		void set(std::function<void(GraphicsCommandExecuter&)>&& lambdaParam, const std::string& nameParam);

		void execute(GraphicsCommandExecuter& executer) override;

	private:

		std::function<void(GraphicsCommandExecuter&)> lambda;

		std::string name;
	};

	GraphicsCommand* makeLambdaCommand(std::function<void(GraphicsCommandExecuter&)>&& func,
	                                   const std::string& name = "");

}

#endif
