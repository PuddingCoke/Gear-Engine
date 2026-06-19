#pragma once

#ifndef _GEAR_CORE_RENDERTHREADLOCAL_H_
#define _GEAR_CORE_RENDERTHREADLOCAL_H_

#include<Gear/Resource/ResourceBase.h>

namespace Gear::Core::RenderThreadLocal
{
	void pushToCopiedResources(Resource::ResourceBase* const resource);
}

#endif // !_GEAR_CORE_RENDERTHREADLOCAL_H_
