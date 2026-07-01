#pragma once

#ifndef _GEAR_CORE_GLOBALSHADER_INTERNAL_H_
#define _GEAR_CORE_GLOBALSHADER_INTERNAL_H_

namespace Gear::Core::GlobalShader::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };
}

#endif // !_GEAR_CORE_GLOBALSHADER_INTERNAL_H_
