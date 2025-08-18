#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>

enum class EventType {
    Closed,
    Resized,
    KeyPressed,
    KeyReleased,
    MouseMoved,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseWheelScrolled,
    Custom // 用户定义事件
};

class EventManager {
public:
    using Callback = std::function<bool(const sf::Event&)>;
    using CallbackID = size_t;

    // 订阅单个事件，返回唯一ID
    CallbackID subscribe(EventType type, Callback callback) {
        CallbackID id = ++m_lastID;
        m_callbacks[type].emplace_back(CallbackEntry{ id, std::move(callback) });
        return id;
    }

    // 批量订阅，返回所有生成的ID
    std::vector<CallbackID> subscribe(const std::vector<EventType>& types, Callback callback) {
        std::vector<CallbackID> ids;
        for (auto& type : types) {
            ids.push_back(subscribe(type, callback));
        }
        return ids;
    }

    // 根据ID取消订阅
    void unsubscribe(EventType type, CallbackID id) {
        auto it = m_callbacks.find(type);
        if (it != m_callbacks.end()) {
            auto& vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                [id](const CallbackEntry& entry) { return entry.id == id; }), vec.end());
        }
    }

    // 事件轮询
    void poll(sf::RenderWindow& window) {
        sf::Event event;
        while (window.pollEvent(event)) {
            dispatch(event);
        }

        while (!m_customEvents.empty()) {
            sf::Event custom = std::move(m_customEvents.front());
            m_customEvents.pop();
            call(EventType::Custom, custom);
        }
    }

    void dispatchSingle(const sf::Event& event) {
        // 直接用你原来私有 dispatch + call 逻辑
        dispatch(event);
    }

    // 触发自定义事件
    void fireCustomEvent(sf::Event customEvent) {
        m_customEvents.push(std::move(customEvent));
    }

private:
    struct CallbackEntry {
        CallbackID id;
        Callback callback;
    };

    void dispatch(const sf::Event& event) {
        EventType type;
        switch (event.type) {
        case sf::Event::Closed: type = EventType::Closed; break;
        case sf::Event::Resized: type = EventType::Resized; break;
        case sf::Event::KeyPressed: type = EventType::KeyPressed; break;
        case sf::Event::KeyReleased: type = EventType::KeyReleased; break;
        case sf::Event::MouseMoved: type = EventType::MouseMoved; break;
        case sf::Event::MouseButtonPressed: type = EventType::MouseButtonPressed; break;
        case sf::Event::MouseButtonReleased: type = EventType::MouseButtonReleased; break;
        case sf::Event::MouseWheelScrolled: type = EventType::MouseWheelScrolled; break;
        default: return; // 忽略不支持的事件
        }
        call(type, event);
    }

    void call(EventType type, const sf::Event& event) {
        auto it = m_callbacks.find(type);
        if (it != m_callbacks.end()) {
            for (auto& entry : it->second) {
                if (entry.callback(event)) break; // 事件处理返回true则停止传播
            }
        }
    }

    std::unordered_map<EventType, std::vector<CallbackEntry>> m_callbacks;
    std::queue<sf::Event> m_customEvents;
    CallbackID m_lastID = 0;
};
