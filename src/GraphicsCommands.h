#pragma once

#include "GraphicsDevice.h"

#include <functional>

namespace rkrai {
class GraphicsCommands {
    public:
    static void submitSingleTimeCommand(GraphicsDevice& device, std::function<void(vk::CommandBuffer)> command);

    private:
};
}