#include<Gear/Gear.h>

#include"MyGame.h"

using namespace Gear;

int wmain(int argc, const wchar_t* argv[])
{
	const InitializationParam param = InitializationParam::VideoRender(1920, 1080);

#ifdef _DEBUG
	try
	{
#endif // _DEBUG
		Gear::initialize();

		if (!Gear::iniEngine(param, argc, argv))
		{
			Gear::iniGame(makeUnique<MyGame>());
		}

		Gear::release();
#ifdef _DEBUG
	}
	catch (const std::exception&)
	{
		Gear::failureExit();
	}
#endif // _DEBUG
}