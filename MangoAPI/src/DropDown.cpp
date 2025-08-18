// DropDown.cpp
#include "DropDown.h"

DropDown::DropDown(const sf::Font& font, unsigned int charSize)
    : m_font(font), m_charSize(charSize)
{
    m_box.setFillColor(sf::Color(50, 50, 50));
    m_box.setOutlineColor(sf::Color::White);
    m_box.setOutlineThickness(1.f);
    m_box.setSize({ 200.f, static_cast<float>(charSize + 10) });

    m_selectedText.setFont(m_font);
    m_selectedText.setCharacterSize(m_charSize);
    m_selectedText.setFillColor(sf::Color::White);
    m_selectedText.setPosition(m_box.getPosition().x + 5.f, m_box.getPosition().y + 5.f);
}

void DropDown::addOption(const std::string& text)
{
    m_options.push_back(text);

    sf::Text optionText(text, m_font, m_charSize);
    optionText.setFillColor(sf::Color::White);
    optionText.setPosition(m_box.getPosition().x + 5.f,
        m_box.getPosition().y + (m_options.size()) * m_box.getSize().y);
    m_optionTexts.push_back(optionText);

    sf::RectangleShape optionBox;
    optionBox.setFillColor(sf::Color(70, 70, 70));
    optionBox.setOutlineColor(sf::Color::White);
    optionBox.setOutlineThickness(1.f);
    optionBox.setSize(m_box.getSize());
    optionBox.setPosition(m_box.getPosition().x,
        m_box.getPosition().y + (m_options.size()) * m_box.getSize().y);
    m_optionBoxes.push_back(optionBox);

    if (m_selectedIndex == -1)
        setSelected(0);
}

void DropDown::setSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(m_options.size()))
    {
        m_selectedIndex = index;
        m_selectedText.setString(m_options[index]);

        if (m_callback)
            m_callback(index, m_options[index]);
    }
}

int DropDown::getSelectedIndex() const
{
    return m_selectedIndex;
}

std::string DropDown::getSelectedValue() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_options.size()))
        return m_options[m_selectedIndex];
    return "";
}

void DropDown::setCallback(Callback callback)
{
    m_callback = std::move(callback);
}

void DropDown::setPosition(const sf::Vector2f& pos)
{
    m_box.setPosition(pos);
    m_selectedText.setPosition(pos.x + 5.f, pos.y + 5.f);
    updateLayout();
}

void DropDown::updateLayout()
{
    for (std::size_t i = 0; i < m_optionTexts.size(); ++i)
    {
        sf::Vector2f base = m_box.getPosition();
        float offsetY = (i + 1) * m_box.getSize().y;

        m_optionBoxes[i].setPosition(base.x, base.y + offsetY);
        m_optionTexts[i].setPosition(base.x + 5.f, base.y + offsetY + 5.f);
    }
}

void DropDown::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2f mousePos = { static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y) };

        if (m_box.getGlobalBounds().contains(mousePos))
        {
            m_expanded = !m_expanded;
            return;
        }

        if (m_expanded)
        {
            for (std::size_t i = 0; i < m_optionBoxes.size(); ++i)
            {
                if (m_optionBoxes[i].getGlobalBounds().contains(mousePos))
                {
                    setSelected(static_cast<int>(i));
                    m_expanded = false;
                    break;
                }
            }
        }
        else
        {
            m_expanded = false;
        }
    }
}

void DropDown::render(sf::RenderTarget& target)
{
    target.draw(m_box);
    target.draw(m_selectedText);

    if (m_expanded)
    {
        for (std::size_t i = 0; i < m_optionBoxes.size(); ++i)
        {
            target.draw(m_optionBoxes[i]);
            target.draw(m_optionTexts[i]);
        }
    }
}
