#pragma once

#include <Gear/Core/RenderTask.h>

#include <Gear/DevEssential.h>

class MyRenderTask : public RenderTask
{
public:

	MyRenderTask()
	{
		LOGUSER() << "窄字符串测试" << "日志测试" << "Log Testing" << "Проверка журнала";

		LOGUSER() << L"宽字符串测试" << L"日志测试" << L"Log Testing" << L"Проверка журнала";

		LOGUSER() << u8"UTF8字符串测试" << u8"日志测试" << u8"Log Testing" << u8"Проверка журнала";

		LOGUSER() << "表情" << "😎😂🎉🔥👍🏻";

		LOGUSER() << "组合字符" << "e\u0301 ñ";

		LOGUSER() << "零宽空间" << "Hel\u200Blo";

		LOGUSER() << "极长连字" << "﷽";

		LOGUSER() << "单线程测试";

		LOGUSER() << "32位有符号整数测试" << LogIntegerMode::HEX << 12895 << -123456 << INT_MAX << INT_MIN
				  << LogIntegerMode::DEC << 12895 << -123456 << INT_MAX << INT_MIN;

		LOGUSER() << "32位无符号整数测试" << LogIntegerMode::HEX << 13689u << UINT_MAX << LogIntegerMode::DEC
				  << 13689u << UINT_MAX;

		LOGUSER() << "64位有符号整数测试" << LogIntegerMode::HEX << 1222222ll << -188888ll << INT64_MAX
				  << INT64_MIN << LogIntegerMode::DEC << 1222222ll << -188888ll << INT64_MAX << INT64_MIN;

		LOGUSER() << "64位无符号整数测试" << LogIntegerMode::HEX << 13579ull << UINT64_MAX
				  << LogIntegerMode::DEC << 13579ull << UINT64_MAX;

		LOGUSER() << "32位浮点测试" << LogFloatPrecision(4) << 125.6f << FLT_MAX << LogFloatPrecision(2)
				  << 125.6f << FLT_MAX;

		LOGUSER() << "64位浮点测试" << LogFloatPrecision(6) << 200.123 << DBL_MAX << LogFloatPrecision(5)
				  << 200.123 << DBL_MAX;

		LOGUSER() << "多线程测试";

		const size_t threadCount = 16;

		std::vector<std::thread> workers;

		workers.reserve(threadCount);

		for (size_t i = 0; i < threadCount; ++i)
		{
			workers.emplace_back([i]()
			{
				for (int j = 0; j < 11; j++)
				{
					LOGUSER() << "子线程" << i << "值" << j;
				}
			});
		}

		for (auto& t : workers)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
	}

	~MyRenderTask()
	{
	}

protected:

	void recordCommand() override
	{
		context->setDefRenderTarget();

		context->clearDefRenderTarget(DirectX::Colors::Cyan);
	}

private:
};
