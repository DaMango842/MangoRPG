#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "SafeCallback.hpp"
#include "LanguageObserver.hpp"
#include "Button.h"
#include "RadioBox.h"
#include "Slider.h"
#include "Label.h"
#include "DropDown.h"

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>
#include <vector>
#include <unordered_map>

struct Settings {
    int windowModeIndex = 0;
    int resolutionIndex = 0;
    int languageIndex = 0;
    std::string languageStr = "zh_CN";
    float volumeMaster = 100.f;  // 主音量
    float volumeMusic = 100.f;   // 音乐音量
    float volumeSFX = 100.f;     // 音效音量
    

    static Settings load(const std::string& path) {
        Settings s;
        std::ifstream in(path);
        if (!in.is_open()) return s;
        try {
            nlohmann::json j;
            in >> j;
            s.windowModeIndex = j.value("window_mode", 0);
            s.resolutionIndex = j.value("resolution", 0);
            s.languageIndex = j.value("langIndex", 0);

            //s.languageStr = j["language"];

            if (j.contains("volume") && j["volume"].is_array()) {
                auto& volArray = j["volume"];
                s.volumeMaster = volArray.size() > 0 ? volArray[0].get<float>() : 100.f;
                s.volumeMusic = volArray.size() > 1 ? volArray[1].get<float>() : 100.f;
                s.volumeSFX = volArray.size() > 2 ? volArray[2].get<float>() : 100.f;
            }
            else {
                s.volumeMaster = 100.f;
                s.volumeMusic = 100.f;
                s.volumeSFX = 100.f;
            }
        }
        catch (...) {}
        return s;
    }

    void save(const std::string& path) const {
        nlohmann::json j;
       
        j["window_mode"] = windowModeIndex;
        j["resolution"] = resolutionIndex;
        j["langIndex"] = languageIndex;

        //j["language"] = availableLanguage[languageIndex];
        j["volume"] = { volumeMaster, volumeMusic, volumeSFX };

        std::ofstream out(path);
        if (out.is_open()) out << j.dump(4);
        LOG_DEBUG("Saving settings to: " + path);
    }

};

class SettingsMenuState : public BaseState {
public:
    explicit SettingsMenuState(SafeRef<StateManager> stateManager);
    SettingsMenuState(const SettingsMenuState&) = delete;
    SettingsMenuState& operator=(const SettingsMenuState&) = delete;

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

private:
    void setupUI();
    void clearUI();
    void refreshUI();

    template<typename T, typename... Args>
    T* addComponent(Args&&... args);

private:
    SafeRef<StateManager> m_stateManager;
    sf::Font& m_font;
    sf::Text m_title;

    std::vector<std::unique_ptr<BaseComponent>> m_components;

    RadioBox* m_windowBox = nullptr;
    RadioBox* m_languageBox = nullptr;
    DropDown* m_resolutionDropdown = nullptr;

    int m_selectedWindowIndex = 0;
    int m_selectedLanguageIndex = 0;

    bool m_needsRefresh = false;
    CallbackID m_langObserverId = 0;

    Settings m_settings;
    std::string m_settingsPath = "saves/settings.json";
};
