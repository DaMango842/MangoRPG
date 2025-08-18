#pragma once
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <locale>
#include "Utils.h"
#include "LanguageObserver.hpp"

#define TR(key) Translator::get().tr(key)
#define TRV(key, vars) Translator::get().tr(key, vars)

inline const char* availableLanguage[] = {
    "zh_CN",
    "en"
};

class Translator {
public:
    static Translator& get();

    bool loadFromFile(const std::string& filename);

    sf::String tr(const std::string& key) const;
    sf::String tr(const std::string& key, const std::unordered_map<std::string, std::string>& variables) const;

    void setLanguage(const std::string& langCode);
    const std::string& getCurrentLanguage() const;

private:
    Translator() = default;

    // 兼容旧版扁平翻译缓存（可选）
    std::unordered_map<std::string, sf::String> m_translations;

    // 原始 JSON，用于支持数组和层级访问
    nlohmann::json m_jsonRoot;

    std::string m_languageCode;

    // 通过路径访问 JSON，支持形如 "SETTINGS.WINDOW.VALUE[0]"
    const nlohmann::json* getJsonValueByPath(const std::string& path) const;
};
