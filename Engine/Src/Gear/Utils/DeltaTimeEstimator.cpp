#include<Gear/Utils/DeltaTimeEstimator.h>

#include<algorithm>

namespace Gear::Utils
{
	DeltaTimeEstimator::DeltaTimeEstimator() :
		historyDeltatime{}, sortedDeltaTime{}, historyDeltaTimeIndex(0), populated(false)
	{
	}

	DeltaTimeEstimator::~DeltaTimeEstimator()
	{
	}

	float DeltaTimeEstimator::getDeltaTime(const float lastDeltaTime)
	{
		historyDeltatime[historyDeltaTimeIndex] = lastDeltaTime;

		if (historyDeltaTimeIndex == 10)
		{
			populated = true;
		}

		historyDeltaTimeIndex = (historyDeltaTimeIndex + 1) % numRecord;

		if (!populated)
		{
			return lastDeltaTime;
		}

		memcpy(sortedDeltaTime, historyDeltatime, sizeof(float) * numRecord);

		std::sort(sortedDeltaTime, sortedDeltaTime + numRecord);

		float averageDeltaTime = 0.f;

		for (uint32_t i = numDiscard; i < numRecord - numDiscard; i++)
		{
			averageDeltaTime += sortedDeltaTime[i];
		}

		averageDeltaTime /= static_cast<float>(numRecord - 2 * numDiscard);

		const float lerpDeltaTime = averageDeltaTime * (1.f - lerpFactor) + lastDeltaTime * lerpFactor;

		return lerpDeltaTime;
	}
}
