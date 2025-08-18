#include "Translator.h"
#include <fstream>
#include <regex>
#include <Utils.h> // 你的工具函数，比如 toStdString

using json = nlohmann::json;

Translator& Translator::get() {
    static Translator instance;
    return instance;
}

bool Translator::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    try {
        file >> j;
    }
    catch (...) {
        return false;
    }

    m_jsonRoot = j;  // 保存原始 JSON

    // 兼容旧扁平结构，递归解析字符串叶子节点
    m_translations.clear();
    std::function<void(const std::string&, const json&)> parse;
    parse = [&](const std::string& prefix, const json& node) {
        for (auto& [key, val] : node.items()) {
            std::string fullKey = prefix.empty() ? key : (prefix + "." + key);
            if (val.is_string()) {
                // 直接用 UTF-8 转换 sf::String
                std::string raw = val.get<std::string>();
                m_translations[fullKey] = sf::String::fromUtf8(raw.begin(), raw.end());
            }
            else if (val.is_object()) {
                parse(fullKey, val);
            }
            else if (val.is_array()) {
                // 数组中每个元素递归处理，键用索引表示
                for (size_t i = 0; i < val.size(); ++i) {
                    parse(fullKey + "[" + std::to_string(i) + "]", val[i]);
                }
            }
        }
        };
    parse("", j);

    return true;
}

const nlohmann::json* Translator::getJsonValueByPath(const std::string& path) const {
    const nlohmann::json* current = &m_jsonRoot;
    std::regex re(R"(([^.\[\]]+)(?:\[(\d+)\])?)");
    auto begin = std::sregex_iterator(path.begin(), path.end(), re);
    auto end = std::sregex_iterator();

    for (auto& i = begin; i != end; ++i) {
        std::smatch match = *i;
        std::string key = match[1].str();
        int idx = match[2].matched ? std::stoi(match[2].str()) : -1;

        if (!current->is_object() || !current->contains(key))
            return nullptr;
        current = &(*current)[key];

        if (idx >= 0) {
            if (!current->is_array() || idx >= current->size())
                return nullptr;
            current = &(*current)[idx];
        }
    }
    return current;
}

sf::String Translator::tr(const std::string& key) const {
    // 优先从 JSON 原始数据查找（支持数组路径）
    const json* val = getJsonValueByPath(key);
    if (val && val->is_string()) {
        const std::string& s = val->get_ref<const std::string&>();
        return sf::String::fromUtf8(s.begin(), s.end());
    }

    // 旧版扁平映射兼容
    auto it = m_translations.find(key);
    if (it != m_translations.end()) {
        return it->second;
    }

    // 找不到时返回 [[key]]
    return sf::String("[[") + key + "]]";
}

sf::String Translator::tr(const std::string& key, const std::unordered_map<std::string, std::string>& variables) const {
    sf::String str = tr(key);
    std::string raw = toStdString(str);  // 你需实现把 sf::String 转 std::string 的工具

    for (const auto& [var, val] : variables) {
        std::string placeholder = "{" + var + "}";
        size_t pos = 0;
        while ((pos = raw.find(placeholder, pos)) != std::string::npos) {
            raw.replace(pos, placeholder.size(), val);
            pos += val.length();
        }
    }

    return sf::String::fromUtf8(raw.begin(), raw.end());
}

void Translator::setLanguage(const std::string& langCode) {
    m_languageCode = simplifyLangCode(langCode);
    std::string path = "Assets/Locales/" + m_languageCode + ".json";

    if (!loadFromFile(path)) {
        std::cerr << "[Translator] Failed to load language file: " << path << std::endl;
    }

    LanguageObserver::get().notify();
}

const std::string& Translator::getCurrentLanguage() const {
    return m_languageCode;
}
