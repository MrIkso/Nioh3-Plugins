#pragma once

#include <cstdint>
#include <string>

namespace ConfigUtils {
    std::int32_t ReadInt(std::string_view section, std::string_view key, std::int32_t defaultValue = 0);
    std::string ReadString(std::string_view section, std::string_view key, std::string_view defaultValue = "");
}
