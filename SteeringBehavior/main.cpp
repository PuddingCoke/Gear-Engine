#include<Gear/Gear.h>

#include<iostream>

#include"MyGame.h"

using namespace Gear;

int wmain(int argc, const wchar_t* argv[])
{
	const InitializationParam param = InitializationParam::RealTime(1920, 1080, L"Steering Behavior", true);

#ifdef _DEBUG
	try
	{
#endif // _DEBUG
		Gear::initialize();

		if (!Gear::iniEngine(param, argc, argv))
		{
			Gear::iniGame(new MyGame());
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