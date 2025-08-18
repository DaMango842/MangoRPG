#include "SaveMenuState.h"

#include "ResourceLoader.h"
#include "SaveManager.h"
#include "GameState.h"

#include <iostream>
#include <iomanip>

SaveMenuState::SaveMenuState(SafeRef<StateManager> stateManager, MangoPtr<Player> player, SaveSlotType type)
    : m_stateManager(stateManager),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf")),
    m_player(player),
    m_scrollOffset(0.f),
    m_type(type)
{
}


void SaveMenuState::setupUI() 
{
    auto& window = Window::getInstance().getWindow();
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());

    m_title.setFont(m_font);
    m_title.setString(m_type == SaveSlotType::Save ? TR("SAVE_LOAD_MENU.SAVE_TITLE") : TR("SAVE_LOAD_MENU.LOAD_TITLE"));
    m_title.setCharacterSize(36);
    m_title.setFillColor(sf::Color::White);
    m_title.setPosition(50.f, 20.f);

    const float titleHeight = 60.f;
    const float viewportTop = titleHeight / windowSize.y;
    const float viewportHeight = 0.9f - viewportTop;

    m_scrollView.setSize(windowSize);
    m_scrollView.setCenter(windowSize.x / 2.f, windowSize.y / 2.f);
    m_scrollView.setViewport(sf::FloatRect(0.f, viewportTop, 1.f, viewportHeight));

    const float scrollBarWidth = 15.f;
    const float scrollBarX = windowSize.x - scrollBarWidth - 5.f;
    m_scrollBar.setSize({ scrollBarWidth, windowSize.y * viewportHeight });
    m_scrollBar.setPosition({ scrollBarX, titleHeight });
    m_scrollBar.setViewHeight(windowSize.y * viewportHeight);

    m_backBtn = std::make_unique<Button>(TR("BACKBTN"), m_font);
    m_backBtn->setSize({ 200.f, 50.f });
    m_backBtn->setCallback([this]() {
        m_stateManager->safePopState();
        });

    const float buttonWidth = 1000.f;
    const float buttonHeight = 200.f;
    const float verticalPadding = 30.f;
    const float startX = windowSize.x * 0.05f;
    const float startY = 20.f;

    m_container.clearComponents();

    auto slotInfos = SaveManager::getInstance().getAllSlotInfos();

    for (int i = 0; i < MAX_SLOTS; ++i) {
        float y = startY + i * (buttonHeight + verticalPadding);

        if (m_type == SaveSlotType::Save) {
            auto saveBtn = std::make_unique<Button>(L"", m_font);
            saveBtn->setPosition({ startX, y });
            saveBtn->setSize({ buttonWidth * 0.6f, buttonHeight });
            saveBtn->setCallback([this, i]() {
                if (SaveManager::getInstance().saveToSlot(i, SafePtr<Player>(m_player.get()))) {
                    std::wcout << L"保存成功：槽位 " << i + 1 << std::endl;
                    m_refreshRequested = true;
                    m_refreshTimer.restart();
                }
                else {
                    std::wcout << L"保存失败：槽位 " << i + 1 << std::endl;
                }
                });
            m_container.bindComponent(std::move(saveBtn));
        }
        else if (m_type == SaveSlotType::Load) {
            auto loadBtn = std::make_unique<Button>(L"", m_font);
            loadBtn->setPosition({ startX, y });
            loadBtn->setSize({ buttonWidth * 0.6f, buttonHeight });
            loadBtn->setCallback([this, i]() {
                m_player = SaveManager::getInstance().loadFromSlot(i);
                if (m_player) {

                    m_stateManager->queueStateChange(std::make_unique<GameState>(m_stateManager, m_player));
                }
                else
                {
                    std::wcout << L"读取失败：槽位 " << i + 1 << std::endl;
                }

                });
            m_container.bindComponent(std::move(loadBtn));
        }

        std::wstring slotTitle = TR("SAVE_LOAD_MENU.SLOT.SLOT_TITLE") + std::to_wstring(i + 1);
        std::wstring playerName = TR("SAVE_LOAD_MENU.SLOT.SLOT_EMPTY");
        std::wstring timeText = L"--";
        std::wstring attrText = L"";

        if (slotInfos.size() > (size_t)i && slotInfos[i].valid) {
            const std::string& nameUtf8 = slotInfos[i].playerName;
            if (!nameUtf8.empty()) {
                playerName = sf::String::fromUtf8(nameUtf8.begin(), nameUtf8.end());
            }

            if (slotInfos[i].timestamp > 0) {
                timeText = formatTimestamp(slotInfos[i].timestamp);
            }

            const auto& attrs = slotInfos[i].attributesPreview;
            int lv = attrs.contains("LV") ? static_cast<int>(attrs.at("LV")) : -1;
            int hp = attrs.contains("HP") ? static_cast<int>(attrs.at("HP")) : -1;
            int money = attrs.contains("MONEY") ? static_cast<int>(attrs.at("MONEY")) : -1;

            if (lv != -1 && hp != -1 && money != -1) {
                attrText = TR("STATUS.LV") + ": " + std::to_wstring(lv)
                    + L"   " + TR("STATUS.HP") + ": " + std::to_wstring(hp)
                    + L"   " + TR("STATUS.MONEY") + ": " + std::to_wstring(money);
            }
            else {
                attrText = L"(属性数据不完整)";
            }
        }
        else {
            timeText = L"--:--:--";
            attrText = L"";
        }

        auto slotLabel = std::make_unique<Label>(slotTitle, m_font, 20);
        slotLabel->setPosition({ startX + 10.f, y + 8.f });
        m_container.bindComponent(std::move(slotLabel));

        auto timeLabel = std::make_unique<Label>(timeText, m_font, 20);
        timeLabel->setPosition({ startX + buttonWidth - 180.f, y + 8.f });
        m_container.bindComponent(std::move(timeLabel));

        auto nameLabel = std::make_unique<Label>(playerName, m_font, 30);
        nameLabel->setPosition({ startX + 10.f, y + 45.f });
        m_container.bindComponent(std::move(nameLabel));

        auto attrLabel = std::make_unique<Label>(attrText, m_font, 22);
        attrLabel->setPosition({ startX + 10.f, y + 90.f });
        m_container.bindComponent(std::move(attrLabel));
    }

    float contentHeight = startY + MAX_SLOTS * (buttonHeight + verticalPadding) + 30.f;
    m_scrollBar.setContentHeight(contentHeight);

    // 设置返回按钮位置（屏幕右上）
    m_backBtn->setPosition({ windowSize.x - 220.f, 20.f });
}

void SaveMenuState::refreshUI()
{
    setupUI();
}

void SaveMenuState::handleEvent(const sf::Event& event) {
    m_scrollBar.handleEvent(event);
    updateScrollView();

    sf::Event transformedEvent = transformEventForContainer(event);
    m_container.handleEvent(transformedEvent);

    m_backBtn->handleEvent(event); // 添加按钮事件处理
}

sf::Event SaveMenuState::transformEventForContainer(const sf::Event& event) {
    if (!isMouseEvent(event)) return event;

    auto& window = Window::getInstance().getWindow();
    sf::Event transformedEvent = event;

    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, m_scrollView);
    updateEventCoordinates(transformedEvent, worldPos);

    return transformedEvent;
}

bool SaveMenuState::isMouseEvent(const sf::Event& event) {
    return event.type == sf::Event::MouseButtonPressed ||
        event.type == sf::Event::MouseButtonReleased ||
        event.type == sf::Event::MouseMoved ||
        event.type == sf::Event::MouseWheelScrolled;
}

sf::Vector2f SaveMenuState::getMousePosition(const sf::Event& event) {
    auto& window = Window::getInstance().getWindow();
    sf::Vector2i pixelPos;

    if (event.type == sf::Event::MouseMoved) {
        pixelPos = { event.mouseMove.x, event.mouseMove.y };
    }
    else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased) {
        pixelPos = { event.mouseButton.x, event.mouseButton.y };
    }
    else {
        return { 0.f, 0.f };
    }

    window.setView(m_scrollView);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
    window.setView(window.getDefaultView());
    worldPos.y += m_scrollOffset;
    return worldPos;
}

void SaveMenuState::updateEventCoordinates(sf::Event& event, const sf::Vector2f& pos) {
    switch (event.type) {
    case sf::Event::MouseMoved:
        event.mouseMove.x = static_cast<int>(pos.x);
        event.mouseMove.y = static_cast<int>(pos.y);
        break;
    case sf::Event::MouseButtonPressed:
    case sf::Event::MouseButtonReleased:
        event.mouseButton.x = static_cast<int>(pos.x);
        event.mouseButton.y = static_cast<int>(pos.y);
        break;
    case sf::Event::MouseWheelScrolled:
        event.mouseWheelScroll.x = static_cast<int>(pos.x);
        event.mouseWheelScroll.y = static_cast<int>(pos.y);
        break;
    default:
        break;
    }
}

void SaveMenuState::update(float deltaTime) {
    m_scrollBar.update(deltaTime);
    updateScrollView();
    m_container.update(deltaTime);

    if (m_refreshRequested && m_refreshTimer.getElapsedTime().asSeconds() > 0.1f) {
        setupUI();
        m_refreshRequested = false;
    }
}

void SaveMenuState::render(sf::RenderTarget& target) {
    sf::View oldView = target.getView();

    // 默认视图绘制标题 & 按钮
    target.setView(oldView);
    target.draw(m_title);
    m_backBtn->render(target);

    // 滚动视图绘制内容
    target.setView(m_scrollView);
    m_container.render(target);

    // 滚动条绘制
    target.setView(oldView);
    m_scrollBar.render(target);
}

void SaveMenuState::onEnter()
{
    setupUI();

    SafePtr<SaveMenuState> self(this);

    SafeCallback<> cb(self, &SaveMenuState::refreshUI);
    LanguageObserver::get().subscribe([cb]() mutable {
        if (cb) cb();
        });
}

void SaveMenuState::onExit()
{
}

void SaveMenuState::updateScrollView() {
    m_scrollOffset = m_scrollBar.getValue();
    sf::Vector2f center = m_scrollView.getSize() / 2.f;
    center.y += m_scrollOffset;
    m_scrollView.setCenter(center);
}
