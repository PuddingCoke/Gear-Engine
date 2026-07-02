#pragma once

#include<Gear/Core/RenderTask.h>

#include<Gear/DevEssential.h>

class MyRenderTask :public RenderTask
{
public:

	MyRenderTask()
	{
		LOGUSER("窄字符串测试", "日志测试", "Log Testing", "Проверка журнала");

		LOGUSER(L"宽字符串测试", L"日志测试", L"Log Testing", L"Проверка журнала");

		LOGUSER("单线程测试");

		LOGUSER("32位有符号整数测试", IntegerMode::HEX, 12895, -123456, INT_MAX, INT_MIN, IntegerMode::DEC, 12895, -123456, INT_MAX, INT_MIN);

		LOGUSER("32位无符号整数测试", IntegerMode::HEX, 13689u, UINT_MAX, IntegerMode::DEC, 13689u, UINT_MAX);

		LOGUSER("64位有符号整数测试", IntegerMode::HEX, 1222222ll, -188888ll, INT64_MAX, INT64_MIN, IntegerMode::DEC, 1222222ll, -188888ll, INT64_MAX, INT64_MIN);

		LOGUSER("64位无符号整数测试", IntegerMode::HEX, 13579ull, UINT64_MAX, IntegerMode::DEC, 13579ull, UINT64_MAX);

		LOGUSER("32位浮点测试", FloatPrecision(4), 125.6f, FLT_MAX, FloatPrecision(2), 125.6f, FLT_MAX);

		LOGUSER("64位浮点测试", FloatPrecision(6), 200.123, DBL_MAX, FloatPrecision(5), 200.123, DBL_MAX);

		LOGUSER("多线程测试");

		const size_t threadCount = 16;

		std::vector<std::thread> workers;

		workers.reserve(threadCount);

		for (size_t i = 0; i < threadCount; ++i)
		{
			workers.emplace_back([i]()
				{
					for (int j = 0; j < 11; j++)
					{
						LOGUSER("子线程", i, "值", j);
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
