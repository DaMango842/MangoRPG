#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @file SafeHandles.hpp
 * @brief 提供类型安全的智能指针包装器和实用工具
 */

template<typename>
inline constexpr bool always_false = false;

 // 前置声明
template<typename T, typename Deleter>
class MangoPtr;

/**
 * @class SafeRef
 * @brief 类型安全的非拥有引用包装器，具有空值检查功能
 * @tparam T 被引用对象的类型
 */
template<typename T>
class SafeRef {
public:
    SafeRef() = delete;

    /**
     * @brief 从引用构造SafeRef
     * @param ref 要包装的引用
     */
    explicit SafeRef(T& ref) noexcept : m_ptr(&ref) {}

    /**
     * @brief 检查引用是否有效（非空）
     * @return 有效返回true，否则返回false
     */
    bool isValid() const noexcept { return m_ptr != nullptr; }

    /**
     * @brief 获取底层引用
     * @return 被包装对象的引用
     */
    T& get() const noexcept { return *m_ptr; }

    // 指针式接口
    T* operator->() const noexcept { return m_ptr; }
    T& operator*() const noexcept { return *m_ptr; }

    // 转换运算符
    operator T& () const noexcept { return *m_ptr; }
    explicit operator bool() const noexcept { return isValid(); }

    // 比较运算符
    bool operator==(const SafeRef& other) const noexcept { return m_ptr == other.m_ptr; }
    bool operator!=(const SafeRef& other) const noexcept { return !(*this == other); }
    bool operator==(const T* ptr) const noexcept { return m_ptr == ptr; }
    bool operator!=(const T* ptr) const noexcept { return m_ptr != ptr; }

private:
    T* m_ptr;
};

/**
 * @class SafePtr
 * @brief 类型安全的非拥有指针包装器，具有空值检查功能
 * @tparam T 指向对象的类型
 */
template<typename T>
class SafePtr {
public:
    SafePtr() noexcept = default;

    /**
     * @brief 从原始指针构造SafePtr
     * @param ptr 要包装的原始指针
     */
    explicit SafePtr(T* ptr) noexcept : m_ptr(ptr) {}
    SafePtr(std::nullptr_t) noexcept : m_ptr(nullptr) {}

    /**
     * @brief 检查指针是否有效（非空）
     * @return 有效返回true，否则返回false
     */
    bool isValid() const noexcept { return m_ptr != nullptr; }

    /**
     * @brief 获取底层指针
     * @return 被包装对象的原始指针
     */
    T* get() const noexcept { return m_ptr; }

    // 带断言的指针式接口
    T& operator*() const { assert(m_ptr && "SafePtr: 解引用空指针"); return *m_ptr; }
    T* operator->() const { assert(m_ptr && "SafePtr: 访问空指针"); return m_ptr; }

    /**
     * @brief 重置包装的指针
     * @param ptr 要包装的新指针（默认为nullptr）
     */
    void reset(T* ptr = nullptr) noexcept { m_ptr = ptr; }

    explicit operator bool() const noexcept { return isValid(); }

    // 比较运算符
    friend bool operator==(const SafePtr& a, const SafePtr& b) noexcept { return a.m_ptr == b.m_ptr; }
    friend bool operator!=(const SafePtr& a, const SafePtr& b) noexcept { return !(a == b); }

private:
    T* m_ptr = nullptr;
};

/**
 * @class SafeHandle
 * @brief 类型安全的拥有指针包装器，具有空值检查功能
 * @tparam T 被管理对象的类型
 * @tparam PtrType 要包装的指针类型（unique_ptr、shared_ptr、原始指针等）
 */
template<typename T, typename PtrType>
class SafeHandle {
public:
    SafeHandle() = default;

    /**
     * @brief 从指针构造SafeHandle
     * @param ptr 要包装的指针
     */
    explicit SafeHandle(PtrType ptr) noexcept : m_ptr(std::move(ptr)) {}

    /**
     * @brief 从兼容指针类型构造SafeHandle
     * @tparam OtherPtr 兼容指针类型
     * @param otherPtr 要包装的指针
     */
    template<typename OtherPtr,
        typename = std::enable_if_t<std::is_convertible_v<OtherPtr, PtrType>>>
    SafeHandle(OtherPtr otherPtr)
        : m_ptr(std::move(otherPtr)) {
    }

    /**
     * @brief 从MangoPtr构造SafeHandle
     * @tparam D 删除器类型（默认为std::default_delete<T>）
     * @param mangoPtr 要包装的MangoPtr
     */
    template<typename D = std::default_delete<T>>
    explicit SafeHandle(const MangoPtr<T, D>& mangoPtr)
        : m_ptr(mangoPtr ? std::make_unique<T>(*mangoPtr) : nullptr) {
    }

    // 带断言的核心访问方法
    T* get() {
        assert(m_ptr && "SafeHandle: 访问空指针");
        return getRawPtr();
    }

    const T* get() const {
        assert(m_ptr && "SafeHandle: 访问空指针");
        return getRawPtr();
    }

    T& operator*() {
        assert(m_ptr && "SafeHandle: 解引用空指针");
        return *getRawPtr();
    }

    const T& operator*() const {
        assert(m_ptr && "SafeHandle: 解引用空指针");
        return *getRawPtr();
    }

    T* operator->() {
        assert(m_ptr && "SafeHandle: 访问空指针");
        return getRawPtr();
    }

    const T* operator->() const {
        assert(m_ptr && "SafeHandle: 访问空指针");
        return getRawPtr();
    }

    /**
     * @brief 重置包装的指针
     * @param newPtr 要包装的新指针（默认为nullptr）
     */
    void reset(PtrType newPtr = nullptr) noexcept {
        m_ptr = std::move(newPtr);
    }

    /**
     * @brief 检查句柄是否有效（非空）
     * @return 有效返回true，否则返回false
     */
    bool isValid() const noexcept {
        return getRawPtr() != nullptr;
    }

    explicit operator bool() const noexcept {
        return isValid();
    }

    /**
     * @brief 释放对包装指针的所有权
     * @return 被释放的指针
     */
    PtrType release() noexcept {
        return std::move(m_ptr);
    }

    /**
     * @brief 转换为MangoPtr的运算符
     * @tparam D 删除器类型（默认为std::default_delete<T>）
     * @return 包含被管理对象副本的MangoPtr
     */
    template<typename D = std::default_delete<T>>
    operator MangoPtr<T, D>() const {
        return isValid() ? MangoPtr<T, D>(std::make_unique<T>(*get())) : MangoPtr<T, D>();
    }

    SafeHandle copy() const {
        if (!isValid()) return SafeHandle{};

        if constexpr (requires { get()->clone(); }) {
            return SafeHandle{ PtrType(get()->clone()) };
        }
        else {
            return SafeHandle{ PtrType(std::make_unique<T>(*get())) };
        }
    }

    std::unique_ptr<T> convert_unique() const {
        if (!m_ptr) return nullptr;

        if constexpr (requires(const T & t) { t.clone(); }) {
            return std::unique_ptr<T>(m_ptr->clone());
        }
        else if constexpr (std::is_copy_constructible_v<T>) {
            return std::make_unique<T>(*m_ptr);
        }
        else {
            static_assert(always_false<T>, "T must be copy-constructible or provide clone()");
        }
    }


private:
    T* getRawPtr() const noexcept {
        if constexpr (std::is_pointer_v<PtrType>)
            return m_ptr;
        else
            return m_ptr.get();
    }

    PtrType m_ptr;
};

// 类型别名
template<typename T> using UniqueHandle = SafeHandle<T, std::unique_ptr<T>>;   ///< unique_ptr包装器
template<typename T> using SharedHandle = SafeHandle<T, std::shared_ptr<T>>;  ///< shared_ptr包装器
template<typename T> using RawHandle = SafeHandle<T, T*>;                     ///< 原始指针包装器
template<typename T, typename Deleter = std::default_delete<T>>
using MangoHandle = SafeHandle<T, MangoPtr<T, Deleter>>;                      ///< MangoPtr包装器

// 类型特征辅助类
template<typename T>
struct is_unique_ptr : std::false_type {};
template<typename U>
struct is_unique_ptr<std::unique_ptr<U>> : std::true_type {};

template<typename T>
struct is_shared_ptr : std::false_type {};
template<typename U>
struct is_shared_ptr<std::shared_ptr<U>> : std::true_type {};

template<typename T>
constexpr bool is_smart_ptr_v = is_unique_ptr<std::decay_t<T>>::value ||
is_shared_ptr<std::decay_t<T>>::value;

template<typename T, typename = void>
struct is_vector : std::false_type {};
template<typename... Args>
struct is_vector<std::vector<Args...>, void> : std::true_type {};

/**
 * @concept Clonable
 * @brief 可通过clone()方法克隆的类型概念
 */
template<typename T>
concept Clonable = requires(const T & obj) {
    { obj.clone() } -> std::same_as<std::unique_ptr<T>>;
};

/**
 * @brief 创建对象的深拷贝
 * @tparam T 要拷贝的对象类型
 * @param obj 要拷贝的对象
 * @return 指向拷贝后对象的unique_ptr
 */
template<typename T>
std::unique_ptr<T> deepCopyObject(const T& obj) {
    if constexpr (Clonable<T>) return obj.clone();
    else return std::make_unique<T>(obj);
}

/**
 * @brief 创建对象的共享深拷贝
 * @tparam T 要拷贝的对象类型
 * @param obj 要拷贝的对象
 * @return 指向拷贝后对象的shared_ptr
 */
template<typename T>
std::shared_ptr<T> sharedDeepCopyObject(const T& obj) {
    if constexpr (Clonable<T>) return std::shared_ptr<T>(obj.clone().release());
    else return std::make_shared<T>(obj);
}

/**
 * @namespace SafeConvert
 * @brief 提供不同句柄类型之间的转换工具
 */
namespace SafeConvert {
    /**
     * @brief 将MangoPtr转换为UniqueHandle
     * @tparam T 被管理对象的类型
     * @tparam D 删除器类型（默认为std::default_delete<T>）
     * @param mangoPtr 要转换的MangoPtr
     * @return 包含被管理对象副本的UniqueHandle
     */
    template<typename T, typename D = std::default_delete<T>>
    UniqueHandle<T> toUniqueHandle(const MangoPtr<T, D>& mangoPtr) {
        return mangoPtr ? UniqueHandle<T>(std::make_unique<T>(*mangoPtr)) : UniqueHandle<T>();
    }

    /**
     * @brief 将UniqueHandle转换为MangoPtr
     * @tparam T 被管理对象的类型
     * @tparam Deleter 删除器类型（默认为std::default_delete<T>）
     * @param handle 要转换的UniqueHandle
     * @return 包含被管理对象副本的MangoPtr
     */
    template<typename T, typename Deleter = std::default_delete<T>>
    MangoPtr<T, Deleter> toMangoPtr(const UniqueHandle<T>& handle) {
        return handle ? MangoPtr<T, Deleter>(std::make_unique<T>(*handle)) : MangoPtr<T, Deleter>();
    }

    /**
     * @brief 将MangoPtr转换为SharedHandle
     * @tparam T 被管理对象的类型
     * @tparam D 删除器类型
     * @param mangoPtr 要转换的MangoPtr
     * @return 包含被管理对象副本的SharedHandle
     */
    template<typename T, typename D>
    SharedHandle<T> toSharedHandle(const MangoPtr<T, D>& mangoPtr) {
        return mangoPtr ? SharedHandle<T>(std::make_shared<T>(*mangoPtr)) : SharedHandle<T>();
    }
} // namespace SafeConvert

// 深拷贝接口
template<typename T> std::unique_ptr<T> deepCopy(const SafePtr<T>& ptr) {
    assert(ptr); return deepCopyObject(*ptr.get());
}
template<typename T> std::unique_ptr<T> deepCopy(const SafeRef<T>& ref) {
    return deepCopyObject(ref.get());
}
template<typename T, typename P> std::unique_ptr<T> deepCopy(const SafeHandle<T, P>& handle) {
    assert(handle); return deepCopyObject(*handle.get());
}
template<typename T> std::unique_ptr<T> deepCopy(T* ptr) {
    assert(ptr); return deepCopyObject(*ptr);
}

// 工厂函数
template <typename T>
UniqueHandle<T> makeHandle(std::unique_ptr<T>&& ptr) {
    return UniqueHandle<T>(std::move(ptr));
}

template <typename T>
UniqueHandle<T> makeHandle(T* ptr) {
    return UniqueHandle<T>(std::unique_ptr<T>(ptr));
}

template <typename T, typename D>
UniqueHandle<T> makeHandle(const MangoPtr<T, D>& mangoPtr) {
    return SafeConvert::toUniqueHandle(mangoPtr);
}

template <typename T>
SharedHandle<T> makeSharedHandle(std::shared_ptr<T>&& ptr) {
    return SharedHandle<T>(std::move(ptr));
}

template <typename T>
SharedHandle<T> makeSharedHandle(T* ptr) {
    return SharedHandle<T>(std::shared_ptr<T>(ptr));
}

template <typename T, typename D>
SharedHandle<T> makeSharedHandle(const MangoPtr<T, D>& mangoPtr) {
    return SafeConvert::toSharedHandle(mangoPtr);
}

template <typename T>
RawHandle<T> makeRawHandle(T* ptr) {
    return RawHandle<T>(ptr);
}

template <typename T, typename D>
MangoHandle<T> makeMangoHandle(const MangoPtr<T, D>& mangoPtr) {
    return MangoHandle<T>(mangoPtr);
}

/**
 * @brief 自动为输入创建适当的句柄类型
 * @tparam T 输入类型
 * @param obj 输入对象
 * @return 适合输入的句柄类型
 */
template<typename T>
auto deepCopyAuto(T&& obj) {
    using Decayed = std::decay_t<T>;

    if constexpr (is_unique_ptr<Decayed>::value) {
        return UniqueHandle<typename Decayed::element_type>(std::move(obj));
    }
    else if constexpr (is_shared_ptr<Decayed>::value) {
        return SharedHandle<typename Decayed::element_type>(std::move(obj));
    }
    else if constexpr (std::is_same_v<Decayed, MangoPtr<typename Decayed::element_type, typename Decayed::deleter_type>>) {
        return SafeConvert::toUniqueHandle(obj);
    }
    else if constexpr (std::is_pointer_v<Decayed>) {
        return RawHandle<std::remove_pointer_t<Decayed>>(obj);
    }
    else if constexpr (is_vector<Decayed>::value) {
        using ElemType = typename Decayed::value_type;

        if constexpr (is_unique_ptr<ElemType>::value) {
            std::vector<UniqueHandle<typename ElemType::element_type>> result;
            for (auto& e : obj) {
                result.emplace_back(std::move(e));
            }
            return result;
        }
        else if constexpr (is_shared_ptr<ElemType>::value) {
            std::vector<SharedHandle<typename ElemType::element_type>> result;
            for (auto& e : obj) {
                result.emplace_back(std::move(e));
            }
            return result;
        }
        else if constexpr (std::is_same_v<ElemType, MangoPtr<typename ElemType::element_type, typename ElemType::deleter_type>>) {
            std::vector<UniqueHandle<typename ElemType::element_type>> result;
            for (const auto& e : obj) {
                result.push_back(SafeConvert::toUniqueHandle(e));
            }
            return result;
        }
        else if constexpr (std::is_pointer_v<ElemType>) {
            std::vector<RawHandle<std::remove_pointer_t<ElemType>>> result;
            for (auto& e : obj) {
                result.emplace_back(e);
            }
            return result;
        }
        else {
            static_assert(sizeof(T) == 0, "deepCopyAuto: 不支持的vector元素类型");
        }
    }
    else {
        static_assert(sizeof(T) == 0, "deepCopyAuto: 不支持的类型");
    }
}

// 便捷宏
#define Ref(T) SafeRef<T>  ///< 创建T类型的SafeRef
#define Ptr(T) SafePtr<T>  ///< 创建T类型的SafePtr
