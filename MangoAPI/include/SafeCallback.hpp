#pragma once
#include <functional>
#include "SafeTypes.hpp" // 你已有的 SafePtr, SafeRef

template<typename... Args>
class SafeCallback {
public:
    SafeCallback() = default;

    template<typename T>
    SafeCallback(SafePtr<T> obj, void (T::* method)(Args...)) {
        bind(obj, method);
    }

    template<typename T>
    void bind(SafePtr<T> obj, void (T::* method)(Args...)) {
        m_func = [obj, method](Args... args) {
            if (obj) {
                ((*obj).*method)(std::forward<Args>(args)...);
            }
            };
    }

    void reset() {
        m_func = nullptr;
    }

    bool valid() const {
        return static_cast<bool>(m_func);
    }

    void operator()(Args... args) const {
        if (m_func) m_func(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
        return valid();
    }

private:
    std::function<void(Args...)> m_func;
};

