#include "LogUtils.h"
#include "FileUtils.h"
#include <cstdarg>
#include <memory>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <vector>

#pragma warning(disable: 4073)	// yes this is intentional
#pragma init_seg(lib)

std::shared_ptr<spdlog::logger> globalLogger = nullptr;

void InitLogger(bool enableConsole) {
    if (globalLogger) 
        return;

    std::vector<spdlog::sink_ptr> sinks;

    auto exeDir = FileUtils::GetExecutableDirectory();
    auto logFilePath = exeDir / "logs" / (FileUtils::GetCurrentModuleName() + ".log");
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true));

    if (enableConsole) {
        if (AllocConsole()) {
            FILE* fDummy;
            freopen_s(&fDummy, "CONOUT$", "w", stdout);
            freopen_s(&fDummy, "CONOUT$", "w", stderr);
            freopen_s(&fDummy, "CONIN$", "r", stdin);
            std::clog.clear();
            std::cout.clear();
        }

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(console_sink);
    }

    globalLogger = std::make_shared<spdlog::logger>("multi_logger", sinks.begin(), sinks.end());

    globalLogger->set_level(spdlog::level::info);
    globalLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][%t] %v");
    globalLogger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(globalLogger);
    globalLogger->info("Logger initialized (Console: {})", enableConsole ? "YES" : "NO");
}


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
