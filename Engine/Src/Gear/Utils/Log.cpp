#include<Gear/Utils/Log.h>

#include<Gear/Utils/Logger/Logger.h>

#include<stdexcept>

namespace Gear::Utils::Logger
{
	Log::Log(const std::string_view& functionName, const LogType& type) :
		context(LogContext::get())
	{
		context.createLogMessage(functionName, type);
	}

	Log::~Log()
	{
		finishAndPostMessage();
	}

	bool Log::needThrow() const
	{
		return context.getLogType() == LogType::LOG_ERROR;
	}

	void Log::finishAndPostMessage() const
	{
		//如果需要抛出异常，则不在这里提交日志信息
		if (!needThrow())
		{
			context.packArgument(LogNewLine());

			Logger::submitLogMessage(context.getLogMessage());
		}
	}

	void Log::finishAndThrowMessage() const
	{
		//检查一下是否真的需要抛出信息
		if (needThrow())
		{
			context.packArgument(LogNewLine());

			const LogMessage message = context.getLogMessage();

			const std::string errorStr = message.str;

			Logger::submitLogMessage(message);

            throw std::runtime_error(errorStr);
		}
	}

	[[noreturn]] void ThrowLog(const Log& log)
	{
		log.finishAndThrowMessage();

		throw std::runtime_error("THROWLOG不能被用于非错误类型的日志！");
	}
}
