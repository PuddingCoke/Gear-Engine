#include<Gear/Gear.h>

#include<iostream>

#include"MyGame.h"

using namespace Gear;

int wmain(int argc, const wchar_t* argv[])
{
	const InitializationParam param = InitializationParam::RealTime(1920, 1080, L"Steering Behavior", true);

	try
	{
		Gear::InitializeToken token;

		if (!Gear::iniEngine(param, argc, argv))
		{
			Gear::iniGame(makeUnique<MyGame>());
		}
	}
	catch (const std::exception& e)
	{
		Gear::failureExit(e);
	}
}