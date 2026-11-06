#include <Gear/Utils/DeltaTimeEstimator.h>

#include <algorithm>

#include <numeric>

namespace Gear::Utils
{
	DeltaTimeEstimator::DeltaTimeEstimator() :
		historyDeltatime{}, sortedDeltaTime{}, historyDeltaTimeIndex(0), frameRate(0.f), populated(false)
	{
	}

	DeltaTimeEstimator::~DeltaTimeEstimator()
	{
	}

	float DeltaTimeEstimator::getDeltaTime(const float lastDeltaTime)
	{
		historyDeltatime[historyDeltaTimeIndex] = lastDeltaTime;

		if (historyDeltaTimeIndex == numRecord - 1u)
		{
			populated = true;
		}

		historyDeltaTimeIndex = (historyDeltaTimeIndex + 1) % numRecord;

		if (!populated)
		{
			const float sumTime =
				std::accumulate(historyDeltatime, historyDeltatime + historyDeltaTimeIndex, 0.f);

			frameRate = historyDeltaTimeIndex / sumTime;

			return lastDeltaTime;
		}

		memcpy(sortedDeltaTime, historyDeltatime, sizeof(float) * numRecord);

		std::sort(sortedDeltaTime, sortedDeltaTime + numRecord);

		const float sumTime =
			std::accumulate(sortedDeltaTime + numDiscard, sortedDeltaTime + numRecord - numDiscard, 0.f);

		const float averageDeltaTime = sumTime / (numRecord - 2 * numDiscard);

		frameRate = (numRecord - 2 * numDiscard) / sumTime;

		const float lerpDeltaTime = averageDeltaTime * (1.f - lerpFactor) + lastDeltaTime * lerpFactor;

		return lerpDeltaTime;
	}

	float DeltaTimeEstimator::getFrameRate() const
	{
		return frameRate;
	}
}
