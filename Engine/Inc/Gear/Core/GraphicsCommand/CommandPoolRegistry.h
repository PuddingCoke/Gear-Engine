#ifndef _GEAR_CORE_COMMANDPOOLREGISTRY_H_
#define _GEAR_CORE_COMMANDPOOLREGISTRY_H_

#include <vector>

namespace Gear::Core
{
	class CommandPoolRegistry
	{
	public:

		using ResetFn = void (*)();

		static void add(ResetFn fn)
		{
			resetters().push_back(fn);
		}

		static void resetAll()
		{
			for (ResetFn fn : resetters())
			{
				fn();
			}
		}

	private:

		static std::vector<ResetFn>& resetters()
		{
			static thread_local std::vector<ResetFn> result;

			return result;
		}
	};
}

#endif // !_GEAR_CORE_COMMANDPOOLREGISTRY_H_
