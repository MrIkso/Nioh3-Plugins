#pragma once
#include <string_view>
// Logging related
void InitLogger(bool enableConsole);

void _MESSAGE(const char* fmt, ...);
