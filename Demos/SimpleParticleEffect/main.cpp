#include<Gear/Gear.h>

#include"MyGame.h"

int wmain(int argc, const wchar_t* argv[])
{
	const InitializationParam param = InitializationParam::RealTime(1920, 1080, L"Simple Particle Effect", true);

	try
	{
		Gear::InitializeToken token;

		if (!Gear::initEngine(param, argc, argv))
		{
			Gear::initGame(makeUnique<MyGame>());
		}
	}
	catch (const std::exception& e)
	{
		Gear::failureExit(e);
	}
}