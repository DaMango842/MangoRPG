#include "Utils.h"

#define NOMINMAX              // 避免 windows.h 定义 min/max 宏
#define NOGDI                 // 避免引入 wingdi.h
#define WIN32_LEAN_AND_MEAN   // 精简 windows.h，减少包含内容
#include <Windows.h>
#undef ERROR

#include <cstdarg> // va_list

#include <iostream>
#include <sstream>
#include <set>

std::string toStdString(const sf::String& str)
{
    auto utf8 = str.toUtf8();
    return std::string(utf8.begin(), utf8.end());
}

// 简化语言代码实现
std::string simplifyLangCode(const std::string& langCode) {
    if (langCode.rfind("zh", 0) == 0 && langCode.size() >= 5) {
        return langCode.substr(0, 5);
    }
    return langCode.substr(0, 2);
}

// 获取支持分辨率实现
std::vector<ResolutionOption> getAvailableResolutions(unsigned minWidth, unsigned minHeight) {
    std::vector<ResolutionOption> result;
    std::set<std::string> seen;

    for (const auto& mode : sf::VideoMode::getFullscreenModes()) {
        if (mode.width < minWidth || mode.height < minHeight) continue;

        std::string label = std::to_string(mode.width) + "x" + std::to_string(mode.height);
        if (seen.insert(label).second) {
            result.push_back({ label, { mode.width, mode.height } });
        }
    }
    return result;
}

// 启用 Windows 控制台 ANSI 颜色支持（需 Windows 10 以上）
void enableANSIColor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(CP_UTF8);
}

// 重置终端颜色
void resetColor() {
    std::cout << "\x1b[0m";
}

// 计算带信箱 (letterbox) 视口，保持宽高比
sf::View applyLetterboxView(const sf::View& view, int windowWidth, int windowHeight) {
    float windowRatio = static_cast<float>(windowWidth) / windowHeight;
    float viewRatio = view.getSize().x / view.getSize().y;

    float viewportWidth = 1.f;
    float viewportHeight = 1.f;
    float viewportX = 0.f;
    float viewportY = 0.f;

    if (windowRatio > viewRatio) {
        viewportWidth = viewRatio / windowRatio;
        viewportX = (1.f - viewportWidth) / 2.f;
    }
    else if (windowRatio < viewRatio) {
        viewportHeight = windowRatio / viewRatio;
        viewportY = (1.f - viewportHeight) / 2.f;
    }

    sf::View adjusted = view;
    adjusted.setViewport(sf::FloatRect(viewportX, viewportY, viewportWidth, viewportHeight));
    return adjusted;
}

// 格式化打印 (类似 printf)
void cprintf(const char* format, ...) {
    constexpr size_t bufferSize = 1024;
    char buffer[bufferSize];

    va_list args;
    va_start(args, format);
    vsnprintf_s(buffer, bufferSize, _TRUNCATE, format, args);
    va_end(args);

    std::cout << buffer;
}

// 宽字符版本格式化打印
void wcprintf(const wchar_t* format, ...) {
    constexpr size_t bufferSize = 1024;
    wchar_t buffer[bufferSize];

    va_list args;
    va_start(args, format);
    _vsnwprintf_s(buffer, bufferSize, _TRUNCATE, format, args);
    va_end(args);

    std::wcout << buffer;
}
