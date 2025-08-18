#pragma once
#include "BaseComponent.h"
#include "RadioButton.h"
#include <vector>
#include <memory>
#include <optional>

class RadioBox : public BaseComponent {
public:
    RadioBox() = default;
    ~RadioBox() { m_buttons.clear(); }

    void addOption(const sf::String& label, sf::Font& font, std::function<void()> callback = nullptr);
    void setPosition(const sf::Vector2f& startPos, float offsetY = 40.f);
    sf::Vector2f getPosition() const;  // ✅ 新增

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void selectOnly(SafePtr<RadioButton> selected);
    void selectOnly(bool selected);

    void selectOption(size_t index);
    RadioButton* getOption(size_t index);

    bool getOptionSelected();
    std::optional<size_t> getSelectedIndex() const;

private:
    std::vector<std::unique_ptr<RadioButton>> m_buttons;
    size_t m_selectedIndex = 0;
    sf::Vector2f m_position;  // ✅ 新增
};
