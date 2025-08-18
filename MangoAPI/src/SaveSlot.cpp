#include "SaveSlot.h"
#include <sstream>
#include <iomanip>

SaveSlot::SaveSlot() {
    m_background.setFillColor(sf::Color(50, 50, 50));
    m_background.setOutlineColor(sf::Color::White);
    m_background.setOutlineThickness(2.f);
}

SaveSlot::~SaveSlot() {}

void SaveSlot::handleEvent(const sf::Event& event) {
    if (!isEnabled()) return;

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        m_isHovered = containsPoint(mousePos);
        updateVisual();
    }
    else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f clickPos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            if (containsPoint(clickPos) && !m_locked && m_callback) {
                m_callback();
            }
        }
    }
}

void SaveSlot::update(float /*deltaTime*/) {
    // 可扩展
}

void SaveSlot::render(sf::RenderTarget& target) {
    if (!isVisible()) return;

    target.draw(m_background);
    if (m_font && m_occupied) {
        target.draw(m_nameText);
        target.draw(m_levelText);
        target.draw(m_timeText);
    }
}

void SaveSlot::setPosition(const sf::Vector2f& pos) {
    m_background.setPosition(pos);
    m_nameText.setPosition(pos + sf::Vector2f(10, 5));
    m_levelText.setPosition(pos + sf::Vector2f(10, 30));
    m_timeText.setPosition(pos + sf::Vector2f(10, 55));
}

void SaveSlot::setSize(const sf::Vector2f& size) {
    m_background.setSize(size);
}

void SaveSlot::setData(const BaseSaveSlotData& data) {
    m_data = data;
    setOccupied(true);
    updateVisual();
}

void SaveSlot::setOccupied(bool occupied) {
    m_occupied = occupied;
    updateVisual();
}

void SaveSlot::setLocked(bool locked) {
    m_locked = locked;
    updateVisual();
}

void SaveSlot::setType(SaveSlotType type) {
    m_slotType = type;
}

void SaveSlot::setCallback(std::function<void()> callback) {
    m_callback = std::move(callback);
}

void SaveSlot::setFont(const sf::Font& font) {
    m_font = &font;
    m_nameText.setFont(font);
    m_levelText.setFont(font);
    m_timeText.setFont(font);

    m_nameText.setCharacterSize(18);
    m_levelText.setCharacterSize(16);
    m_timeText.setCharacterSize(14);

    updateVisual();
}

bool SaveSlot::isOccupied() const {
    return m_occupied;
}

bool SaveSlot::isLocked() const {
    return m_locked;
}

SaveSlotType SaveSlot::getType() const {
    return m_slotType;
}

void SaveSlot::updateVisual() {
    if (!m_font) return;

    if (m_occupied) {
        m_nameText.setString(m_data.savePlayerName);

        std::wstringstream lv;
        lv << L"Lv." << m_data.savePlayerLv;
        m_levelText.setString(lv.str());

        m_timeText.setString(formatTime(m_data.saveTime));
    }
    else {
        m_nameText.setString(L"< Empty Slot >");
        m_levelText.setString(L"");
        m_timeText.setString(L"");
    }

    if (!isEnabled()) {
        m_background.setFillColor(sf::Color(30, 30, 30));
    }
    else if (m_locked) {
        m_background.setFillColor(sf::Color(80, 80, 80));
    }
    else if (m_isHovered) {
        m_background.setFillColor(sf::Color(100, 100, 100));
    }
    else {
        m_background.setFillColor(sf::Color(50, 50, 50));
    }
}

bool SaveSlot::containsPoint(sf::Vector2f point) const {
    return m_background.getGlobalBounds().contains(point);
}

std::wstring SaveSlot::formatTime(std::time_t rawTime) const {
    std::wstringstream ss;
    std::tm timeinfo{};
#ifdef _WIN32
    // Windows 平台使用 localtime_s
    if (localtime_s(&timeinfo, &rawTime) == 0) {
        ss << std::put_time(&timeinfo, L"%Y-%m-%d %H:%M");
    }
    else {
        ss << L"--";
    }
#else
    // 其他平台用 localtime (线程不安全)
    std::tm* pTime = std::localtime(&rawTime);
    if (pTime) {
        ss << std::put_time(pTime, L"%Y-%m-%d %H:%M");
    }
    else {
        ss << L"--";
    }
#endif
    return ss.str();
}

