#include "ButtonGroup.h"
#include <algorithm>

ButtonGroup::ButtonGroup() = default;
ButtonGroup::~ButtonGroup() = default;

void ButtonGroup::addButton(ButtonPtr button)
{
    if (!button) return;

    // 如果按钮已在组中，不再添加
    auto it = std::find_if(m_buttons.begin(), m_buttons.end(),
        [button](const ButtonPtr& ptr) { return ptr.get() == button.get(); });

    if (it != m_buttons.end()) return;

    m_buttons.push_back(button);

    // 设置按钮回调
    if (m_buttonCallback) {
        button->setCallback([this, btn = button.get()]() {
            m_buttonCallback(btn);
            });
    }

    // 自动聚焦第一个可用按钮
    if (m_focusedIndex == -1 && button->isEnabled()) {
        button->setFocused(true);
        m_focusedIndex = static_cast<int>(m_buttons.size()) - 1;
    }

    updateButtonPositions();
}

bool ButtonGroup::removeButton(Button* button)
{
    if (!button) return false;

    // 手动查找并移除按钮
    size_t removeIndex = static_cast<size_t>(-1);
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].get() == button) {
            removeIndex = i;
            break;
        }
    }

    if (removeIndex == static_cast<size_t>(-1)) return false;

    // 更新焦点索引
    if (m_focusedIndex == static_cast<int>(removeIndex)) {
        m_buttons[removeIndex]->setFocused(false);
        m_focusedIndex = -1;

        // 尝试找到新的焦点按钮
        for (int i = 0; i < static_cast<int>(m_buttons.size()); ++i) {
            if (i != static_cast<int>(removeIndex) && m_buttons[i] && m_buttons[i]->isEnabled()) {
                setFocus(i);
                break;
            }
        }
    }
    else if (m_focusedIndex > static_cast<int>(removeIndex)) {
        m_focusedIndex--;
    }

    // 手动移除元素
    for (size_t i = removeIndex; i < m_buttons.size() - 1; ++i) {
        m_buttons[i] = m_buttons[i + 1];
    }
    m_buttons.pop_back();

    updateButtonPositions();
    return true;
}

void ButtonGroup::clearButtons()
{
    if (m_focusedIndex >= 0 && m_focusedIndex < static_cast<int>(m_buttons.size()) &&
        m_buttons[m_focusedIndex]) {
        m_buttons[m_focusedIndex]->setFocused(false);
    }

    m_buttons.clear();
    m_focusedIndex = -1;
    m_lastSize.reset();
}

void ButtonGroup::setFocus(int index)
{
    if (index < 0 || index >= static_cast<int>(m_buttons.size()) || !m_buttons[index])
        return;

    if (!m_buttons[index]->isEnabled())
        return;

    // 移除旧焦点
    if (m_focusedIndex >= 0 && m_focusedIndex < static_cast<int>(m_buttons.size()) &&
        m_buttons[m_focusedIndex]) {
        m_buttons[m_focusedIndex]->setFocused(false);
    }

    // 设置新焦点
    m_buttons[index]->setFocused(true);
    m_focusedIndex = index;
}

void ButtonGroup::moveFocus(int direction)
{
    if (m_buttons.empty() || !m_navigationEnabled)
        return;

    int total = static_cast<int>(m_buttons.size());
    if (total == 0) return;

    int start = (m_focusedIndex >= 0) ? m_focusedIndex : 0;
    int current = start;

    for (int i = 0; i < total; ++i) {
        current = (current + direction + total) % total;

        if (m_buttons[current] && m_buttons[current]->isEnabled()) {
            setFocus(current);
            return;
        }
    }
}

void ButtonGroup::setButtonCallback(const ButtonCallback& callback)
{
    m_buttonCallback = callback;

    // 更新所有现有按钮的回调
    for (auto& button : m_buttons) {
        if (button) {
            button->setCallback([this, btn = button.get()]() {
                m_buttonCallback(btn);
                });
        }
    }
}

void ButtonGroup::setNavigationEnabled(bool enabled)
{
    m_navigationEnabled = enabled;
}

void ButtonGroup::setSpacing(float spacing)
{
    m_spacing = spacing;
    updateButtonPositions();
}

void ButtonGroup::arrangeVertically(float padding)
{
    if (m_buttons.empty()) return;

    float y = 0;
    for (auto& button : m_buttons) {
        if (!button) continue;

        // 使用正确的Vector2f设置位置
        button->setPosition(sf::Vector2f(0, y));

        // 获取按钮尺寸
        auto bounds = button->getLocalBounds();
        y += bounds.height + m_spacing;
    }

    // 保存布局尺寸
    m_lastSize = sf::Vector2f(0, y - m_spacing + padding);
}

void ButtonGroup::arrangeHorizontally(float padding)
{
    if (m_buttons.empty()) return;

    float x = 0;
    for (auto& button : m_buttons) {
        if (!button) continue;

        // 使用正确的Vector2f设置位置
        button->setPosition(sf::Vector2f(x, 0));

        // 获取按钮尺寸
        auto bounds = button->getLocalBounds();
        x += bounds.width + m_spacing;
    }

    // 保存布局尺寸
    m_lastSize = sf::Vector2f(x - m_spacing + padding, 0);
}

void ButtonGroup::updateButtonPositions()
{
    if (m_lastSize) {
        // 重新应用上次的排列
        if (m_lastSize->y > 0) {
            arrangeVertically(m_lastSize->y);
        }
        else if (m_lastSize->x > 0) {
            arrangeHorizontally(m_lastSize->x);
        }
    }
}

void ButtonGroup::handleEvent(const sf::Event& event)
{
    if (m_buttons.empty()) return;

    bool eventHandled = false;

    // 优先处理导航事件
    if (m_navigationEnabled && event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Left) {
            moveFocus(-1);
            eventHandled = true;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Right) {
            moveFocus(+1);
            eventHandled = true;
        }
        else if (
            (event.key.code == sf::Keyboard::Enter ||
                event.key.code == sf::Keyboard::Space ||
                event.key.code == sf::Keyboard::Z) &&
            m_focusedIndex >= 0) {
            if (m_buttons[m_focusedIndex]) {
                m_buttons[m_focusedIndex]->simulateClick();
                eventHandled = true;
            }
        }
    }

    // 如果导航事件未处理，传递给按钮
    if (!eventHandled) {
        // 首先传递给焦点按钮
        if (m_focusedIndex >= 0 && m_focusedIndex < static_cast<int>(m_buttons.size()) &&
            m_buttons[m_focusedIndex]) {
            m_buttons[m_focusedIndex]->handleEvent(event);
        }

        // 然后传递给其他按钮（处理鼠标悬停等）
        for (auto& button : m_buttons) {
            if (button && button.get() != focusedButton()) {
                button->handleEvent(event);
            }
        }
    }
}

void ButtonGroup::update(float deltaTime)
{
    for (auto& button : m_buttons) {
        if (button) {
            button->update(deltaTime);
        }
    }
}

void ButtonGroup::render(sf::RenderTarget& target)
{
    for (auto& button : m_buttons) {
        if (button) {
            button->render(target);
        }
    }
}

size_t ButtonGroup::buttonCount() const noexcept
{
    return m_buttons.size();
}

int ButtonGroup::focusedIndex() const noexcept
{
    return m_focusedIndex;
}

const Button* ButtonGroup::focusedButton() const noexcept
{
    if (m_focusedIndex >= 0 && m_focusedIndex < static_cast<int>(m_buttons.size()) &&
        m_buttons[m_focusedIndex]) {
        return m_buttons[m_focusedIndex].get();
    }
    return nullptr;
}

Button* ButtonGroup::focusedButton() noexcept
{
    if (m_focusedIndex >= 0 && m_focusedIndex < static_cast<int>(m_buttons.size()) &&
        m_buttons[m_focusedIndex]) {
        return m_buttons[m_focusedIndex].get();
    }
    return nullptr;
}
