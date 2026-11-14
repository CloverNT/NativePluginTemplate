#include <CloverNT/API/Events/DllEvent.hpp>
#include <CloverNT/API/Events/EventBus.hpp>
#include <CloverNT/API/Logger.hpp>
#include <CloverNT/API/Memory/Memory.hpp>
#include <CloverNT/API/Plugin/PluginBase.hpp>
#include <CloverNT/API/Plugin/RegisterHelper.hpp>

using namespace CloverNT;

namespace {

class MyNativePlugin : public Plugin::PluginBase {
public:
    bool load() {
        logger().info("loaded from {}", directory().string());

        if (const auto host = Memory::findModule()) {
            logger().info("host module '{}' @ {:#x}, {} bytes", host->name(), host->base(), host->size());
        }

        auto listener =
                Event::EventBus::getInstance().emplaceListener<Event::DllLoadEvent>([this](Event::DllLoadEvent& event) {
                    if (std::string_view{event.baseName()}.ends_with(".node")) {
                        logger().info("node addon loaded: {} ({} bytes)", event.baseName(), event.imageSize());
                    }
                });
        if (!listener) {
            logger().error("failed to subscribe to DllLoadEvent: {}", listener.error().message);
            return false;
        }
        return true;
    }

    bool unload() {
        logger().info("unloaded");
        return true;
    }
};

} // namespace

CloverNT_REGISTER_PLUGIN(MyNativePlugin)
