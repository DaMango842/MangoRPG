#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <atomic>
#include <memory>
#include <concepts>

template<typename Derived, typename Base>
concept DerivedFrom = std::is_base_of_v<Base, Derived>;

template<typename T>
concept Polymorphic = std::is_polymorphic_v<T>;

/**
 * @brief Hybrid smart pointer combining unique_ptr and shared_ptr features
 * - Thread-safe atomic reference counting
 * - Supports custom deleters
 * - C++20 concept-constrained type conversions
 * - Both owning and non-owning (observer) modes
 * - Full nullptr support (construction, assignment, comparison)
 */
template<typename T, typename Deleter = std::default_delete<T>>
class MangoPtr {
    template<typename U, typename UD>
    friend class MangoPtr;

public:
    using element_type = T;
    using deleter_type = Deleter;

    // Default constructor (nullptr)
    constexpr MangoPtr() noexcept = default;

    // nullptr constructor
    constexpr MangoPtr(std::nullptr_t) noexcept
        : m_ptr(nullptr), m_count(nullptr), m_deleter() {
    }

    // Constructor taking ownership
    explicit MangoPtr(T* ptr, Deleter d = Deleter())
        noexcept(std::is_nothrow_move_constructible_v<Deleter>)
        : m_ptr(ptr),
        m_count(ptr ? new std::atomic<size_t>(1) : nullptr),
        m_deleter(std::move(d)) {
    }

    // Conversion from unique_ptr
    explicit MangoPtr(std::unique_ptr<T, Deleter>&& uptr) noexcept
        : MangoPtr(uptr.release(), std::move(uptr.get_deleter())) {
    }

    // Conversion from shared_ptr (deep copy)
    template<typename D = Deleter>
    explicit MangoPtr(const std::shared_ptr<T>& sptr, D d = Deleter())
        noexcept(std::is_nothrow_constructible_v<Deleter, D>)
        : m_deleter(std::move(d)) {
        if (sptr) {
            m_ptr = new T(*sptr);
            m_count = new std::atomic<size_t>(1);
        }
    }

    // Copy operations
    MangoPtr(const MangoPtr& other) noexcept
        : m_ptr(other.m_ptr),
        m_count(other.m_count),
        m_deleter(other.m_deleter) {
        if (m_count) m_count->fetch_add(1, std::memory_order_relaxed);
    }

    MangoPtr& operator=(const MangoPtr& other) noexcept {
        if (this != &other) {
            releaseControl();
            m_ptr = other.m_ptr;
            m_count = other.m_count;
            m_deleter = other.m_deleter;
            if (m_count) m_count->fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    // nullptr assignment
    MangoPtr& operator=(std::nullptr_t) noexcept {
        releaseControl();
        return *this;
    }

    // Move operations
    MangoPtr(MangoPtr&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)),
        m_count(std::exchange(other.m_count, nullptr)),
        m_deleter(std::move(other.m_deleter)) {
    }

    MangoPtr& operator=(MangoPtr&& other) noexcept {
        if (this != &other) {
            releaseControl();
            m_ptr = std::exchange(other.m_ptr, nullptr);
            m_count = std::exchange(other.m_count, nullptr);
            m_deleter = std::move(other.m_deleter);
        }
        return *this;
    }

    ~MangoPtr() noexcept { releaseControl(); }

    // Core functionality
    [[nodiscard]] MangoPtr copy() const {
        if (!m_ptr) return nullptr;  // 支持空指针复制
        return MangoPtr(new T(*m_ptr), m_deleter);
    }

    [[nodiscard]] T* release() noexcept {
        T* tmp = std::exchange(m_ptr, nullptr);
        if (m_count && m_count->fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete m_count;
        }
        m_count = nullptr;
        return tmp;
    }

    void reset(T* ptr = nullptr) noexcept(std::is_nothrow_move_constructible_v<Deleter>) {
        releaseControl();
        if (ptr) {
            m_ptr = ptr;
            m_count = new std::atomic<size_t>(1);
        }
    }

    // 新增：reset with nullptr
    void reset(std::nullptr_t) noexcept {
        releaseControl();
    }

    // Observers
    [[nodiscard]] bool unique() const noexcept {
        return use_count() == 1;
    }

    [[nodiscard]] size_t use_count() const noexcept {
        return m_count ? m_count->load(std::memory_order_relaxed) : 0;
    }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }
    [[nodiscard]] bool empty() const noexcept { return !m_ptr; }

    // Accessors
    [[nodiscard]] T* operator->() noexcept {
        return get();
    }

    [[nodiscard]] const T* operator->() const noexcept {
        return get();
    }

    [[nodiscard]] T& operator*() noexcept {
        return *get();
    }

    [[nodiscard]] const T& operator*() const noexcept {
        return *get();
    }

    [[nodiscard]] T* get() const noexcept { return m_ptr; }
    [[nodiscard]] const T* get_const() const noexcept { return m_ptr; }

    // Alternative C++20 concept version
    template<typename U>
        requires DerivedFrom<U, T>
    [[nodiscard]] MangoPtr<U> staticCast() const noexcept {
        if (!m_ptr) return nullptr;  // 支持空指针转换
        return MangoPtr<U>(static_cast<U*>(m_ptr), m_count, m_deleter);
    }

    template<typename U>
        requires Polymorphic<T>&& DerivedFrom<U, T>
    [[nodiscard]] MangoPtr<U, Deleter> dynamicCast() const {
        if (!m_ptr) return nullptr;  // 支持空指针转换
        if (U* derived = dynamic_cast<U*>(m_ptr)) {
            return MangoPtr<U, Deleter>(new U(*derived), m_deleter);
        }
        return nullptr;
    }

    // Utility
    void swap(MangoPtr& other) noexcept {
        using std::swap;
        swap(m_ptr, other.m_ptr);
        swap(m_count, other.m_count);
        swap(m_deleter, other.m_deleter);
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

    [[nodiscard]] auto operator<=>(const MangoPtr& o) const noexcept {
        return m_ptr <=> o.m_ptr;
    }

    // Observer factory
    [[nodiscard]] static MangoPtr observe(T* raw) noexcept {
        if (!raw) return nullptr;  // 支持空指针观察
        MangoPtr p;
        p.m_ptr = raw;
        return p;
    }

private:
    T* m_ptr{ nullptr };
    std::atomic<size_t>* m_count{ nullptr };
    Deleter m_deleter{};

    // Private constructor for shared control
    MangoPtr(T* ptr, std::atomic<size_t>* count, Deleter d = Deleter()) noexcept
        : m_ptr(ptr), m_count(count), m_deleter(std::move(d)) {
        if (m_count) m_count->fetch_add(1, std::memory_order_relaxed);
    }

    void releaseControl() noexcept {
        if (!m_count) return;

        if (m_count->fetch_sub(1, std::memory_order_acq_rel) == 1) {
            m_deleter(m_ptr);
            delete m_count;
        }
        m_ptr = nullptr;
        m_count = nullptr;
    }

    [[nodiscard]] T* check() const {
        if (!m_ptr) throw std::runtime_error("Dereferencing null MangoPtr");
        return m_ptr;
    }
};

// Factory function with perfect forwarding and nullptr support
template<typename T, typename... Args>
[[nodiscard]]
auto make_mango_ptr(Args&&... args)
noexcept(noexcept(T(std::forward<Args>(args)...)))
-> std::enable_if_t<std::is_constructible_v<T, Args...>, MangoPtr<T>>
{
    try {
        return MangoPtr<T>(new T(std::forward<Args>(args)...));
    }
    catch (...) {
        return nullptr; // 构造失败时返回 nullptr
    }
}

// Specialization for array types with nullptr support
template<typename T>
[[nodiscard]]
MangoPtr<T[]> make_mango_ptr(size_t size)
noexcept(std::is_nothrow_default_constructible_v<T>)
{
    try {
        return MangoPtr<T[]>(new T[size]());
    }
    catch (...) {
        return nullptr; // 分配失败时返回 nullptr
    }
}

// Deduction guide for initializer lists
template<typename T, typename... Args>
MangoPtr(T*, Args...) -> MangoPtr<T>;

// Special factory for initializer lists with nullptr support
template<typename T, typename U, typename... Args>
[[nodiscard]]
MangoPtr<T> make_mango_ptr(Args&&... args, std::initializer_list<U> il)
noexcept(noexcept(T(std::forward<Args>(args)..., il)))
{
    try {
        return MangoPtr<T>(new T(std::forward<Args>(args)..., il));
    }
    catch (...) {
        return nullptr; // 构造失败时返回 nullptr
    }
}

template<typename T, typename D>
void swap(MangoPtr<T, D>& a, MangoPtr<T, D>& b) noexcept {
    a.swap(b);
}

// Hash support
template<typename T, typename D>
struct std::hash<MangoPtr<T, D>> {
    size_t operator()(const MangoPtr<T, D>& p) const noexcept {
        return std::hash<T*>{}(p.get());
    }
};
