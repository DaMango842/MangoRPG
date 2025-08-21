#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <atomic>
#include <memory>
#include <concepts>
#include <memory_resource>
#include <functional>
#include <vector>
#include <algorithm>

template<typename Derived, typename Base>
concept DerivedFrom = std::is_base_of_v<Base, Derived>;

template<typename T>
concept Polymorphic = std::is_polymorphic_v<T>;

// 前向声明
template<typename T, typename Deleter = std::default_delete<T>>
class MangoPtr;

// ControlBlock 基类
struct ControlBlockBase {
    std::atomic<size_t> count;
    virtual ~ControlBlockBase() = default;
    virtual void delete_object(void* ptr) = 0;
};

// 自由函数形式的类型转换
template<typename U, typename T, typename D>
[[nodiscard]] MangoPtr<U> mango_static_pointer_cast(const MangoPtr<T, D>& ptr) noexcept;

template<typename U, typename T, typename D>
[[nodiscard]] MangoPtr<U> mango_dynamic_pointer_cast(const MangoPtr<T, D>& ptr);

template<typename U, typename T, typename D>
[[nodiscard]] MangoPtr<U> mango_reinterpret_pointer_cast(const MangoPtr<T, D>& ptr) noexcept;

/**
 * @brief Hybrid smart pointer combining unique_ptr and shared_ptr features
 */
template<typename T, typename Deleter>
class MangoPtr {
    template<typename U, typename UD>
    friend class MangoPtr;

    // 友元声明类型转换函数
    template<typename U2, typename T2, typename D2>
    friend MangoPtr<U2> mango_static_pointer_cast(const MangoPtr<T2, D2>& ptr) noexcept;

    template<typename U2, typename T2, typename D2>
    friend MangoPtr<U2> mango_dynamic_pointer_cast(const MangoPtr<T2, D2>& ptr);

    template<typename U2, typename T2, typename D2>
    friend MangoPtr<U2> mango_reinterpret_pointer_cast(const MangoPtr<T2, D2>& ptr) noexcept;

private:
    T* m_ptr{ nullptr };
    ControlBlockBase* m_control{ nullptr };

    // 具体的 ControlBlock 实现
    struct ControlBlock : public ControlBlockBase {
        Deleter deleter;
        std::pmr::memory_resource* memory_resource{ nullptr };

        template<typename D>
        ControlBlock(D&& d, std::pmr::memory_resource* mr = nullptr)
            : deleter(std::forward<D>(d)), memory_resource(mr) {
            count.store(1, std::memory_order_relaxed);
        }

        void delete_object(void* ptr) override {
            deleter(static_cast<T*>(ptr));
        }
    };

    void release_control() noexcept {
        if (!m_control) return;

        if (m_control->count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (m_ptr) {
                m_control->delete_object(m_ptr);
            }
            auto* control = static_cast<ControlBlock*>(m_control);
            if (control->memory_resource) {
                control->memory_resource->deallocate(control, sizeof(ControlBlock), alignof(ControlBlock));
            }
            else {
                delete control;
            }
        }
        m_ptr = nullptr;
        m_control = nullptr;
    }

public:
    using element_type = T;
    using deleter_type = Deleter;

    // Default constructor (nullptr)
    constexpr MangoPtr() noexcept = default;

    // nullptr constructor
    constexpr MangoPtr(std::nullptr_t) noexcept
        : m_ptr(nullptr), m_control(nullptr) {
    }

    // Constructor taking ownership
    template<typename D = Deleter>
    explicit MangoPtr(T* ptr, D&& d = Deleter(),
        std::pmr::memory_resource* mr = nullptr)
        : m_ptr(ptr),
        m_control(ptr ? new ControlBlock(std::forward<D>(d), mr) : nullptr) {
    }

    // Conversion from unique_ptr
    template<typename D>
    explicit MangoPtr(std::unique_ptr<T, D>&& uptr,
        std::pmr::memory_resource* mr = nullptr)
        : MangoPtr(uptr.release(), std::move(uptr.get_deleter()), mr) {
    }

    // 转换构造函数 - 用于派生类到基类的转换
    template<typename U, typename UDeleter>
    MangoPtr(const MangoPtr<U, UDeleter>& other) noexcept
        : m_ptr(other.m_ptr), m_control(other.m_control) {
        if (m_control) {
            m_control->count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 移动转换构造函数
    template<typename U, typename UDeleter>
    MangoPtr(MangoPtr<U, UDeleter>&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)),
        m_control(std::exchange(other.m_control, nullptr)) {
    }

    // Copy operations
    MangoPtr(const MangoPtr& other) noexcept
        : m_ptr(other.m_ptr), m_control(other.m_control) {
        if (m_control) {
            m_control->count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    MangoPtr& operator=(const MangoPtr& other) noexcept {
        if (this != &other) {
            release_control();
            m_ptr = other.m_ptr;
            m_control = other.m_control;
            if (m_control) {
                m_control->count.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    // 转换赋值运算符
    template<typename U, typename UDeleter>
    MangoPtr& operator=(const MangoPtr<U, UDeleter>& other) noexcept {
        if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
            release_control();
            m_ptr = other.m_ptr;
            m_control = other.m_control;
            if (m_control) {
                m_control->count.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    // 移动转换赋值运算符
    template<typename U, typename UDeleter>
    MangoPtr& operator=(MangoPtr<U, UDeleter>&& other) noexcept {
        if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
            release_control();
            m_ptr = std::exchange(other.m_ptr, nullptr);
            m_control = std::exchange(other.m_control, nullptr);
        }
        return *this;
    }

    // nullptr assignment
    MangoPtr& operator=(std::nullptr_t) noexcept {
        release_control();
        return *this;
    }

    // Move operations
    MangoPtr(MangoPtr&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)),
        m_control(std::exchange(other.m_control, nullptr)) {
    }

    MangoPtr& operator=(MangoPtr&& other) noexcept {
        if (this != &other) {
            release_control();
            m_ptr = std::exchange(other.m_ptr, nullptr);
            m_control = std::exchange(other.m_control, nullptr);
        }
        return *this;
    }

    ~MangoPtr() noexcept {
        release_control();
    }

    // Core functionality
    [[nodiscard]] MangoPtr copy() const {
        if (!m_ptr) return nullptr;
        try {
            return MangoPtr(new T(*m_ptr), get_deleter());
        }
        catch (...) {
            return nullptr;
        }
    }

    [[nodiscard]] T* release() noexcept {
        T* tmp = std::exchange(m_ptr, nullptr);
        if (m_control && m_control->count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete static_cast<ControlBlock*>(m_control);
        }
        m_control = nullptr;
        return tmp;
    }

    void reset(T* ptr = nullptr) {
        if (ptr == m_ptr) return;

        release_control();
        if (ptr) {
            m_ptr = ptr;
            m_control = new ControlBlock(Deleter{});
        }
    }

    void reset(std::nullptr_t) noexcept {
        release_control();
    }

    void reset(T* ptr, Deleter deleter) {
        if (ptr == m_ptr) return;

        release_control();
        if (ptr) {
            m_ptr = ptr;
            m_control = new ControlBlock(std::move(deleter));
        }
    }

    // Observers
    [[nodiscard]] bool unique() const noexcept {
        return use_count() == 1;
    }

    [[nodiscard]] size_t use_count() const noexcept {
        return m_control ? m_control->count.load(std::memory_order_relaxed) : 0;
    }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }
    [[nodiscard]] bool empty() const noexcept { return !m_ptr; }

    // Accessors
    [[nodiscard]] T* operator->() {
        if (!m_ptr) throw std::runtime_error("Dereferencing null MangoPtr");
        return m_ptr;
    }

    [[nodiscard]] const T* operator->() const {
        if (!m_ptr) throw std::runtime_error("Dereferencing null MangoPtr");
        return m_ptr;
    }

    [[nodiscard]] T& operator*() {
        if (!m_ptr) throw std::runtime_error("Dereferencing null MangoPtr");
        return *m_ptr;
    }

    [[nodiscard]] const T& operator*() const {
        if (!m_ptr) throw std::runtime_error("Dereferencing null MangoPtr");
        return *m_ptr;
    }

    [[nodiscard]] T* get() noexcept { return m_ptr; }
    [[nodiscard]] const T* get() const noexcept { return m_ptr; }

    [[nodiscard]] Deleter get_deleter() const noexcept {
        if (m_control) {
            return static_cast<ControlBlock*>(m_control)->deleter;
        }
        return Deleter{};
    }

    // Type casting - 使用不同的命名避免关键字冲突
    template<typename U>
    [[nodiscard]] MangoPtr<U> cast_static() const noexcept {
        return mango_static_pointer_cast<U>(*this);
    }

    template<typename U>
    [[nodiscard]] MangoPtr<U> cast_dynamic() const {
        return mango_dynamic_pointer_cast<U>(*this);
    }

    template<typename U>
    [[nodiscard]] MangoPtr<U> cast_reinterpret() const noexcept {
        return mango_reinterpret_pointer_cast<U>(*this);
    }

    // Observer factory
    [[nodiscard]] static MangoPtr observe(T* raw) noexcept {
        MangoPtr p;
        p.m_ptr = raw;
        return p;
    }

    // Weak reference support
    [[nodiscard]] MangoPtr weak() const noexcept {
        MangoPtr weak_ref;
        weak_ref.m_ptr = m_ptr;
        weak_ref.m_control = m_control;
        return weak_ref;
    }

    [[nodiscard]] bool expired() const noexcept {
        return !m_control || m_control->count.load(std::memory_order_acquire) == 0;
    }

    [[nodiscard]] MangoPtr lock() const noexcept {
        if (expired()) return nullptr;
        MangoPtr strong;
        strong.m_ptr = m_ptr;
        strong.m_control = m_control;
        if (strong.m_control) {
            strong.m_control->count.fetch_add(1, std::memory_order_relaxed);
        }
        return strong;
    }

    // Utility
    void swap(MangoPtr& other) noexcept {
        using std::swap;
        swap(m_ptr, other.m_ptr);
        swap(m_control, other.m_control);
    }

    friend void swap(MangoPtr& a, MangoPtr& b) noexcept {
        a.swap(b);
    }

    // Comparison
    [[nodiscard]] bool operator==(const MangoPtr& o) const noexcept {
        return m_ptr == o.m_ptr;
    }

    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept {
        return empty();
    }

    [[nodiscard]] bool operator!=(std::nullptr_t) const noexcept {
        return !empty();
    }

    [[nodiscard]] bool operator!=(const MangoPtr& o) const noexcept {
        return m_ptr != o.m_ptr;
    }

    [[nodiscard]] bool operator<(const MangoPtr& o) const noexcept {
        return m_ptr < o.m_ptr;
    }

    [[nodiscard]] bool operator>(const MangoPtr& o) const noexcept {
        return m_ptr > o.m_ptr;
    }

    [[nodiscard]] bool operator<=(const MangoPtr& o) const noexcept {
        return m_ptr <= o.m_ptr;
    }

    [[nodiscard]] bool operator>=(const MangoPtr& o) const noexcept {
        return m_ptr >= o.m_ptr;
    }

    // Memory resource access
    [[nodiscard]] std::pmr::memory_resource* get_memory_resource() const noexcept {
        if (m_control) {
            return static_cast<ControlBlock*>(m_control)->memory_resource;
        }
        return nullptr;
    }

    // Hash support
    [[nodiscard]] size_t hash() const noexcept {
        return std::hash<T*>{}(m_ptr);
    }
};

// 实现类型转换自由函数
template<typename U, typename T, typename D>
[[nodiscard]] MangoPtr<U> mango_static_pointer_cast(const MangoPtr<T, D>& ptr) noexcept {
    if (!ptr.m_ptr) return MangoPtr<U>();
    MangoPtr<U> result;
    result.m_ptr = static_cast<U*>(ptr.m_ptr);
    result.m_control = ptr.m_control;
    if (result.m_control) {
        result.m_control->count.fetch_add(1, std::memory_order_relaxed);
    }
    return result;
}

template<typename U, typename T, typename D>
[[nodiscard]] MangoPtr<U> mango_dynamic_pointer_cast(const MangoPtr<T, D>& ptr) {
    if (!ptr.m_ptr) return MangoPtr<U>();
    if (U* derived = dynamic_cast<U*>(ptr.m_ptr)) {
        MangoPtr<U> result;
        result.m_ptr = derived;
        result.m_control = ptr.m_control;
        if (result.m_control) {
            result.m_control->count.fetch_add(1, std::memory_order_relaxed);
        }
        return result;
    }
    return MangoPtr<U>();
}

template<typename U, typename T, typename D>
[[nodiscard]] MangoPtr<U> mango_reinterpret_pointer_cast(const MangoPtr<T, D>& ptr) noexcept {
    if (!ptr.m_ptr) return MangoPtr<U>();
    MangoPtr<U> result;
    result.m_ptr = reinterpret_cast<U*>(ptr.m_ptr);
    result.m_control = ptr.m_control;
    if (result.m_control) {
        result.m_control->count.fetch_add(1, std::memory_order_relaxed);
    }
    return result;
}

// Factory functions
template<typename T, typename... Args>
[[nodiscard]] MangoPtr<T> make_mango_ptr(Args&&... args) {
    try {
        return MangoPtr<T>(new T(std::forward<Args>(args)...));
    }
    catch (...) {
        return nullptr;
    }
}

// Hash support
template<typename T, typename D>
struct std::hash<MangoPtr<T, D>> {
    size_t operator()(const MangoPtr<T, D>& p) const noexcept {
        return std::hash<T*>{}(p.get());
    }
};
