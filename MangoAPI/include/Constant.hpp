#pragma once

#include <variant>
#include <string>
#include <string_view>
#include <unordered_map>
#include <mutex>
#include <shared_mutex> // 添加缺失的头文件
#include <functional>
#include <stdexcept>
#include <optional>
#include <utility>
#include <concepts>
#include <cctype>
#include <algorithm>
#include <iostream> // 用于日志输出

// 使用 string_view 避免不必要的字符串拷贝
class FlagName {
public:
    using KeyVariant = std::variant<std::string, std::wstring>;

    // 支持各种字符串类型
    constexpr FlagName(std::string_view sv) : m_key(std::string(sv)) {}
    constexpr FlagName(std::wstring_view wsv) : m_key(std::wstring(wsv)) {}
    constexpr FlagName(const char* s) : m_key(std::string(s)) {}
    constexpr FlagName(const wchar_t* ws) : m_key(std::wstring(ws)) {}

    // 移动语义支持
    constexpr FlagName(std::string&& s) : m_key(std::move(s)) {}
    constexpr FlagName(std::wstring&& ws) : m_key(std::move(ws)) {}

    // 三向比较 (C++20)
    auto operator<=>(const FlagName& other) const = default;

    const KeyVariant& value() const noexcept { return m_key; }

private:
    KeyVariant m_key;
};

// 自定义哈希 (使用 if constexpr)
template <>
struct std::hash<FlagName> {
    size_t operator()(const FlagName& name) const noexcept {
        return std::visit([](auto&& str) {
            using T = std::decay_t<decltype(str)>;
            if constexpr (std::is_same_v<T, std::string> ||
                std::is_same_v<T, std::wstring>) {
                return std::hash<T>{}(str);
            }
            else {
                static_assert(std::is_same_v<T, void>, "Unsupported type");
            }
            }, name.value());
    }
};

// 简单的 Flag 类 (C++17/20 风格)
class Flag {
public:
    using Value = std::variant<bool, int, float, std::string, std::wstring>;

    // 设置标志值
    void set(FlagName name, Value value) {
        flags_.insert_or_assign(std::move(name), std::move(value));
    }

    // 获取标志值 (返回 optional)
    [[nodiscard]] std::optional<Value> get(const FlagName& name) const {
        if (auto it = flags_.find(name); it != flags_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 安全获取值 (带默认值)
    template <typename T>
    [[nodiscard]] T get_or(const FlagName& name, T&& default_value) const {
        if (auto it = flags_.find(name); it != flags_.end()) {
            if constexpr (std::is_same_v<T, bool>) {
                if (auto val = std::get_if<bool>(&it->second)) return *val;
            }
            else if constexpr (std::is_integral_v<T>) {
                if (auto val = std::get_if<int>(&it->second)) return static_cast<T>(*val);
            }
            else if constexpr (std::is_floating_point_v<T>) {
                if (auto val = std::get_if<float>(&it->second)) return static_cast<T>(*val);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                if (auto val = std::get_if<std::string>(&it->second)) return *val;
            }
            else if constexpr (std::is_same_v<T, std::wstring>) {
                if (auto val = std::get_if<std::wstring>(&it->second)) return *val;
            }
        }
        return std::forward<T>(default_value);
    }

private:
    std::unordered_map<FlagName, Value> flags_;
};

// ====================================
// 游戏全局标志系统 (C++17/20 现代实现)
// ====================================
class GlobalFlag {
public:
    using Value = std::variant<bool, int, float, std::string>;
    using Callback = std::function<void(const FlagName&, const Value&)>;

    // 获取单例实例 (C++17 保证线程安全)
    static GlobalFlag& instance() noexcept {
        static GlobalFlag instance;
        return instance;
    }

    // 设置全局标志 (带变化检测)
    void set(FlagName name, Value value) {
        std::scoped_lock lock(mutex_);

        // 检查值是否变化 (C++17 if初始化 + 结构化绑定)
        bool changed = true;
        if (auto [it, inserted] = flags_.try_emplace(name, std::move(value)); !inserted) {
            changed = (it->second != value);
            if (!changed) return;  // 值未变化，直接返回
            it->second = std::move(value);
        }

        // 通知回调
        notify(name, flags_.at(name));
    }

    // 获取标志值 (返回 optional)
    [[nodiscard]] std::optional<Value> get(const FlagName& name) const {
        std::shared_lock lock(mutex_);
        if (auto it = flags_.find(name); it != flags_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 安全获取值 (带默认值)
    template <typename T>
    [[nodiscard]] T get_or(const FlagName& name, T&& default_value) const {
        std::shared_lock lock(mutex_);
        if (auto it = flags_.find(name); it != flags_.end()) {
            return std::visit([&](auto&& arg) -> T {
                using U = std::decay_t<decltype(arg)>;

                if constexpr (std::is_convertible_v<U, T>) {
                    return static_cast<T>(arg);
                }
                else {
                    return std::forward<T>(default_value);
                }
                }, it->second);
        }
        return std::forward<T>(default_value);
    }

    // 强类型获取 (失败时抛出异常)
    template <typename T>
    [[nodiscard]] T get_as(const FlagName& name) const {
        std::shared_lock lock(mutex_);
        if (auto it = flags_.find(name); it != flags_.end()) {
            try {
                if constexpr (std::is_same_v<T, bool>) {
                    return std::get<bool>(it->second);
                }
                else if constexpr (std::is_integral_v<T>) {
                    return static_cast<T>(std::get<int>(it->second));
                }
                else if constexpr (std::is_floating_point_v<T>) {
                    return static_cast<T>(std::get<float>(it->second));
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return std::get<std::string>(it->second);
                }
            }
            catch (const std::bad_variant_access& e) {
                throw std::runtime_error("Type mismatch for flag: " + std::string(e.what()));
            }
        }
        throw std::out_of_range("Flag not found: " + std::visit([](auto&& str) {
            using U = std::decay_t<decltype(str)>;
            if constexpr (std::is_same_v<U, std::string>) {
                return str;
            }
            else if constexpr (std::is_same_v<U, std::wstring>) {
                return std::string(str.begin(), str.end());
            }
            else {
                return std::string("unknown");
            }
            }, name.value()));
    }

    // 检查标志是否存在
    [[nodiscard]] bool contains(const FlagName& name) const noexcept {
        std::shared_lock lock(mutex_);
        return flags_.contains(name);
    }

    // 添加回调 (返回唯一标识)
    [[nodiscard]] size_t add_callback(Callback callback) {
        std::scoped_lock lock(mutex_);
        const size_t id = next_id_++;
        callbacks_.emplace(id, std::move(callback));
        return id;
    }

    // 移除回调
    void remove_callback(size_t id) noexcept {
        std::scoped_lock lock(mutex_);
        callbacks_.erase(id);
    }

    // 重置所有标志
    void reset() noexcept {
        std::scoped_lock lock(mutex_);
        flags_.clear();
    }

private:
    GlobalFlag() = default;

    // 通知所有回调 (安全处理异常)
    void notify(const FlagName& name, const Value& value) noexcept {
        for (const auto& [id, callback] : callbacks_) {
            if (callback) {
                try {
                    callback(name, value);
                }
                catch (const std::exception& e) {
                    // 记录异常但不传播
                    std::cerr << "GlobalFlag callback exception: " << e.what() << "\n";
                }
                catch (...) {
                    std::cerr << "GlobalFlag callback unknown exception\n";
                }
            }
        }
    }

    // 使用 shared_mutex 实现读写锁
    mutable std::shared_mutex mutex_;
    std::unordered_map<FlagName, Value> flags_;
    std::unordered_map<size_t, Callback> callbacks_;
    size_t next_id_ = 0;
};
