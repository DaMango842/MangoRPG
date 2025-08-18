#pragma once

#include <vector>
#include <memory>
#include <concepts>
#include <type_traits>
#include <SFML/Graphics.hpp>
#include "BaseComponent.h"
#include "MangoPtr.hpp"

class Container : public BaseComponent {
public:
    Container() = default;
    ~Container() override = default;

    // Component management
    void addComponent(std::unique_ptr<BaseComponent> component);
    void addComponent(MangoPtr<BaseComponent> observer);
    void removeComponent(BaseComponent* component);
    void clearComponents();
    void safeClearComponents();

    // Component binding with parent relationship
    void bindComponent(std::unique_ptr<BaseComponent> component);

    template<typename T>
    void bindComponent(MangoPtr<T>& component) {
        if constexpr (std::derived_from<T, BaseComponent>) {
            if (!component) return;
            component->setParent(this);
            addComponent(MangoPtr<BaseComponent>::observe(component.get()));
        }
    }

    void bindComponent(MangoPtr<BaseComponent>& component);

    // BaseComponent interface
    void handleEvent(const sf::Event& event) override;
    void render(sf::RenderTarget& target) override;
    void update(float deltaTime) override;

    // Component access
    [[nodiscard]] const std::vector<MangoPtr<BaseComponent>>& getComponents() const noexcept;
    [[nodiscard]] BaseComponent* getComponent(std::size_t index) const noexcept;

    // Iteration support
    [[nodiscard]] auto begin() noexcept { return m_components.begin(); }
    [[nodiscard]] auto end() noexcept { return m_components.end(); }
    [[nodiscard]] auto begin() const noexcept { return m_components.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_components.end(); }
    [[nodiscard]] auto cbegin() const noexcept { return m_components.cbegin(); }
    [[nodiscard]] auto cend() const noexcept { return m_components.cend(); }

private:
    std::vector<MangoPtr<BaseComponent>> m_components;
    std::vector<std::unique_ptr<BaseComponent>> m_ownedComponents;
};
