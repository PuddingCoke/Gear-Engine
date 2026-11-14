#ifndef _GEAR_CORE_COMMANDPOOL_H_
#define _GEAR_CORE_COMMANDPOOL_H_

#include "CommandPoolRegistry.h"

#include <Gear/Utils/Memory.h>

#include <vector>

#include <cstdint>

namespace Gear::Core
{
	template<typename Derived>
	class CommandPool
	{
	private:

		struct Storage
		{
			std::vector<UniquePtr<Derived>> items;

			uint64_t writeLocation = 0;
		};

		struct Registrar
		{
			Registrar()
			{
				CommandPoolRegistry::add(&CommandPool::resetPool);
			}
		};

		static Storage& storage()
		{
			static thread_local Registrar registrar;

			static thread_local Storage s;

			return s;
		}

	public:

		static Derived* acquire()
		{
			Storage& s = storage();

			if (s.writeLocation == s.items.size())
			{
				s.items.push_back(makeUnique<Derived>());
			}

			return s.items[s.writeLocation++].get();
		}

		static void resetPool()
		{
			storage().writeLocation = 0;
		}
	};
}

#endif // !_GEAR_CORE_COMMANDPOOL_H_
