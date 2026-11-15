#include <Gear/Core/GraphicsCommand/LambdaCommand.h>

namespace Gear::Core
{
	void LambdaCommand::set(std::function<void(GraphicsCommandExecuter&)>&& lambda, const std::string& name)
	{
		this->lambda = std::move(lambda);

		this->name = name;
	}

	void LambdaCommand::execute(GraphicsCommandExecuter& executer)
	{
#ifdef _DEBUG
		if (!lambda)
		{
			THROWLOG(LOGERROR() << "LambdaCommand 执行前必须调用 set");
		}
#endif

		lambda(executer);

		lambda = nullptr;
	}

	GraphicsCommand* makeLambdaCommand(std::function<void(GraphicsCommandExecuter&)>&& func,
	                                   const std::string& name)
	{
		auto* const command = LambdaCommand::acquire();

		command->set(std::move(func), name);

		return command;
	}

}
