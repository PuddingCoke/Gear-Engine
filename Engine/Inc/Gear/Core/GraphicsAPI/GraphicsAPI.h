#ifndef _GEAR_CORE_GRAPHICSAPI_H_
#define _GEAR_CORE_GRAPHICSAPI_H_

#include"GraphicsAPIExecuter.h"

namespace Gear::Core
{
    class GraphicsAPI
    {
        public:

            //准备
            virtual void prepare() = 0;

            //执行
            virtual void execute(GraphicsAPIExecuter& executer) = 0;

            //清理（用于后续循环利用）
            virtual void cleanup() = 0;

    };
}

#endif // !_GEAR_CORE_GRAPHICSAPI_H_
