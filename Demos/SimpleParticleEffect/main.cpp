#include<Gear/Gear.h>

#include<iostream>

#include"MyGame.h"

using namespace Gear;

int wmain(int argc, const wchar_t* argv[])
{
	const InitializationParam param = InitializationParam::RealTime(1920, 1080, L"Simple Particle Effect", true);

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