// DropDown.h
#pragma once

#include "BaseComponent.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

// DropDown.h（只改了 setPosition 的声明）
class DropDown : public BaseComponent
{
public:
    using Callback = std::function<void(int index, const std::string& value)>;

    DropDown(const sf::Font& font, unsigned int charSize = 16);

    void addOption(const std::string& text);
    void setSelected(int index);
    int getSelectedIndex() const;
    std::string getSelectedValue() const;

    void setCallback(Callback callback);

    void setPosition(const sf::Vector2f& pos);  // ✅ 不再 override
    const sf::Vector2f& getPosition() const;

    void handleEvent(const sf::Event& event) override;
    void render(sf::RenderTarget& target) override;
    void update(float deltaTime) override {}

private:
    void updateLayout();

    sf::Font m_font;
    unsigned int m_charSize;

    std::vector<std::string> m_options;
    int m_selectedIndex = -1;

    bool m_expanded = false;

    sf::Vector2f m_position;

    sf::RectangleShape m_box;
    sf::Text m_selectedText;

    std::vector<sf::Text> m_optionTexts;
    std::vector<sf::RectangleShape> m_optionBoxes;

    Callback m_callback;
};


