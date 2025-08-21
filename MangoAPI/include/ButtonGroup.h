#pragma once

#include "BaseComponent.h"
#include "Button.h"
#include <MangoPtr.hpp>
#include <MangoVector.hpp>
#include <functional>
#include <optional>

class ButtonGroup : public BaseComponent
{
public:
    using ButtonPtr = MangoPtr<Button>;
    using ButtonCallback = std::function<void(Button*)>;

    ButtonGroup();
    ~ButtonGroup();

    void addButton(ButtonPtr button);
    bool removeButton(Button* button);
    void clearButtons();

    void setFocus(int index);
    void moveFocus(int direction); // -1: up/left, +1: down/right

    void setButtonCallback(const ButtonCallback& callback);
    void setNavigationEnabled(bool enabled);

    void setSpacing(float spacing);
    void arrangeVertically(float padding = 10.0f);
    void arrangeHorizontally(float padding = 10.0f);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    [[nodiscard]] size_t buttonCount() const noexcept;
    [[nodiscard]] int focusedIndex() const noexcept;
    [[nodiscard]] const Button* focusedButton() const noexcept;
    [[nodiscard]] Button* focusedButton() noexcept;

private:
    MangoVector<ButtonPtr> m_buttons;
    int m_focusedIndex = -1;
    bool m_navigationEnabled = true;
    ButtonCallback m_buttonCallback;

    void updateButtonPositions();
    float m_spacing = 10.0f;
    std::optional<sf::Vector2f> m_lastSize;
};
