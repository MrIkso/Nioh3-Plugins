#include "ConfigUtils.h"

#include <windows.h>

#include "FileUtils.h"

namespace {
    std::filesystem::path GetDefaultIniPath() {
        const std::filesystem::path modulePath = FileUtils::GetCurrentModulePath();
        if (modulePath.empty() || !modulePath.has_stem()) {
            return {};
        }
        return modulePath.parent_path() / (modulePath.stem().string() + ".ini");
    }
}

namespace ConfigUtils {
    std::int32_t ReadInt(std::string_view section, std::string_view key, std::int32_t defaultValue) {
        static const std::filesystem::path iniPath = GetDefaultIniPath();
        if (iniPath.empty() || section.empty() || key.empty()) {
            return defaultValue;
        }
        return static_cast<int>(GetPrivateProfileIntA(
            section.data(),
            key.data(),
            defaultValue,
            iniPath.string().c_str()));
    }

    std::string ReadString(std::string_view section, std::string_view key, std::string_view defaultValue)
    {
        static const std::filesystem::path iniPath = GetDefaultIniPath();
        if (iniPath.empty() || section.empty() || key.empty()) {
            return defaultValue.data();
        }

        char buffer[1024] = { 0 };
        GetPrivateProfileStringA(
            section.data(),
            key.data(),
            defaultValue.data(),
            buffer,
            static_cast<DWORD>(sizeof(buffer)),
            iniPath.string().c_str());
        return std::string(buffer);
    }
}
