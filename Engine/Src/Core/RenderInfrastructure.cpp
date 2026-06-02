#include<Gear/Core/GraphicsDevice.h>

#include<Gear/Core/Internal/RenderInfrastructure.h>

namespace
{
	class RenderInfrastructureImpl
	{
	public:



	private:



	};

	UniquePtr<RenderInfrastructureImpl> impl;
}

void Gear::Core::RenderInfrastructure::Internal::initialize()
{

}

void Gear::Core::RenderInfrastructure::Internal::release()
{
	impl.reset();
}

