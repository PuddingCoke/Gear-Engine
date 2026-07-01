#include<Gear/Utils/Random.h>

#include<ctime>

#include<random>

namespace Gear::Utils::Random
{
	thread_local std::mt19937 randomEngine = std::mt19937(static_cast<uint32_t>(time(nullptr)));

	thread_local std::normal_distribution<float> normalDistribution = std::normal_distribution<float>(0.f, 1.f);

	thread_local std::uniform_real_distribution<float> floatDistribution = std::uniform_real_distribution<float>(0.f, 1.f);

	thread_local std::uniform_real_distribution<double> doubleDistribution = std::uniform_real_distribution<double>(0.0, 1.0);

	thread_local std::uniform_int_distribution<int32_t> intDistribution = std::uniform_int_distribution<int32_t>(INT32_MIN, INT32_MAX);

	thread_local std::uniform_int_distribution<uint32_t> uintDistribution = std::uniform_int_distribution<uint32_t>(0u, UINT32_MAX);

	float genGauss()
	{
		return normalDistribution(randomEngine);
	}

	float genFloat()
	{
		return floatDistribution(randomEngine);
	}

	double genDouble()
	{
		return doubleDistribution(randomEngine);
	}

	int32_t genInt()
	{
		return intDistribution(randomEngine);
	}

	uint32_t genUint()
	{
		return uintDistribution(randomEngine);
	}
}
