#pragma once

#include <functional>
#include <string>

namespace vestige {

using FaultSink = std::function<void(const std::string&)>;

}
