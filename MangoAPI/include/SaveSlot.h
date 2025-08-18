#pragma once

#include "BaseComponent.h"
#include <SFML/Graphics.hpp>
#include <ctime>
#include <functional>

// 存档槽类型（用于不同操作场景）
enum class SaveSlotType {
    Save,
    Load,
    Delete
};

// 存档槽数据结构
struct BaseSaveSlotData {
    size_t saveID;
    sf::String savePlayerName = L"Player";
    size_t savePlayerLv = 1;
    std::time_t saveTime = 0;
    size_t gameTime = 0;

    // 便携创建静态方法
    static BaseSaveSlotData Create(
        size_t id,
        const sf::String& name = L"Player",
        size_t level = 1,
        std::time_t time = std::time(nullptr),
        size_t game_time = 0)
    {
        BaseSaveSlotData data;
        data.saveID = id;
        data.savePlayerName = name;
        data.savePlayerLv = level;
        data.saveTime = time;
        data.gameTime = game_time;
        return data;
    }
};


class SaveSlot : public BaseComponent {
public:
    SaveSlot();
    ~SaveSlot();

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void setData(const BaseSaveSlotData& data);
    void setOccupied(bool occupied);
    void setLocked(bool locked);
    void setType(SaveSlotType type);
    void setCallback(std::function<void()> callback);
    void setFont(const sf::Font& font);

    bool isOccupied() const;
    bool isLocked() const;
    SaveSlotType getType() const;

private:
    BaseSaveSlotData m_data;
    bool m_occupied = false;
    bool m_locked = false;
    SaveSlotType m_slotType = SaveSlotType::Save;

    sf::RectangleShape m_background;
    sf::Text m_nameText;
    sf::Text m_levelText;
    sf::Text m_timeText;

    const sf::Font* m_font = nullptr;
    std::function<void()> m_callback;

    bool m_isHovered = false;

    void updateVisual();
    bool containsPoint(sf::Vector2f point) const;
    std::wstring formatTime(std::time_t rawTime) const;
};
