#pragma once

#ifndef _GEAR_H_
#define _GEAR_H_

#include<Gear/InitializationParam.h>

#include<Gear/Game.h>

namespace Gear
{

	int32_t initEngine(const InitializationParam& param, const int32_t argc, const wchar_t* argv[]);

	void initGame(UniquePtr<Game> gamePtr);

	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

	void failureExit(const std::exception& e);

}

#endif // !_GEAR_H_
