#define NOMINMAX
#include <Windows.h>

#include <PluginAPI.h>
#include <BranchTrampoline.h>
#include <LogUtils.h>

#include "Common.h"
#include "ModHooks.h"
#include "ModAssetManager.h"
#include <sstream> 
#include <vector>
#include <string>
#include <FileUtils.h>

namespace {
    std::filesystem::path GetIniPath(const Nioh3PluginInitializeParam* param) {
        std::filesystem::path pluginsDir = (param && param->plugins_dir) ? param->plugins_dir : "";
        std::string moduleName = PLUGIN_NAME;
        return pluginsDir / (moduleName + ".ini");
    }

    bool ReadIniBool(const std::filesystem::path& iniPath, const char* section, const char* key, bool defaultValue) {
        if (!std::filesystem::exists(iniPath)) {
            return defaultValue;
        }
        int value = GetPrivateProfileIntA(section, key, defaultValue ? 1 : 0, iniPath.string().c_str());
        return value != 0;
    }

    std::string ReadIniString(const std::filesystem::path& iniPath, const char* section, const char* key, const char* defaultValue) {
        char buffer[1024];
        GetPrivateProfileStringA(section, key, defaultValue, buffer, sizeof(buffer), iniPath.string().c_str());
        return std::string(buffer);
    }

}


using namespace LooseFileLoader;
// cmake --build build --config Release --target LooseFileLoader

std::atomic<bool> g_isInitialized{ false };

void DoInitialize(std::filesystem::path pluginsDir, std::filesystem::path gameRootDir) {
    if (g_isInitialized.exchange(true))
        return;

    auto iniPath = pluginsDir / (std::string(PLUGIN_NAME) + ".ini");

    g_enableConsole = ReadIniBool(iniPath, PLUGIN_NAME, "EnableConsole", false);
    InitLogger(g_enableConsole);
    _MESSAGE("Initializing plugin: %s, version: %d.%d.%d",
        PLUGIN_NAME, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH);
    _MESSAGE("Plugins Dir: %s", pluginsDir.string().c_str());
    _MESSAGE("Game Root: %s", gameRootDir.string().c_str());

    g_enableAssetLoadingLog = ReadIniBool(iniPath, PLUGIN_NAME, "EnableAssetLoadingLog", false);
    _MESSAGE("EnableAssetLoadingLog: %d", g_enableAssetLoadingLog ? 1 : 0);
    g_disableStreamingLoading = ReadIniBool(iniPath, PLUGIN_NAME, "DisableStreamingLoading", false);
    _MESSAGE("DisableStreamingLoading: %d", g_disableStreamingLoading ? 1 : 0);

    g_enableLoggerFilters = ReadIniBool(iniPath, PLUGIN_NAME, "EnableLoggerFilters", false);
    if (g_enableLoggerFilters) {
        std::string filtersStr = ReadIniString(iniPath, PLUGIN_NAME, "Filters", "");
        std::stringstream ss(filtersStr);
        std::string item;
        while (std::getline(ss, item, ',')) {
            item.erase(0, item.find_first_not_of(" \t\r\n"));
            auto last = item.find_last_not_of(" \t\r\n");
            if (last != std::string::npos) item.erase(last + 1);
            if (!item.empty()) g_logFilters.insert(item);
        }
        _MESSAGE("Filters loaded: %zu items", g_logFilters.size());
    }

    g_modAssetManager.Build(gameRootDir);
    if (!InstallHooks()) {
        _MESSAGE("Failed to install LooseFileLoader hooks");
    }
    else {
        _MESSAGE("All hooks installed successfully.");
    }
}


extern "C" __declspec(dllexport) bool nioh3_plugin_initialize(const Nioh3PluginInitializeParam* param) {
    if (param) {
        DoInitialize(param->plugins_dir, param->game_root_dir);
    }
    return true;
}

// init plugin as asi
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        CreateThread(nullptr, 0, [](LPVOID hMod) -> DWORD {
            auto exeDir = FileUtils::GetExecutableDirectory();

            auto pluginsDir = exeDir / "plugins";
            if (!std::filesystem::exists(pluginsDir)) {
                pluginsDir = exeDir;
            }

            DoInitialize(pluginsDir, exeDir);
            return 0;
            }, hModule, 0, nullptr);
    }
    return TRUE;
}

