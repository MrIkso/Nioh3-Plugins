#include "LogUtils.h"
#include "FileUtils.h"
#include "ConfigUtils.h"
#include <cstdarg>
#include <memory>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#pragma warning(disable: 4073)	// yes this is intentional
#pragma init_seg(lib)
namespace {
    void EnsureConsoleOpen() {
        if (GetConsoleWindow() != nullptr) {
            return;
        }

        if (!AllocConsole()) {
            return;
        }

        FILE* fp = nullptr;
        freopen_s(&fp, "CONIN$", "r", stdin);
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleTitleA("LooseFileLoader Console");
    }
}

std::shared_ptr<spdlog::logger> globalLogger = []()->auto {
    auto exeDir = FileUtils::GetExecutableDirectory();
    auto logFilePath = exeDir / "logs" / (FileUtils::GetCurrentModuleName() + ".log");

    bool enableConsole = ConfigUtils::ReadInt(FileUtils::GetCurrentModuleName(), "EnableConsole", 0) != 0;

    std::vector<spdlog::sink_ptr> sinks = {
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true)
    };
    if (enableConsole) {
        EnsureConsoleOpen();
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    }
    auto logger = std::make_shared<spdlog::logger>("multi_logger", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::info);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][%t] %v");
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(logger);
    logger->info("===============================================================");
    return logger;
}();


void _MESSAGE(const char* fmt, ...) {
    if (fmt == nullptr) 
        return;

    va_list args;
    va_start(args, fmt);

    int len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len <= 0) 
        return;
    std::string buffer(len, '\0');
    va_start(args, fmt);
    vsnprintf(buffer.data(), len + 1, fmt, args);
    va_end(args);

    if (globalLogger) {
        globalLogger->info(buffer);
    }
}
