#pragma once
#include <string>
#include <SFML/System.hpp>
#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;
using Timestamp = uint64_t;

struct SlotInfo {
    int slotIndex = -1;
    bool valid = false;

    std::string saveName;
    std::string playerName;
    Timestamp timestamp = 0;
    sf::Vector2f position{};
    json attributesPreview;
    std::optional<int> playTimeSeconds;
    std::string playerImgPath;
    int chapter = 0;
    std::string screenshotPath;
    bool isAutosave = false;
    std::string description;
    int difficultyLevel = 0;
    bool isFavorite = false;
    int version = 1;

    std::string filePath;  // ✅用于 getAllSlotInfos 返回结果展示
};

inline std::wstring formatTimestamp(Timestamp timestamp) {
    try {
        std::time_t time = static_cast<std::time_t>(timestamp / 1000);
        std::tm tm;

#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        tm = *std::localtime(&time);
#endif

        std::wstringstream wss;
        wss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
        return wss.str();
    }
    catch (...) {
        return L"无效时间";
    }
}
