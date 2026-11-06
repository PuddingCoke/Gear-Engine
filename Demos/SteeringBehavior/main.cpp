#include <Gear/Gear.h>

#include "MyGame.h"

int wmain(int argc, const wchar_t* argv[])
{
	//InitializationParam param = InitializationParam::RealTime(1920, 1080, L"Steering Behavior", true);

	InitializationParam param = InitializationParam::VideoRender(1920, 1080, 10, false);

	try
	{
		Gear::EngineInitializeToken token(param, argc, argv);

		Gear::initGame(makeUnique<MyGame>());
	}
	catch (const std::exception& e)
	{
		Gear::failureExit(e);
	}
}
