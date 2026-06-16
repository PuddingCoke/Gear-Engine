#include<Gear/Gear.h>

#include"MyGame.h"

#include<iostream>

using namespace Gear;

int wmain(int argc, const wchar_t* argv[])
{
	const InitializationParam param = InitializationParam::RealTime(1920, 1080, L"Black Hole", true);

	//const InitializationParam param = InitializationParam::VideoRender(2560, 1440, 360);

#ifdef _DEBUG
	try
	{
#endif // _DEBUG
		Gear::InitializeToken token;

		if (!Gear::iniEngine(param, argc, argv))
		{
			Gear::iniGame(makeUnique<MyGame>());
		}
#ifdef _DEBUG
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";

		Gear::failureExit();
	}
#endif // _DEBUG
}