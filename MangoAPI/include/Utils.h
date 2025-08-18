#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <algorithm> // for std::clamp

//#include "Logger.h"

// ---------- 模板函数写这里 ----------

// 通用打印函数，支持任意参数
template<typename... Args>
inline void print(const Args&... args) {
    std::ostringstream oss;
    (oss << ... << args);
    std::cout << oss.str();
}

// 打印并换行
template<typename... Args>
inline void println(const Args&... args) {
    print(args...);
    std::cout << std::endl;
}

// 安全的 clamp，自动检测并记录异常
template<typename T>
inline T safeClamp(T value, T minVal, T maxVal, const char* context = nullptr) {
    try {
        if (minVal > maxVal) {
            //Logger::writeLog(LogLevel::WARNING, std::string("Invalid clamp range: min > max") +
            //    (context ? std::string(" (") + context + ")" : ""));
            return value; // 或者 return minVal; 依据你的容忍策略
        }
        return std::clamp(value, minVal, maxVal);
    }
    catch (const std::exception& e) {
        //Logger::writeLog(LogLevel::ERROR, std::string("Exception in safeClamp") +
        //    (context ? std::string(" (") + context + ")" : "") +
        //    ": " + e.what());
        return value; // fallback
    }
    catch (...) {
        //Logger::writeLog(LogLevel::ERROR, std::string("Unknown exception in safeClamp") +
        //    (context ? std::string(" (") + context + ")" : ""));
        return value; // fallback
    }
}

// sf::String 转 std::string UTF8
std::string toStdString(const sf::String& str);

// 简化语言代码，如 zh_CN -> zh_CN，en_US -> en
std::string simplifyLangCode(const std::string& langCode);

struct ResolutionOption {
    std::string label;
    sf::Vector2u size;
};

// 获取系统支持的分辨率，默认过滤小于 800x600 的
std::vector<ResolutionOption> getAvailableResolutions(unsigned minWidth = 800, unsigned minHeight = 600);

// 其他非模板声明，写在 cpp 里
void enableANSIColor();
void resetColor();
sf::View applyLetterboxView(const sf::View& view, int windowWidth, int windowHeight);
void cprintf(const char* format, ...);
void wcprintf(const wchar_t* format, ...);
