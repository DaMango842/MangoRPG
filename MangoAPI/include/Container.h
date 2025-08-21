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

            // 直接将组件添加到观察者列表
            m_components.push_back(component.template cast_static<BaseComponent>());
        }
    }

    // 版本1：只绑定观察者（不获取所有权）
    template<typename T>
    void bindComponentObserver(MangoPtr<T>& component) {
        static_assert(std::derived_from<T, BaseComponent>,
            "T must be derived from BaseComponent");

        if (!component) return;
        component->setParent(this);
        m_components.push_back(component.template cast_static<BaseComponent>());
    }

    // 版本2：绑定并获取所有权
    template<typename T>
    void bindComponentOwned(MangoPtr<T> component) {
        static_assert(std::derived_from<T, BaseComponent>,
            "T must be derived from BaseComponent");

        if (!component) return;
        component->setParent(this);

        // 添加到观察者列表
        m_components.push_back(component.template staticCast<BaseComponent>());

        // 转换为 unique_ptr 并添加到拥有列表
        // 这需要 MangoPtr 提供 release() 方法
        if (component.unique()) {
            T* raw = component.release();
            m_ownedComponents.push_back(std::unique_ptr<BaseComponent>(raw));
        }
    }

    void bindComponent(MangoPtr<BaseComponent>& component);

    // BaseComponent interface
    void handleEvent(const sf::Event& event) override;
    void render(sf::RenderTarget& target) override;
    void update(float deltaTime) override;

    // Component access
    [[nodiscard]] const std::vector<MangoPtr<BaseComponent>>& getComponents() const noexcept;
    [[nodiscard]] const BaseComponent* getComponent(std::size_t index) const noexcept;
    [[nodiscard]] BaseComponent* getComponent(std::size_t index) noexcept;

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
