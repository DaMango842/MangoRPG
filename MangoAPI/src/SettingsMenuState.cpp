#include "SettingsMenuState.h"
#include "ResourceLoader.h"
#include "Translator.h"
#include "Logger.h"
#include "Window.h"
#include "Utils.h" // 包含 getAvailableResolutions

SettingsMenuState::SettingsMenuState(SafeRef<StateManager> stateManager)
    : m_stateManager(stateManager),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
{
    m_settings = Settings::load(m_settingsPath);
    m_selectedLanguageIndex = m_settings.languageIndex;
    m_selectedWindowIndex = m_settings.windowModeIndex;

    LOG_DEBUG("SettingsMenuState constructed at: " + std::to_string(reinterpret_cast<uintptr_t>(this)));
}

template<typename T, typename... Args>
T* SettingsMenuState::addComponent(Args&&... args) {
    auto comp = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = comp.get();
    m_components.push_back(std::move(comp));
    return ptr;
}

void SettingsMenuState::clearUI() {
    for (auto& comp : m_components) {
        comp->setEnabled(false);
        comp->setVisible(false);
    }
    m_components.clear();
    m_windowBox = nullptr;
    m_languageBox = nullptr;
    m_resolutionDropdown = nullptr;
}

void SettingsMenuState::setupUI() {
    m_title.setFont(m_font);
    m_title.setString(TR("SETTINGS.TITLE"));
    m_title.setCharacterSize(36);
    m_title.setFillColor(sf::Color::White);
    m_title.setPosition(20.f, 20.f);

    float startY = 100.f;
    float offsetY = 60.f;

    SafePtr<SettingsMenuState> safeThis(this);

    auto windowLabel = addComponent<Label>(TR("SETTINGS.WINDOW.LABEL"), m_font);
    windowLabel->setPosition({ 100.f, startY - 40.f });

    m_windowBox = addComponent<RadioBox>();
    m_windowBox->addOption(TR("SETTINGS.WINDOW.VALUE[0]"), m_font, [safeThis]() {
        if (Window::getInstance().isFullscreen()) {
            Window::getInstance().toggleFullscreen();
            safeThis->m_settings.windowModeIndex = 0;
            safeThis->m_settings.save(safeThis->m_settingsPath);
        }
        });
    m_windowBox->addOption(TR("SETTINGS.WINDOW.VALUE[1]"), m_font, [safeThis]() {
        if (!Window::getInstance().isFullscreen()) {
            Window::getInstance().toggleFullscreen();
            safeThis->m_settings.windowModeIndex = 1;
            safeThis->m_settings.save(safeThis->m_settingsPath);
        }
        });
    m_windowBox->setPosition({ 100.f, startY });
    m_windowBox->selectOption(m_selectedWindowIndex);

    auto resolutions = getAvailableResolutions();
    auto currentRes = Window::getInstance().getCurrentResolution();

    m_resolutionDropdown = addComponent<DropDown>(m_font);
    for (const auto& res : resolutions)
        m_resolutionDropdown->addOption(res.label);
    m_resolutionDropdown->setCallback([=](int index, const std::string&) {
        const auto& r = resolutions[index];
        Window::getInstance().setResolution(sf::VideoMode(r.size.x, r.size.y));
        m_settings.resolutionIndex = index;
        m_settings.save(m_settingsPath);
        });
    m_resolutionDropdown->setPosition({ 360.f, startY });

    if (m_settings.resolutionIndex >= 0 && m_settings.resolutionIndex < resolutions.size())
        m_resolutionDropdown->setSelected(m_settings.resolutionIndex);
    else {
        for (size_t i = 0; i < resolutions.size(); ++i) {
            if (resolutions[i].size.x == currentRes.width && resolutions[i].size.y == currentRes.height) {
                m_resolutionDropdown->setSelected(static_cast<int>(i));
                break;
            }
        }
    }

    auto languageLabel = addComponent<Label>(TR("SETTINGS.LANGUAGE.LABEL"), m_font);
    languageLabel->setPosition({ 100.f, startY + offsetY * 2 - 40.f });

    m_languageBox = addComponent<RadioBox>();
    m_languageBox->addOption(TR("SETTINGS.LANGUAGE.VALUE[0]"), m_font, [safeThis]() {
        if (safeThis) {
            Translator::get().setLanguage("zh_CN");
            safeThis->m_settings.languageIndex = 0;
            safeThis->m_settings.save(safeThis->m_settingsPath);
        }
        });
    m_languageBox->addOption(TR("SETTINGS.LANGUAGE.VALUE[1]"), m_font, [safeThis]() {
        if (safeThis) {
            Translator::get().setLanguage("en");
            safeThis->m_settings.languageIndex = 1;
            safeThis->m_settings.save(safeThis->m_settingsPath);
        }
        });
    m_languageBox->setPosition({ 100.f, startY + offsetY * 2 });
    m_languageBox->selectOption(m_selectedLanguageIndex);

    sf::String volumeLabels[] = {
        TR("SETTINGS.VOLUME.VALUE[0]"), // 主音量
        TR("SETTINGS.VOLUME.VALUE[1]"), // 音乐音量
        TR("SETTINGS.VOLUME.VALUE[2]")  // 音效音量
    };

    auto masterVol = addComponent<Slider>(0.f, 100.f, 1.f, m_font);
    masterVol->setLabel(volumeLabels[0]);
    masterVol->setValue(m_settings.volumeMaster);
    masterVol->setPosition({ 100.f, startY + offsetY * 3 + 30.f });
    masterVol->setOnEventChanged([safeThis, masterVol]() {
        if (safeThis) {
            safeThis->m_settings.volumeMaster = masterVol->getValue();
            safeThis->m_settings.save(safeThis->m_settingsPath);
        }
        });

    auto musicVol = addComponent<Slider>(0.f, 100.f, 1.f, m_font);
    musicVol->setLabel(volumeLabels[1]);
    musicVol->setValue(m_settings.volumeMusic);
    musicVol->setPosition({ 100.f, startY + offsetY * 4 + 30.f });
    musicVol->setOnEventChanged([safeThis, musicVol]() {
        safeThis->m_settings.volumeMusic = musicVol->getValue();
        safeThis->m_settings.save(safeThis->m_settingsPath);
        });

    auto sfxVol = addComponent<Slider>(0.f, 100.f, 1.f, m_font);
    sfxVol->setLabel(volumeLabels[2]);
    sfxVol->setValue(m_settings.volumeSFX);
    sfxVol->setPosition({ 100.f, startY + offsetY * 5 + 30.f });
    sfxVol->setOnEventChanged([safeThis, sfxVol]() {
        safeThis->m_settings.volumeSFX = sfxVol->getValue();
        safeThis->m_settings.save(safeThis->m_settingsPath);
        });

    auto returnBtn = addComponent<Button>(TR("BACKBTN"), m_font);
    returnBtn->setPosition({ 100.f, startY + offsetY * 6 + 40.f });
    returnBtn->setCallback([this]() {
        m_stateManager->safePopState();
        });
}

void SettingsMenuState::refreshUI() {
    LOG_DEBUG("refreshUI requested");
    m_needsRefresh = true;
}

void SettingsMenuState::handleEvent(const sf::Event& event) {
    for (auto& comp : m_components)
        if (comp->isVisible() && comp->isEnabled())
            comp->handleEvent(event);
}

void SettingsMenuState::update(float deltaTime) {
    if (m_needsRefresh) {
        LOG_DEBUG("Executing deferred refreshUI");

        if (m_windowBox && m_windowBox->getSelectedIndex())
            m_selectedWindowIndex = static_cast<int>(*m_windowBox->getSelectedIndex());
        if (m_languageBox && m_languageBox->getSelectedIndex())
            m_selectedLanguageIndex = static_cast<int>(*m_languageBox->getSelectedIndex());

        clearUI();
        setupUI();
        m_needsRefresh = false;
        return;
    }

    for (auto& comp : m_components)
        if (comp->isVisible() && comp->isEnabled())
            comp->update(deltaTime);
}

void SettingsMenuState::render(sf::RenderTarget& target) {
    target.draw(m_title);
    for (auto& comp : m_components)
        if (comp->isVisible())
            comp->render(target);
}

void SettingsMenuState::onEnter() {
    setupUI();

    SafePtr<SettingsMenuState> self(this);
    SafeCallback<> cb(self, &SettingsMenuState::refreshUI);
    m_langObserverId = LanguageObserver::get().subscribe([cb]() mutable {
        if (cb) cb();
        });


    if (self->m_selectedLanguageIndex) {
        Translator::get().setLanguage(availableLanguage[self->m_selectedLanguageIndex]);
    }
}

void SettingsMenuState::onExit() {
    LanguageObserver::get().unsubscribe(m_langObserverId);
}
