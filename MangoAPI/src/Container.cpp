#include "Container.h"

void Container::addComponent(std::unique_ptr<BaseComponent> component) {
    if (!component) return;
    BaseComponent* raw = component.get();
    component->setParent(this);
    m_ownedComponents.push_back(std::move(component));
    m_components.push_back(MangoPtr<BaseComponent>::observe(raw));
}

void Container::addComponent(MangoPtr<BaseComponent> observer) {
    if (!observer) return;
    m_components.push_back(observer);
}

void Container::removeComponent(BaseComponent* component) {
    if (!component) return;

    std::erase_if(m_components, [component](const auto& p) {
        return p.get() == component;
        });

    std::erase_if(m_ownedComponents, [component](const auto& p) {
        return p.get() == component;
        });
}

void Container::clearComponents() {
    m_components.clear();
    m_ownedComponents.clear();
}

void Container::safeClearComponents() {
    for (auto& comp : m_components) {
        if (comp) comp->setEnabled(false);
    }
}

void Container::bindComponent(std::unique_ptr<BaseComponent> component) {
    addComponent(std::move(component));
}

void Container::bindComponent(MangoPtr<BaseComponent>& component) {
    if (!component) return;
    component->setParent(this);
    addComponent(component);
}

void Container::handleEvent(const sf::Event& event) {
    for (auto& comp : m_components) {
        if (comp && comp->isEnabled()) {
            comp->handleEvent(event);
        }
    }
}

void Container::render(sf::RenderTarget& target) {
    if (!isVisible()) return;
    for (auto& comp : m_components) {
        if (comp) comp->render(target);
    }
}

void Container::update(float deltaTime) {
    for (auto& comp : m_components) {
        if (comp) comp->update(deltaTime);
    }
}

const std::vector<MangoPtr<BaseComponent>>& Container::getComponents() const noexcept {
    return m_components;
}

BaseComponent* Container::getComponent(std::size_t index) const noexcept {
    return (index < m_components.size()) ? m_components[index].get() : nullptr;
}
