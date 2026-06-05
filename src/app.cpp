#include "app.hpp"

#include <optional>
#include <string>

#include "config/config.hpp"
#include "engine.hpp"
#include "io/content_loader.hpp"
#include "terminal.hpp"
#include "util/ansi.hpp"

namespace vestige {

namespace {

constexpr std::string_view CONFIG_RELATIVE = "/config.json";
constexpr std::string_view DEFAULT_RESOURCE_DIR = "resources";

}

int App::run() {
    Config config = Config::loadOrDefault(std::string{DEFAULT_RESOURCE_DIR} + std::string{CONFIG_RELATIVE});
    const std::string resourceDir = config.resourceDir.empty()
        ? std::string{DEFAULT_RESOURCE_DIR}
        : config.resourceDir;

    const bool ansiActive = config.ansiEnabled && ansi::enableVirtualTerminal();
    Terminal terminal(config, ansiActive);

    ContentLoader loader([&terminal](const std::string& message) {
        terminal.say(Voice::Error, message);
    });

    std::optional<LoadedContent> content = loader.load(resourceDir);
    if (!content) {
        terminal.say(Voice::Error, "Archive content failed to load. Run from the project root so 'resources' is reachable.");
        return 1;
    }

    Engine engine(std::move(config), terminal, std::move(*content));
    engine.run();
    return 0;
}

}
