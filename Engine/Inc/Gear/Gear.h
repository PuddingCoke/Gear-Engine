#pragma once

#ifndef _GEAR_H_
#define _GEAR_H_

#include <Gear/InitializationParam.h>

#include <Gear/Game.h>

namespace Gear
{

	void initGame(UniquePtr<Game> gamePtr);

	void initialize(const InitializationParam& param, const int32_t argc, const wchar_t* argv[]);

	void release();

	struct EngineInitializeToken
	{
		EngineInitializeToken(const InitializationParam& param, const int32_t argc, const wchar_t* argv[])
		{
			initialize(param, argc, argv);
		}
		~EngineInitializeToken()
		{
			release();
		}
	};

	void failureExit(const std::exception& e);

}

#endif // !_GEAR_H_
