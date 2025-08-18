#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include "SafeTypes.hpp"

// 为每个回调分配唯一 ID
using CallbackID = size_t;

class LanguageObserver {
public:
    static LanguageObserver& get() {
        static LanguageObserver instance;
        return instance;
    }

    // 订阅一个普通函数对象，返回 ID
    CallbackID subscribe(std::function<void()> callback) {
        CallbackID id = m_nextId++;
        m_callbacks.emplace_back(id, std::move(callback));
        return id;
    }

    // 支持带 SafePtr 的成员函数订阅
    template<typename T>
    CallbackID subscribe(SafePtr<T> obj, void (T::* method)()) {
        return subscribe([obj, method]() {
            if (obj) ((*obj).*method)();
            });
    }

    // 注销指定 ID 的回调
    void unsubscribe(CallbackID id) {
        m_callbacks.erase(
            std::remove_if(m_callbacks.begin(), m_callbacks.end(),
                [id](const auto& pair) { return pair.first == id; }),
            m_callbacks.end());
    }

    // 通知所有订阅者
    void notify() {
        for (auto& [id, cb] : m_callbacks) {
            if (cb) cb();
        }
    }

private:
    LanguageObserver() = default;

    CallbackID m_nextId = 1;
    std::vector<std::pair<CallbackID, std::function<void()>>> m_callbacks;
};
