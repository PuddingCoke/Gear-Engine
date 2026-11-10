#ifndef _GEAR_CORE_GRAPHICSCOMMAND_H_
#define _GEAR_CORE_GRAPHICSCOMMAND_H_

#include"GraphicsCommandExecuter.h"

namespace Gear::Core
{
    class GraphicsCommand
    {
        public:

            virtual ~GraphicsCommand() = default;

            virtual void execute(GraphicsCommandExecuter& executer) = 0;

    };
}

#endif // !_GEAR_CORE_GRAPHICSCOMMAND_H_
