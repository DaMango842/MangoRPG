#pragma once

#include "BaseState.h"
#include "Button.h"
#include "Label.h"
#include "Container.h"
#include "ScrollBar.h"
#include "StateManager.h"
#include "Player.h"
#include "Window.h"
#include "SaveSlot.h"

#include "SafeTypes.hpp"   // 添加

#include "Translator.h"
#include "SafeCallback.hpp"

#include <vector>
#include <SFML/Graphics.hpp>

#include <MangoPtr.hpp>

class SaveMenuState : public BaseState {
public:
    SaveMenuState(SafeRef<StateManager> stateManager, MangoPtr<Player> player, SaveSlotType type = SaveSlotType::Save);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

private:
    SafeRef<StateManager> m_stateManager;
    MangoPtr<Player> m_player;              // 用 UniqueHandle 替代 std::unique_ptr<Player>
    sf::Font& m_font;
    sf::Text m_title;
    UniqueHandle<Button> m_backBtn;

    Container m_container;
    std::vector<SafePtr<SaveSlot>> m_saveSlots; // 用 SafePtr<SaveSlot> 替代裸指针

    ScrollBar m_scrollBar;
    sf::View m_scrollView;
    float m_scrollOffset = 0.f;

    SaveSlotType m_type;

    bool m_refreshRequested = false;
    sf::Clock m_refreshTimer;

    void setupUI();
    void refreshUI();
    void updateScrollView();

    bool isMouseEvent(const sf::Event& event);
    sf::Vector2f getMousePosition(const sf::Event& event);
    sf::Event transformEventForContainer(const sf::Event& event);
    void updateEventCoordinates(sf::Event& event, const sf::Vector2f& pos);
};
